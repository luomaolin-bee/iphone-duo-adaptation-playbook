// 0.2.21 手柄 Select 兼容层。
// 只读轮询 buttonOptions 的按下状态，不替换 Unity/InControl 已安装的任何 handler；
// 需要转发时复用 PulseControl 发送已有的背包(7)/地图(8)触控键，不新增确认/取消路径。
#include "ControllerSelectPolicy.h"
#import <GameController/GameController.h>

static NSInteger controllerSelectMode=DuoSelectModeAuto;
static NSUInteger selectPresses,selectForwarded,selectDropped;
static NSString *selectLastEvent;
static char selectStateKey,selectGestureKey;
static CADisplayLink *selectLink;

static int DuoPadProfile(GCController *controller){
    GCExtendedGamepad *pad=controller.extendedGamepad;if(!pad)return DuoPadOther;
    Class dualSense=NSClassFromString(@"GCDualSenseGamepad"),dualShock=NSClassFromString(@"GCDualShockGamepad"),xbox=NSClassFromString(@"GCXboxGamepad");
    if(dualSense&&[pad isKindOfClass:dualSense])return DuoPadDualSense;
    if(dualShock&&[pad isKindOfClass:dualShock])return DuoPadDualShock;
    if(xbox&&[pad isKindOfClass:xbox])return DuoPadXbox;
    NSString *category=controller.productCategory.lowercaseString?:@"";
    if([category containsString:@"dualsense"])return DuoPadDualSense;
    if([category containsString:@"dualshock"])return DuoPadDualShock;
    if([category containsString:@"xbox"])return DuoPadXbox;
    return DuoPadGeneric;
}
static NSString *DuoPadProfileName(int profile){
    switch(profile){case DuoPadXbox:return @"Xbox";case DuoPadDualShock:return @"DualShock4";case DuoPadDualSense:return @"DualSense";case DuoPadOther:return @"无扩展手柄";default:return @"通用/MFi";}
}
static NSString *GestureName(GCSystemGestureState state){
    switch(state){case GCSystemGestureStateEnabled:return @"enabled";case GCSystemGestureStateAlwaysReceive:return @"alwaysReceive";case GCSystemGestureStateDisabled:return @"disabled";default:return @"unknown";}
}
static NSMutableData *SelectState(GCController *controller){
    NSMutableData *data=objc_getAssociatedObject(controller,&selectStateKey);
    if(!data){data=[NSMutableData dataWithLength:sizeof(DuoSelectState)];DuoSelectReset(data.mutableBytes);objc_setAssociatedObject(controller,&selectStateKey,data,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
    return data;
}
// 接入/回前台/重连时调用：请求系统不要把 Select/Share 当截屏或录屏手势吞掉（Home 不动）。
static void BindSelectController(GCController *controller){
    DuoSelectReset(SelectState(controller).mutableBytes);
    GCControllerButtonInput *options=controller.extendedGamepad.buttonOptions;
    if(options){
        if(!objc_getAssociatedObject(controller,&selectGestureKey))objc_setAssociatedObject(controller,&selectGestureKey,GestureName(options.preferredSystemGestureState),OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        options.preferredSystemGestureState=GCSystemGestureStateDisabled;
    }
    NSLog(@"[DuoSceneBridge 0.2.21] controller select bind vendor=%@ category=%@ profile=%@ options=%@ gesture=%@->%@ mode=%@",controller.vendorName,controller.productCategory,DuoPadProfileName(DuoPadProfile(controller)),options?@"yes":@"no",objc_getAssociatedObject(controller,&selectGestureKey)?:@"n/a",options?GestureName(options.systemGestureState):@"n/a",@(DuoSelectModeName((int)controllerSelectMode)));
}
@interface DuoSelectPoller : NSObject
- (void)tick:(CADisplayLink *)link;
@end
@implementation DuoSelectPoller
- (void)tick:(CADisplayLink *)link{
    double now=CACurrentMediaTime();
    for(GCController *controller in GCController.controllers){
        GCControllerButtonInput *options=controller.extendedGamepad.buttonOptions;if(!options)continue;
        DuoSelectState *state=SelectState(controller).mutableBytes;BOOL wasDown=state->down;
        int profile=DuoPadProfile(controller);
        int target=DuoSelectTarget((int)controllerSelectMode,profile);
        int fire=DuoSelectStep(state,options.isPressed,now,target);
        if(state->down&&!wasDown&&state->armed){selectPresses++;}
        if(fire==DuoSelectNone){
            if(state->down&&!wasDown&&state->armed)selectLastEvent=[NSString stringWithFormat:@"press profile=%@ forwarded=no target=%d",DuoPadProfileName(profile),target];
            continue;
        }
        UIView *pad=activePad;
        if(!pad||customEditing||pulseControl>=0){selectDropped++;selectLastEvent=[NSString stringWithFormat:@"press profile=%@ dropped pad=%d editing=%d busy=%d",DuoPadProfileName(profile),pad!=nil,customEditing,pulseControl>=0];continue;}
        PulseControl(pad,fire);selectForwarded++;
        selectLastEvent=[NSString stringWithFormat:@"press profile=%@ forwarded control=%d",DuoPadProfileName(profile),fire];
        NSLog(@"[DuoSceneBridge 0.2.21] controller select edge profile=%@ control=%d",DuoPadProfileName(profile),fire);
    }
}
@end
static DuoSelectPoller *selectPoller;
static void UpdateSelectPolling(void){
    BOOL want=GCController.controllers.count>0&&UIApplication.sharedApplication.applicationState==UIApplicationStateActive;
    if(want&&!selectLink){
        if(!selectPoller)selectPoller=[DuoSelectPoller new];
        selectLink=[CADisplayLink displayLinkWithTarget:selectPoller selector:@selector(tick:)];
        selectLink.preferredFramesPerSecond=60;[selectLink addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
    }else if(!want&&selectLink){[selectLink invalidate];selectLink=nil;}
}
static void RebindAllSelectControllers(void){for(GCController *controller in GCController.controllers)BindSelectController(controller);UpdateSelectPolling();}
static void InstallControllerSelect(void){
    static BOOL installed;if(installed)return;installed=YES;
    NSNumber *stored=[NSUserDefaults.standardUserDefaults objectForKey:@"DuoControllerSelect021"];
    controllerSelectMode=DuoSelectNormalizeMode(stored?stored.integerValue:DuoSelectModeAuto);
    NSNotificationCenter *center=NSNotificationCenter.defaultCenter;
    [center addObserverForName:GCControllerDidConnectNotification object:nil queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *note){BindSelectController(note.object);UpdateSelectPolling();}];
    [center addObserverForName:GCControllerDidDisconnectNotification object:nil queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *note){
        GCController *controller=note.object;objc_setAssociatedObject(controller,&selectStateKey,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);UpdateSelectPolling();}];
    [center addObserverForName:UIApplicationDidBecomeActiveNotification object:nil queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *note){RebindAllSelectControllers();}];
    [center addObserverForName:UIApplicationWillResignActiveNotification object:nil queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *note){UpdateSelectPolling();for(GCController *controller in GCController.controllers)DuoSelectReset(SelectState(controller).mutableBytes);}];
    dispatch_async(dispatch_get_main_queue(),^{RebindAllSelectControllers();});
}
static void CycleControllerSelectMode(void){
    controllerSelectMode=DuoSelectNextMode((int)controllerSelectMode);
    [NSUserDefaults.standardUserDefaults setInteger:controllerSelectMode forKey:@"DuoControllerSelect021"];
    for(GCController *controller in GCController.controllers)DuoSelectReset(SelectState(controller).mutableBytes);
}
static NSString *ControllerSelectMenuTitle(void){return [NSString stringWithFormat:@"手柄 Select：%@（点按切换）",@(DuoSelectModeName((int)controllerSelectMode))];}
static NSDictionary *ControllerSelectReport(void){
    NSMutableArray *pads=[NSMutableArray array];
    for(GCController *controller in GCController.controllers){
        GCControllerButtonInput *options=controller.extendedGamepad.buttonOptions;int profile=DuoPadProfile(controller);
        [pads addObject:@{@"vendor":controller.vendorName?:@"",@"category":controller.productCategory?:@"",@"profile":DuoPadProfileName(profile),
            @"hasSelectElement":@(options!=nil),@"selectPressedNow":@(options.isPressed),@"gestureBefore":objc_getAssociatedObject(controller,&selectGestureKey)?:@"n/a",
            @"gestureNow":options?GestureName(options.systemGestureState):@"n/a",@"target":@(DuoSelectTarget((int)controllerSelectMode,profile))}];
    }
    return @{@"mode":@(DuoSelectModeName((int)controllerSelectMode)),@"controllers":pads,@"presses":@(selectPresses),@"forwarded":@(selectForwarded),@"dropped":@(selectDropped),@"lastEvent":selectLastEvent?:@"none",@"polling":@(selectLink!=nil),
        @"policy":@"Auto forwards Select to existing inventory control only for DualShock4/DualSense identity; Xbox/MFi stays native. Read-only polling; no game handlers replaced."};
}
