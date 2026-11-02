#import <UIKit/UIKit.h>
#import <objc/runtime.h>
#import <mach-o/dyld.h>
#import <mach-o/loader.h>
#import <math.h>
#import <dlfcn.h>
#import <stdatomic.h>
#import <Metal/Metal.h>
#import <CoreImage/CoreImage.h>

@interface NSObject (UnityBridgeCalls)
- (void)initUnityWithApplication:(UIApplication *)application;
- (UIWindow *)window;
- (UIView *)unityView;
- (NSMutableArray *)diagnosticSamples;
- (void)clearKeys;
- (NSMutableDictionary *)layouts;
- (NSString *)activeProfile;
- (CGRect)layoutArea;
- (CGRect)padRect;
- (BOOL)editingLayout;
- (BOOL)collapsed;
- (BOOL)hiddenControl:(NSInteger)control;
- (id)entry:(NSInteger)control;
- (NSMutableDictionary *)fingers;
- (void)setFingers:(NSMutableDictionary *)fingers;
- (NSInteger)engineLoadState;
- (void)onUpdateSurfaceSize:(CGSize)size;
- (void)recreateRenderingSurfaceIfNeeded;
- (CGRect)controlRect:(NSInteger)control;
- (void)updateKeys;
- (UIViewController *)presenter;
- (void)applicationDidBecomeActive:(UIApplication *)application;
- (void)applicationWillResignActive:(UIApplication *)application;
- (void)applicationWillEnterForeground:(UIApplication *)application;
- (void)applicationDidEnterBackground:(UIApplication *)application;
@end

static __weak UIWindowScene *connectingScene;
static void (*originalInit)(id,SEL,UIApplication *);
static id (*originalPick)(id,SEL,NSSet *);
static BOOL initialized;
static CGRect gameRegion;
typedef struct {uint32_t width,height,split;} ViewportDimensions;
static ViewportDimensions *viewportDimensions;
static CGRect controlsRegion;
static UIEdgeInsets controlSafeInsets;
static BOOL splitActive;
static NSDictionary *foldState;
static __weak UIWindow *gameWindow;
static __weak UIView *gameView;
static NSTimer *layoutTimer;
static BOOL physicalFold;
static CFTimeInterval lastReflection;
static void (*requestResolution)(unsigned,unsigned);
static void (*getRenderingResolution)(int *,int *);
static void (*getSystemResolution)(int *,int *);
static int (*screenWidth)(void *);
static int (*screenHeight)(void *);
static CGSize lastRequestedSize;
static NSInteger resolutionRetries;
static CFTimeInterval lastResolutionRequest;
static CGSize lastObservedRenderingSize;
static BOOL suppressDPadDrawing;
static char joystickFingerKey;
static UIImage *reflectedGameImage;
static NSString *reflectionStatus=@"not-started";
static NSUInteger reflectionFrames;
static atomic_bool wantsReflection;
static atomic_bool reflectionInFlight;
static __weak CAMetalLayer *gameMetalLayer;
static dispatch_queue_t reflectionQueue;
static uintptr_t presentationBlockAddress;
static void InstallMetalReflection(void);
static void InstallDirectTouch(void);
static void RedrawPadsForImage(UIView *view);
static void RedrawPads(UIView *view);
static atomic_uint reflectionTargetFPS=20;
static atomic_uint reflectionMaxEdge=256;
static double reflectionDeliveryFPS,reflectionLatencyMS;
static NSUInteger reflectionReadbackBytes;
static void UpdateMoreButton(UIView *view);
static void SyncRenderSize(BOOL force);
static void UpdateFoldLayout(void);
static void InstallLayoutHooks(void);


static void BridgeInit(id self, SEL cmd, UIApplication *app) {
    if (!connectingScene && app.connectedScenes.count == 0) {
        NSLog(@"[DuoSceneBridge 0.2.19] defer Unity initialization until scene connects");
        return;
    }
    if (!initialized) {
        initialized = YES;
        originalInit(self,cmd,app);
    }
}
static id BridgePick(id self, SEL cmd, NSSet *scenes) {
    return connectingScene ?: originalPick(self,cmd,scenes);
}

@interface DuoSceneDelegate : UIResponder <UIWindowSceneDelegate>
@property(nonatomic,strong) UIWindow *window;
@property(nonatomic) BOOL sentActive;
@property(nonatomic) BOOL sentBackground;
@end
@implementation DuoSceneDelegate
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    if (![scene isKindOfClass:UIWindowScene.class]) return;
    connectingScene = (UIWindowScene *)scene;
    id controller = UIApplication.sharedApplication.delegate;
    if (![controller respondsToSelector:@selector(initUnityWithApplication:)]) {
        NSLog(@"[DuoSceneBridge 0.2.19] missing Unity initializer");
        return;
    }
    [controller initUnityWithApplication:UIApplication.sharedApplication];
    self.window = [controller window];
    if (self.window.windowScene != connectingScene) self.window.windowScene = connectingScene;
    self.window.frame = connectingScene.coordinateSpace.bounds;
    [self.window makeKeyAndVisible];
    gameWindow = self.window;
    gameView = [controller respondsToSelector:@selector(unityView)] ? [controller unityView] : nil;
    InstallLayoutHooks();
    InstallDirectTouch();
    InstallMetalReflection();
    if (@available(iOS 27.1, *)) {
        UIHingeInteraction *hinge = [[UIHingeInteraction alloc] initWithUpdateHandler:^(UIHingeInteraction *interaction, UIHingeInteractionUpdate *update) {
            NSMutableDictionary *state = [foldState mutableCopy] ?: [NSMutableDictionary dictionary];
            state[@"hingeStatus"] = update.hinge ? @(update.hinge.status) : [NSNull null];
            state[@"hingeAngleRadians"] = update.hinge ? @(update.hinge.angle) : [NSNull null];
            foldState = state;
            UpdateFoldLayout();
        }];
        [self.window addInteraction:hinge];
    }
    [layoutTimer invalidate];
    layoutTimer = [NSTimer timerWithTimeInterval:0.2 repeats:YES block:^(NSTimer *timer) { UpdateFoldLayout(); }];
    [[NSRunLoop mainRunLoop] addTimer:layoutTimer forMode:NSRunLoopCommonModes];
    UpdateFoldLayout();
    NSLog(@"[DuoSceneBridge 0.2.19] scene connected window=%@ bounds=%@",self.window,NSStringFromCGRect(self.window.bounds));
}
- (void)sceneDidBecomeActive:(UIScene *)scene {
    if (!initialized || self.sentActive) return;
    self.sentActive = YES;
    [UIApplication.sharedApplication.delegate applicationDidBecomeActive:UIApplication.sharedApplication];
}
- (void)sceneWillResignActive:(UIScene *)scene {
    if (!self.sentActive) return;
    self.sentActive = NO;
    atomic_store(&wantsReflection,false);
    [UIApplication.sharedApplication.delegate applicationWillResignActive:UIApplication.sharedApplication];
}
- (void)sceneDidEnterBackground:(UIScene *)scene {
    if (!initialized || self.sentBackground) return;
    self.sentBackground = YES;
    [UIApplication.sharedApplication.delegate applicationDidEnterBackground:UIApplication.sharedApplication];
}
- (void)sceneWillEnterForeground:(UIScene *)scene {
    if (!self.sentBackground) return;
    self.sentBackground = NO;
    [UIApplication.sharedApplication.delegate applicationWillEnterForeground:UIApplication.sharedApplication];
}
- (void)windowScene:(UIWindowScene *)scene didUpdateCoordinateSpace:(id<UICoordinateSpace>)previous interfaceOrientation:(UIInterfaceOrientation)orientation traitCollection:(UITraitCollection *)traits {
    self.window.frame = scene.coordinateSpace.bounds;
    [self.window.rootViewController.view setNeedsLayout];
    NSLog(@"[DuoSceneBridge 0.2.19] resize %@",NSStringFromCGRect(self.window.bounds));
}
- (void)sceneDidDisconnect:(UIScene *)scene {
    if (self.sentActive) [self sceneWillResignActive:scene];
    if (initialized && !self.sentBackground) [self sceneDidEnterBackground:scene];
    if (connectingScene == scene) {
        [layoutTimer invalidate]; layoutTimer = nil;
        connectingScene = nil; gameWindow = nil; gameView = nil;
        splitActive = NO;
    }
}
@end

static void (*originalFrame)(UIView *,SEL,CGRect);
static CGRect (*originalArea)(UIView *,SEL);
static id (*originalReport)(id,SEL);
static void (*originalSample)(id,SEL,BOOL);
static id (*originalProfile)(id,SEL);
static id (*originalEntry)(id,SEL,NSInteger);
static void (*originalDraw)(UIView *,SEL,CGRect);
static BOOL (*originalHidden)(id,SEL,NSInteger);
static void (*originalKeys)(id,SEL);
static CGRect (*originalToggle)(UIView *,SEL);
// Recovery baseline: no managed UI raycast or click invocation.
static NSUInteger directTapCount,uiExceptionCount,unityNativeTouchCount;
static void InstallDirectTouch(void) {}
#include "ConsoleLayout.h"
#include "LayoutModel.inc"
#include "Metrics.inc"
#include "HUDSafeArea.inc"
#include "Context.inc"
#include "ConsoleUI.inc"
#include "Feedback.inc"
#include "Branding.inc"
#include "Editor.inc"
#include "Controls.inc"

static NSArray *RectArray(CGRect r) { return @[@(r.origin.x),@(r.origin.y),@(r.size.width),@(r.size.height)]; }
static void FrameHook(UIView *view, SEL cmd, CGRect frame) {
    if (splitActive && view.window == gameWindow && view.superview) {
        frame = [view.superview convertRect:gameRegion fromView:gameWindow];
    }
    originalFrame(view,cmd,frame);
}
static CGRect AreaHook(UIView *view,SEL cmd){return LayoutCanvas(view);}
static id ProfileHook(id self,SEL cmd){return LayoutProfile();}
static id EntryHook(id self,SEL cmd,NSInteger control){return LayoutEntry(self,control);}
static CGRect ControlRectHook(id self,SEL cmd,NSInteger control){return control>=0&&control<=14?UnifiedRect(self,control):originalControlRect(self,cmd,control);}
static void DrawHook(UIView *view,SEL cmd,CGRect dirty){
    ConsoleBegin(view);suppressDPadDrawing=![(id)view editingLayout];
    originalDraw(view,cmd,dirty);suppressDPadDrawing=NO;ConsoleEnd(view);
}
static BOOL HiddenHook(id self,SEL cmd,NSInteger control) {
    if(suppressDPadDrawing&&control==0)return YES;
    if(control==10)return YES;
    if(customEditing)return NO;
    if(pulseInput&&control==pulseControl)return NO;
    if(MenuHides(control))return YES;
    if(control==5||control==6){NSNumber *visible=objc_getAssociatedObject(self,&extrasVisibleKey);if(visible)return !visible.boolValue;}
    return originalHidden(self,cmd,control);
}
static void KeysHook(id self,SEL cmd) {
    if(customEditing)return;
    if ([self editingLayout] || [self collapsed]) { originalKeys(self,cmd); return; }
    NSMutableDictionary *real=[self fingers];
    NSMutableDictionary *mapped=[real mutableCopy];
    CGRect pad=[self padRect]; CGFloat radius=pad.size.width/2;
    CGPoint center=CGPointMake(CGRectGetMidX(pad),CGRectGetMidY(pad));
    id captured=objc_getAssociatedObject(self,&joystickFingerKey);
    if (captured && !real[captured]) { captured=nil; objc_setAssociatedObject(self,&joystickFingerKey,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC); }
    if (!captured) {
        for (id key in real) {
            CGPoint point=[real[key] CGPointValue];
            if (hypot(point.x-center.x,point.y-center.y)<=radius) {
                captured=key;objc_setAssociatedObject(self,&joystickFingerKey,key,OBJC_ASSOCIATION_RETAIN_NONATOMIC);break;
            }
        }
    }
    if (captured) {
        CGPoint point=[real[captured] CGPointValue];
        CGFloat dx=(point.x-center.x)/MAX(radius,1),dy=(point.y-center.y)/MAX(radius,1),d=hypot(dx,dy);
        CGFloat x=0,y=0;
        if (d>.18) {
            if (fabs(dx)/d>.382683) x=copysign(.4,dx);
            if (fabs(dy)/d>.382683) y=copysign(.4,dy);
        }
        mapped[captured]=[NSValue valueWithCGPoint:CGPointMake(center.x+x*pad.size.width,center.y+y*pad.size.height)];
    }
    for(id key in real){
        if([key isEqual:captured])continue;
        CGPoint point=[real[key] CGPointValue];
        if(CGRectContainsPoint(pad,point))mapped[key]=[NSValue valueWithCGPoint:center];
    }
    [self setFingers:mapped]; originalKeys(self,cmd); [self setFingers:real];
}
static CGRect ToggleHook(UIView *view,SEL cmd){return UnifiedRect(view,12);}
static void UpdateReflectionsInView(UIView *view) {
    if ([view isKindOfClass:NSClassFromString(@"TPPad")]) { UpdateMoreButton(view);ConsoleUpdateDeck(view);[view setNeedsDisplay];return; }
    for (UIView *child in [view.subviews copy]) UpdateReflectionsInView(child);
}
static void RedrawPadsForImage(UIView *view){
    if([view isKindOfClass:NSClassFromString(@"TPPad")]){ConsoleUpdateDeck(view);return;}
    for(UIView *child in view.subviews)RedrawPadsForImage(child);
}
static id ReportHook(id self, SEL cmd) {
    NSMutableDictionary *report = [originalReport(self,cmd) mutableCopy];
    if (!report) return nil;
    report[@"touchPatchVersion"] = report[@"patchVersion"] ?: @"unknown";
    report[@"patchVersion"] = @"0.2.19";
    report[@"duoBridge"] = foldState ?: @{};
    report[@"directTouch"]=@{@"experimentalBridgeEnabled":@NO,@"raycastClicks":@(directTapCount),@"managedExceptions":@(uiExceptionCount),@"nativeTouches":@(unityNativeTouchCount)};
    report[@"reflectionCapture"]=@{@"status":reflectionStatus?:@"unknown",@"frames":@(reflectionFrames),@"savesToDisk":@NO,@"targetFPS":@(atomic_load(&reflectionTargetFPS)),@"deliveredFPS":@(reflectionDeliveryFPS),@"latencyMS":@(reflectionLatencyMS),@"readbackBytes":@(reflectionReadbackBytes)};
    report[@"hudSafeArea"]=hudSafeReport?:@{};
    report[@"unifiedLayout"]=@{@"profile":LayoutProfile(),@"entries":LayoutEntries(self),@"editing":@(customEditing),@"coordinates":@"normalized centres within full-width control canvas"};
    report[@"gameContext"]=gameContextReport?:@{};
    report[@"performance"]=@{@"FPS":@(measuredGameFPS),@"presentedMeasurement":@(measuredPresented),@"frameMS":@(frameMS),@"cpuPercentOneCore100":@(cpuPercent),@"gpuSampleMS":@(gpuMS),@"residentMB":@(residentMB),@"footprintMB":@(footprintMB),@"thermalState":ThermalName(),@"reflectionQuality":@(reflectionQuality),@"automaticMenus":@(automaticMenus)};
    report[@"feedback"]=@{@"damageEvents":@(damageFeedbackCount),@"healingEvents":@(healingFeedbackCount),@"interactionEvents":@(interactionFeedbackCount),@"sceneReveals":@(sceneRevealCount)};
    report[@"branding"]=brandingReport?:@{};
    report[@"consoleStyle"]=@{@"glassEnabled":@(consoleGlassEnabled),@"actions":@"All six existing gameplay actions visible by default; existing HID bindings preserved"};
    report[@"notes"] = @"Window coordinates, public UIKit hinge state and division regions are recorded. Bridge selects portrait/folded layouts automatically. Original touch patch manual pose labels remain annotations.";
    report[@"testScope"] = @"0.2.18 geometry/input baseline; full-screen monitor; sampled Metal timing; health/context decorative feedback; finite Fake Lu branding animation";
    if(viewportDimensions)report[@"cameraViewportPixels"]=@[@(viewportDimensions->width),@(viewportDimensions->height)];
    report[@"menuInputPolicy"]=@"Keyboard menu mapping: Submit or Attack confirms; Cancel returns; Jump and Cast are neutral. Dedicated back button sends existing Escape control.";
    report[@"handheldLayout"] = @"Full-width portrait visuals; preserve split rendering orientation; attack confirms; dedicated back key";
    return report;
}
static void SampleHook(id self, SEL cmd, BOOL force) {
    originalSample(self,cmd,force);
    NSMutableArray *samples = [self diagnosticSamples];
    if ([samples isKindOfClass:NSMutableArray.class] && samples.count && [samples.lastObject isKindOfClass:NSDictionary.class]) {
        NSMutableDictionary *sample = [samples.lastObject mutableCopy];
        sample[@"duoBridge"] = foldState ?: @{};
        samples[samples.count-1] = sample;
    }
}
static void InstallLayoutHooks(void) {
    static BOOL installed;
    if (installed) return;
    Class unity = NSClassFromString(@"UnityView");
    Class pad = NSClassFromString(@"TPPad");
    Method frame = class_getInstanceMethod(unity,@selector(setFrame:));
    Method area = class_getInstanceMethod(pad,NSSelectorFromString(@"layoutArea"));
    Method report = class_getInstanceMethod(pad,NSSelectorFromString(@"diagnosticReport"));
    Method sample = class_getInstanceMethod(pad,NSSelectorFromString(@"sampleGeometry:"));
    Method profile = class_getInstanceMethod(pad,NSSelectorFromString(@"activeProfile"));
    Method entry = class_getInstanceMethod(pad,NSSelectorFromString(@"entry:"));
    Method draw = class_getInstanceMethod(pad,@selector(drawRect:));
    Method hidden=class_getInstanceMethod(pad,NSSelectorFromString(@"hiddenControl:"));
    Method keys=class_getInstanceMethod(pad,NSSelectorFromString(@"updateKeys"));
    Method toggle=class_getInstanceMethod(pad,NSSelectorFromString(@"toggleRect"));
    Method control=class_getInstanceMethod(pad,NSSelectorFromString(@"controlRect:"));
    Method settings=class_getInstanceMethod(pad,NSSelectorFromString(@"openSettings"));
    if(control)originalControlRect=(void *)method_setImplementation(control,(IMP)ControlRectHook);
    if(settings)originalSettings=(void *)method_setImplementation(settings,(IMP)UnifiedSettingsHook);
    Method beginEditor=class_getInstanceMethod(pad,NSSelectorFromString(@"beginEditor"));if(beginEditor)method_setImplementation(beginEditor,(IMP)UnifiedSettingsHook);
    if (!frame || !area || !report || !sample || !profile || !entry || !draw || !hidden || !keys || !toggle) { NSLog(@"[DuoSceneBridge] layout hooks unavailable"); return; }
    originalFrame = (void *)method_getImplementation(frame);
    if (!class_addMethod(unity,@selector(setFrame:),(IMP)FrameHook,method_getTypeEncoding(frame))) method_setImplementation(frame,(IMP)FrameHook);
    originalArea = (void *)method_setImplementation(area,(IMP)AreaHook);
    originalReport = (void *)method_setImplementation(report,(IMP)ReportHook);
    originalSample = (void *)method_setImplementation(sample,(IMP)SampleHook);
    originalProfile = (void *)method_setImplementation(profile,(IMP)ProfileHook);
    originalEntry = (void *)method_setImplementation(entry,(IMP)EntryHook);
    originalDraw = (void *)method_setImplementation(draw,(IMP)DrawHook);
    originalHidden=(void *)method_setImplementation(hidden,(IMP)HiddenHook);
    originalKeys=(void *)method_setImplementation(keys,(IMP)KeysHook);
    originalToggle=(void *)method_setImplementation(toggle,(IMP)ToggleHook);
    Method inside=class_getInstanceMethod(pad,@selector(pointInside:withEvent:));
    Method paint=class_getInstanceMethod(pad,NSSelectorFromString(@"paint:label:pressed:"));
    Method begin=class_getInstanceMethod(unity,@selector(touchesBegan:withEvent:));
    if(inside)originalInside=(void *)method_setImplementation(inside,(IMP)InsideHook);
    if(paint)originalPaint=(void *)method_setImplementation(paint,(IMP)PaintHook);
    if(begin)originalUnityBegin=(void *)method_setImplementation(begin,(IMP)UnityBeginHook);
    installed = YES;
}
static void RedrawPads(UIView *view) {
    if ([view isKindOfClass:NSClassFromString(@"TPPad")]) {
        [(id)view clearKeys];
        UpdateMoreButton(view);
        [view setNeedsDisplay]; [view setNeedsLayout];
    }
    for (UIView *child in view.subviews) RedrawPads(child);
}
static void UpdateFoldLayout(void) {
    UIWindow *window = gameWindow;
    if (!window) return;
    CGRect bounds = window.bounds;
    CGRect upper = bounds, lower = CGRectZero;
    BOOL found = NO;
    NSMutableArray *regions = [NSMutableArray array];
    if (@available(iOS 27.1, *)) {
        for (UIViewReservedRegion *region in [window reservedRegionsOfKind:UIViewReservedRegionKind.divisionRegionKind options:UIViewReservedRegionQueryOptionsIncludeInactive]) {
            CGRect r = CGRectIntersection(bounds,region.frame);
            [regions addObject:@{@"frame":RectArray(region.frame),@"active":@(region.isActive)}];
            if (region.isActive && !CGRectIsNull(r) && r.size.width >= bounds.size.width * 0.6 && r.size.width > r.size.height && CGRectGetMinY(r) > bounds.size.height * 0.15 && CGRectGetMaxY(r) < bounds.size.height * 0.85) {
                upper = CGRectMake(0,0,bounds.size.width,MAX(1,CGRectGetMinY(r)));
                lower = CGRectMake(0,CGRectGetMaxY(r),bounds.size.width,MAX(1,CGRectGetMaxY(bounds)-CGRectGetMaxY(r)));
                found = YES;
                break;
            }
        }
    }
    controlSafeInsets=window.safeAreaInsets;
    CGFloat topOcclusion=0;
    NSMutableArray *occlusions=[NSMutableArray array];
    if(@available(iOS 27.1,*)){
        for(UIViewReservedRegion *region in [window reservedRegionsOfKind:UIViewReservedRegionKind.occlusionRegionKind]){
            [occlusions addObject:@{@"frame":RectArray(region.frame),@"active":@(region.isActive)}];
            if(region.isActive&&bounds.size.width>bounds.size.height&&region.frame.size.width<bounds.size.width*.25){
                if(CGRectGetMaxX(region.frame)>=CGRectGetMaxX(bounds)-1)controlSafeInsets.right=MAX(controlSafeInsets.right,region.frame.size.width);
                if(region.frame.origin.x<=1)controlSafeInsets.left=MAX(controlSafeInsets.left,region.frame.size.width);
            }
            CGRect horizontalSafe=UIEdgeInsetsInsetRect(bounds,window.safeAreaInsets);
            CGRect overlap=CGRectIntersection(horizontalSafe,region.frame);
            if(region.isActive && !CGRectIsNull(overlap) && overlap.size.width>0 && region.frame.origin.y<1 && region.frame.size.width<bounds.size.width*.6)topOcclusion=MAX(topOcclusion,CGRectGetMaxY(region.frame));
        }
    }
    UpdateHudSafeArea();
    NSProcessInfo *process=NSProcessInfo.processInfo;
    atomic_store(&reflectionMaxEdge,256);
    atomic_store(&reflectionTargetFPS,process.thermalState>=NSProcessInfoThermalStateSerious?10:(process.lowPowerModeEnabled?12:(reflectionQuality==2?30:20)));
    physicalFold = found;
    if (!found && bounds.size.height > bounds.size.width) {
        UIEdgeInsets safe = window.safeAreaInsets;
        BOOL inner=regions.count>0;
        CGFloat divider=bounds.size.height*(inner?.645:.625);
        CGFloat y=MAX(safe.top,topOcclusion);
        // Visuals extend across the display; control hit areas retain safe insets.
        upper=CGRectMake(0,y,bounds.size.width,MAX(1,divider-y));
        lower=CGRectMake(0,divider,bounds.size.width,MAX(1,bounds.size.height-divider));
        found = YES;
    } else if (found) {
        UIEdgeInsets safe = window.safeAreaInsets;
        CGRect available = UIEdgeInsetsInsetRect(upper,UIEdgeInsetsMake(MAX(safe.top,topOcclusion),safe.left,0,safe.right));
        CGFloat height = available.size.height;
        CGFloat width = MIN(available.size.width,height*2.39);
        if (width/height < 4.0/3.0) height=width*3.0/4.0;
        upper = CGRectMake(CGRectGetMidX(available)-width/2,CGRectGetMaxY(available)-height,width,height);
    }
    BOOL changed = splitActive != found || !CGRectEqualToRect(gameRegion,upper) || !CGRectEqualToRect(controlsRegion,lower);
    splitActive = found; gameRegion = upper; controlsRegion = lower;
    PollGameContext();
    NSMutableDictionary *state = [foldState mutableCopy] ?: [NSMutableDictionary dictionary];
    state[@"bridgeVersion"] = @"0.2.19";
    state[@"divisionRegions"] = regions;
    state[@"occlusionRegions"] = occlusions;
    state[@"controlSafeInsets"] = @[@(controlSafeInsets.top),@(controlSafeInsets.left),@(controlSafeInsets.bottom),@(controlSafeInsets.right)];
    state[@"upperGameLowerControls"] = @(found);
    state[@"physicalFoldSplit"] = @(physicalFold);
    state[@"layoutMode"] = found ? (physicalFold ? @"folded-handheld" : @"portrait-handheld") : @"landscape";
    state[@"reflection"] = @"GPU downsample + Gaussian blur with reusable resources; composited image layer; balanced 20 / smooth 30 FPS, 256px edge cap";
    atomic_store(&wantsReflection,found&&!customEditing&&reflectionQuality>0&&UIApplication.sharedApplication.applicationState==UIApplicationStateActive);
    state[@"gameRegionInWindow"] = RectArray(upper);
    state[@"controlsRegionInWindow"] = RectArray(lower);
    foldState = state;
    if(viewportDimensions)viewportDimensions->split=splitActive?1:0;
    if (!changed) SyncRenderSize(NO);
    CFTimeInterval now = CACurrentMediaTime();
    if (changed || now-lastReflection > 1.5) {
        lastReflection = now;
        for (UIWindow *candidate in window.windowScene.windows) UpdateReflectionsInView(candidate);
    }
    if (!changed) return;
    UpdateEditorGeometry();
    UIView *view = gameView;
    if (view.superview) {
        view.frame = [view.superview convertRect:upper fromView:window];
        [view setNeedsLayout]; [view layoutIfNeeded];
        SyncRenderSize(YES);
    }
    for (UIWindow *candidate in window.windowScene.windows) RedrawPads(candidate);
}

static void ResolveRenderFunctions(void) {
    const unsigned char expected[16]={0xad,0x0a,0x91,0xdf,0xb7,0x8c,0x38,0xbc,0x8f,0xb7,0x0a,0xeb,0xa6,0xd2,0xb6,0xa5};
    for (uint32_t i=0;i<_dyld_image_count();i++) {
        const char *name=_dyld_get_image_name(i);
        if (!name || !strstr(name,"UnityFramework.framework/UnityFramework")) continue;
        const struct mach_header_64 *header=(const void *)_dyld_get_image_header(i);
        const struct load_command *cmd=(const void *)(header+1);
        BOOL match=NO;
        for (uint32_t j=0;j<header->ncmds;j++) {
            if (cmd->cmd==LC_UUID && !memcmp(((const struct uuid_command *)cmd)->uuid,expected,16)) match=YES;
            cmd=(const void *)((const char *)cmd+cmd->cmdsize);
        }
        if (!match) return;
        uintptr_t base=(uintptr_t)header;
        hudEngineBase=base;
        viewportDimensions=(void *)(base+0xd785280);
        if(!viewportDimensions->width||!viewportDimensions->height){viewportDimensions->width=640;viewportDimensions->height=480;}
        presentationBlockAddress=base+0x175d8;
        requestResolution=(void *)(base+0xd0ef7c);
        getRenderingResolution=(void *)(base+0xd0efdc);
        getSystemResolution=(void *)(base+0xd0f010);
        screenWidth=(void *)(base+0x9065ea8);
        screenHeight=(void *)(base+0x9065f4c);
        return;
    }
}
static void SyncRenderSize(BOOL force) {
    if (!requestResolution) ResolveRenderFunctions();
    if(viewportDimensions&&gameView){
        CGSize size=gameView.bounds.size;CGFloat scale=gameView.contentScaleFactor;
        viewportDimensions->width=(uint32_t)MAX(1,MIN(16384,lround(size.width*scale)));
        viewportDimensions->height=(uint32_t)MAX(1,MIN(16384,lround(size.height*scale)));
        viewportDimensions->split=splitActive?1:0;
    }
    id app=UIApplication.sharedApplication.delegate;
    if (!requestResolution || ![app respondsToSelector:@selector(engineLoadState)] || [app engineLoadState]<3 || UIApplication.sharedApplication.applicationState!=UIApplicationStateActive) return;
    InstallMetalReflection();
    CGSize points=gameView.bounds.size;
    CGFloat scale=gameView.contentScaleFactor;
    CGSize target=splitActive ? CGSizeMake(MAX(2,lround(points.width*scale/2)*2),MAX(2,lround(points.height*scale/2)*2)) : CGSizeZero;
    if (!CGSizeEqualToSize(lastRequestedSize,target)) { resolutionRetries=0;force=YES; }
    int rw=0,rh=0,sw=0,sh=0;
    getRenderingResolution(&rw,&rh);getSystemResolution(&sw,&sh);
    int mw=screenWidth(NULL),mh=screenHeight(NULL);
    CGSize observed=CGSizeMake(rw,rh);
    if(!CGSizeEqualToSize(observed,lastObservedRenderingSize)){resolutionRetries=0;lastObservedRenderingSize=observed;}
    CFTimeInterval now=CACurrentMediaTime();
    BOOL mismatch=splitActive && (rw!=(int)target.width || rh!=(int)target.height);
    if (force || (mismatch && resolutionRetries<3 && now-lastResolutionRequest>2)) {
        requestResolution((unsigned)target.width,(unsigned)target.height);
        [(id)gameView onUpdateSurfaceSize:points];
        [(id)gameView recreateRenderingSurfaceIfNeeded];
        lastRequestedSize=target;lastResolutionRequest=now;resolutionRetries++;
    }
    NSMutableDictionary *state=[foldState mutableCopy] ?: [NSMutableDictionary dictionary];
    state[@"renderSync"]=@{@"requestedPixels":@[@(target.width),@(target.height)],@"renderingPixels":@[@(rw),@(rh)],@"systemPixels":@[@(sw),@(sh)],@"managedScreenPixels":@[@(mw),@(mh)],@"requestCount":@(resolutionRetries),@"splitAspectError":@(splitActive&&rh>0&&target.height>0?fabs((double)rw/rh-target.width/target.height):0)};
    if ([gameView.layer isKindOfClass:CAMetalLayer.class]) {
        CGSize drawable=((CAMetalLayer *)gameView.layer).drawableSize;
        state[@"metalDrawablePixels"]=@[@(drawable.width),@(drawable.height)];
    }
    foldState=state;
}

#include "MetalReflection.inc"

__attribute__((constructor)) static void InstallDuoSceneBridge(void) {
    @autoreleasepool {
        id stored=[NSUserDefaults.standardUserDefaults objectForKey:@"DuoConsoleGlass017"];consoleGlassEnabled=stored?[stored boolValue]:YES;
        NSNumber *feedback=[NSUserDefaults.standardUserDefaults objectForKey:@"DuoFeedback019"];feedbackEnabled=feedback?feedback.boolValue:YES;
        NSNumber *autoMenu=[NSUserDefaults.standardUserDefaults objectForKey:@"DuoAutoMenus018"];automaticMenus=autoMenu?autoMenu.boolValue:YES;
        NSNumber *fps=[NSUserDefaults.standardUserDefaults objectForKey:@"DuoFPS018"];showFPS=fps.boolValue;
        NSNumber *quality=[NSUserDefaults.standardUserDefaults objectForKey:@"DuoReflectionQuality018"];reflectionQuality=quality?MAX(0,MIN(2,quality.integerValue)):1;
        NSString *path = [NSBundle.mainBundle.privateFrameworksPath stringByAppendingPathComponent:@"UnityFramework.framework"];
        NSError *error = nil;
        if (![[NSBundle bundleWithPath:path] loadAndReturnError:&error]) {
            NSLog(@"[DuoSceneBridge 0.2.19] UnityFramework load failed: %@",error);
            return;
        }
        ResolveRenderFunctions();
        Class cls = NSClassFromString(@"UnityAppController");
        Method init = class_getInstanceMethod(cls,@selector(initUnityWithApplication:));
        Method pick = class_getInstanceMethod(cls,NSSelectorFromString(@"pickStartupWindowScene:"));
        if (!init || !pick) {
            NSLog(@"[DuoSceneBridge 0.2.19] required methods missing");
            return;
        }
        originalInit = (void *)method_setImplementation(init,(IMP)BridgeInit);
        originalPick = (void *)method_setImplementation(pick,(IMP)BridgePick);
        NSLog(@"[DuoSceneBridge 0.2.19] installed");
    }
}
