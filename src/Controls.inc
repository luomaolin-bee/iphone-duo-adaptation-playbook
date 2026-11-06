static void CycleControllerSelectMode(void);
static NSString *ControllerSelectMenuTitle(void);
static NSInteger pulseControl=-1;
static BOOL pulseInput;
static char moreButtonKey;
static char extrasVisibleKey;
static void PulseControl(id pad,NSInteger control){
    if(pulseControl>=0)return;
    pulseControl=control;
    CGRect rect=[pad controlRect:control];
    NSString *key=NSUUID.UUID.UUIDString;
    NSMutableDictionary *fingers=[[pad fingers] mutableCopy]?:[NSMutableDictionary dictionary];
    fingers[key]=[NSValue valueWithCGPoint:CGPointMake(CGRectGetMidX(rect),CGRectGetMidY(rect))];
    [pad setFingers:fingers];pulseInput=YES;[pad updateKeys];pulseInput=NO;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(.13*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
        NSMutableDictionary *current=[[pad fingers] mutableCopy];[current removeObjectForKey:key];
        [pad setFingers:current];pulseInput=YES;[pad updateKeys];pulseInput=NO;pulseControl=-1;
    });
}
@interface DuoMoreActions : NSObject
@property(nonatomic,weak) UIView *pad;
- (void)show:(UIButton *)button;
@end
@implementation DuoMoreActions
- (void)show:(UIButton *)button {
    id pad=self.pad;UIViewController *presenter=[pad presenter];if(!presenter)return;
    while(presenter.presentedViewController)presenter=presenter.presentedViewController;
    UIAlertController *sheet=[UIAlertController alertControllerWithTitle:nil message:nil preferredStyle:UIAlertControllerStyleActionSheet];
    [sheet addAction:[UIAlertAction actionWithTitle:@"统一布局编辑" style:UIAlertActionStyleDefault handler:^(UIAlertAction *action){dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(.2*NSEC_PER_SEC)),dispatch_get_main_queue(),^{OpenUnifiedEditor(pad);});}]];
    [sheet addAction:[UIAlertAction actionWithTitle:showFPS?@"关闭帧率监测":@"显示帧率监测" style:UIAlertActionStyleDefault handler:^(UIAlertAction *action){showFPS=!showFPS;[NSUserDefaults.standardUserDefaults setBool:showFPS forKey:@"DuoFPS018"];UpdateFPSWidget(pad);}]];
    [sheet addAction:[UIAlertAction actionWithTitle:consoleGlassEnabled?@"关闭液态玻璃":@"开启液态玻璃" style:UIAlertActionStyleDefault handler:^(UIAlertAction *action){
        consoleGlassEnabled=!consoleGlassEnabled;[NSUserDefaults.standardUserDefaults setBool:consoleGlassEnabled forKey:@"DuoConsoleGlass017"];
        for(UIWindow *window in gameWindow.windowScene.windows)RedrawPads(window);
    }]];
    [sheet addAction:[UIAlertAction actionWithTitle:feedbackEnabled?@"关闭场景与受伤反馈":@"开启场景与受伤反馈" style:UIAlertActionStyleDefault handler:^(UIAlertAction *action){feedbackEnabled=!feedbackEnabled;[NSUserDefaults.standardUserDefaults setBool:feedbackEnabled forKey:@"DuoFeedback019"];}]];
    [sheet addAction:[UIAlertAction actionWithTitle:ControllerSelectMenuTitle() style:UIAlertActionStyleDefault handler:^(UIAlertAction *action){CycleControllerSelectMode();}]];
    [sheet addAction:[UIAlertAction actionWithTitle:@"取消" style:UIAlertActionStyleCancel handler:nil]];
    sheet.popoverPresentationController.sourceView=button;sheet.popoverPresentationController.sourceRect=button.bounds;
    [presenter presentViewController:sheet animated:YES completion:nil];
}
@end
static char backButtonKey,backTargetKey;
@interface DuoBackAction : NSObject
@property(nonatomic,weak) UIView *pad;
- (void)back:(UIButton *)button;
@end
@implementation DuoBackAction
- (void)back:(UIButton *)button {PulseControl(self.pad,9);}
@end
static void UpdateBackButton(UIView *pad){
    UIButton *button=objc_getAssociatedObject(pad,&backButtonKey);
    if(!button){
        button=[UIButton buttonWithType:UIButtonTypeSystem];
        button.tintColor=[UIColor colorWithRed:.88 green:.83 blue:.73 alpha:1];
        [button setImage:[UIImage systemImageNamed:@"arrow.uturn.backward"] forState:UIControlStateNormal];
        button.backgroundColor=[UIColor colorWithWhite:.06 alpha:.9];button.layer.cornerRadius=20;
        button.layer.borderWidth=1;button.layer.borderColor=[UIColor colorWithRed:.54 green:.46 blue:.35 alpha:.85].CGColor;
        button.accessibilityLabel=@"返回";
        DuoBackAction *target=[DuoBackAction new];target.pad=pad;
        [button addTarget:target action:@selector(back:) forControlEvents:UIControlEventTouchUpInside];
        objc_setAssociatedObject(pad,&backTargetKey,target,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        objc_setAssociatedObject(pad,&backButtonKey,button,OBJC_ASSOCIATION_RETAIN_NONATOMIC);[pad addSubview:button];
    }
    button.hidden=[(id)pad editingLayout]||[(id)pad collapsed];
    button.hidden=[(id)pad collapsed]||(!customEditing&&[LayoutEntry(pad,11)[@"hidden"]boolValue]);
    button.frame=UnifiedRect(pad,11);button.alpha=[LayoutEntry(pad,11)[@"alpha"]doubleValue];ConsoleStyleButton(button);
}
static char moreTargetKey;
static void UpdateMoreButton(UIView *pad){
    UpdateBackButton(pad);
    UIButton *button=objc_getAssociatedObject(pad,&moreButtonKey);
    if(!button){
        button=[UIButton buttonWithType:UIButtonTypeSystem];
        button.tintColor=[UIColor colorWithRed:.82 green:.77 blue:.66 alpha:1];
        [button setImage:[UIImage systemImageNamed:@"ellipsis"] forState:UIControlStateNormal];
        button.backgroundColor=[UIColor colorWithWhite:.06 alpha:.8];button.layer.cornerRadius=18;
        button.layer.borderColor=[UIColor colorWithRed:.48 green:.40 blue:.30 alpha:.8].CGColor;button.layer.borderWidth=1;
        button.accessibilityLabel=@"更多操作";
        DuoMoreActions *target=[DuoMoreActions new];target.pad=pad;
        [button addTarget:target action:@selector(show:) forControlEvents:UIControlEventTouchUpInside];
        objc_setAssociatedObject(pad,&moreTargetKey,target,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        objc_setAssociatedObject(pad,&moreButtonKey,button,OBJC_ASSOCIATION_RETAIN_NONATOMIC);[pad addSubview:button];
    }
    button.hidden=[(id)pad editingLayout]||[(id)pad collapsed];
    button.frame=UnifiedRect(pad,13);button.alpha=[LayoutEntry(pad,13)[@"alpha"]doubleValue];ConsoleStyleButton(button);
    activePad=pad;StartFPSClock();UpdateFPSWidget(pad);BringEditorFront(pad);
}
static BOOL (*originalInside)(UIView *,SEL,CGPoint,UIEvent *);
static void (*originalPaint)(id,SEL,CGRect,NSString *,BOOL);
static void (*originalUnityBegin)(id,SEL,NSSet *,UIEvent *);
static BOOL InsideHook(UIView *view,SEL cmd,CGPoint point,UIEvent *event){
    if(customEditing)return YES;
    if([(id)view editingLayout])return originalInside(view,cmd,point,event);
    UIButton *more=objc_getAssociatedObject(view,&moreButtonKey);
    if(more&&!more.hidden&&CGRectContainsPoint(more.frame,point))return YES;
    UIButton *back=objc_getAssociatedObject(view,&backButtonKey);
    if(back&&!back.hidden&&CGRectContainsPoint(back.frame,point))return YES;
    if(splitActive){CGRect region=[view convertRect:gameRegion fromView:gameWindow];if(CGRectContainsPoint(region,point))return NO;}
    return originalInside(view,cmd,point,event);
}
static void UnityBeginHook(id self,SEL cmd,NSSet *touches,UIEvent *event){
    unityNativeTouchCount+=touches.count;originalUnityBegin(self,cmd,touches,event);
}
static void PaintHook(id pad,SEL cmd,CGRect rect,NSString *label,BOOL pressed){
    CGRect gear=UnifiedRect(pad,12);if(fabs(CGRectGetMidX(rect)-CGRectGetMidX(gear))<1&&fabs(CGRectGetMidY(rect)-CGRectGetMidY(gear))<1)label=@"设置";
    if(!ConsolePaint(pad,rect,label,pressed))originalPaint(pad,cmd,rect,label,pressed);
}
