#import <mach/mach.h>
static char editorOverlayKey,fpsLabelKey;
@interface DuoLayoutEditor : UIView <UIPickerViewDataSource,UIPickerViewDelegate>
@property(nonatomic,weak)UIView *pad;
@property(nonatomic,strong)UIView *panel;
@property(nonatomic,strong)UIPickerView *picker;
@property(nonatomic,strong)UISlider *sizeSlider,*alphaSlider;
@property(nonatomic,strong)UILabel *values;
@property(nonatomic,strong)CAShapeLayer *selection;
@property(nonatomic)NSInteger selected;
@property(nonatomic)CGPoint fingerOffset,panelStart;
@property(nonatomic,weak)UITouch *dragTouch;
@property(nonatomic)BOOL compact;
- (void)refresh;
@end
@implementation DuoLayoutEditor
- (UIButton *)button:(NSString *)text frame:(CGRect)frame action:(SEL)action{
 UIButton *b=[UIButton buttonWithType:UIButtonTypeSystem];b.frame=frame;[b setTitle:text forState:UIControlStateNormal];b.titleLabel.font=[UIFont systemFontOfSize:12 weight:UIFontWeightMedium];b.tintColor=[UIColor colorWithRed:.94 green:.86 blue:.69 alpha:1];[b addTarget:self action:action forControlEvents:UIControlEventTouchUpInside];[self.panel addSubview:b];return b;
}
- (instancetype)initWithFrame:(CGRect)frame{
 if((self=[super initWithFrame:frame])){
  self.backgroundColor=UIColor.clearColor;self.multipleTouchEnabled=YES;
  self.panel=[[UIView alloc]initWithFrame:CGRectMake(12,MAX(12,gameWindow.safeAreaInsets.top),MIN(324,frame.size.width-24),382)];self.panel.backgroundColor=[UIColor colorWithRed:.04 green:.06 blue:.07 alpha:.96];self.panel.layer.cornerRadius=16;self.panel.layer.borderColor=[UIColor colorWithWhite:1 alpha:.20].CGColor;self.panel.layer.borderWidth=1;[self addSubview:self.panel];
  UILabel *title=[[UILabel alloc]initWithFrame:CGRectMake(12,0,220,36)];title.text=@"布局编辑 · 拖动标题移动面板";title.textColor=UIColor.whiteColor;title.font=[UIFont systemFontOfSize:12];title.userInteractionEnabled=YES;[self.panel addSubview:title];[title addGestureRecognizer:[[UIPanGestureRecognizer alloc]initWithTarget:self action:@selector(movePanel:)]];
  [self button:@"折叠" frame:CGRectMake(222,0,44,36) action:@selector(compactPanel)];[self button:@"完成" frame:CGRectMake(268,0,44,36) action:@selector(done)];
  self.picker=[[UIPickerView alloc]initWithFrame:CGRectMake(8,35,304,90)];self.picker.dataSource=self;self.picker.delegate=self;[self.panel addSubview:self.picker];
  self.values=[[UILabel alloc]initWithFrame:CGRectMake(12,124,300,22)];self.values.textColor=UIColor.lightGrayColor;self.values.font=[UIFont monospacedDigitSystemFontOfSize:11 weight:UIFontWeightRegular];[self.panel addSubview:self.values];
  self.sizeSlider=[[UISlider alloc]initWithFrame:CGRectMake(12,150,300,28)];self.sizeSlider.minimumValue=28;self.sizeSlider.maximumValue=210;[self.panel addSubview:self.sizeSlider];[self.sizeSlider addTarget:self action:@selector(slider:)forControlEvents:UIControlEventValueChanged];[self.sizeSlider addTarget:self action:@selector(save)forControlEvents:UIControlEventTouchUpInside|UIControlEventTouchUpOutside];
  self.alphaSlider=[[UISlider alloc]initWithFrame:CGRectMake(12,182,300,28)];self.alphaSlider.minimumValue=.15;self.alphaSlider.maximumValue=1;[self.panel addSubview:self.alphaSlider];[self.alphaSlider addTarget:self action:@selector(slider:)forControlEvents:UIControlEventValueChanged];[self.alphaSlider addTarget:self action:@selector(save)forControlEvents:UIControlEventTouchUpInside|UIControlEventTouchUpOutside];
  [self button:@"次级键横排" frame:CGRectMake(10,216,98,30)action:@selector(horizontal)];[self button:@"次级键竖排" frame:CGRectMake(110,216,98,30)action:@selector(vertical)];[self button:@"隐藏/显示所选" frame:CGRectMake(208,216,106,30)action:@selector(toggleSelected)];
  [self button:@"FPS 开关" frame:CGRectMake(10,248,98,30)action:@selector(fps)];[self button:@"液态玻璃开关" frame:CGRectMake(110,248,98,30)action:@selector(glass)];[self button:@"倒影：均衡" frame:CGRectMake(208,248,106,30)action:@selector(quality:)];
  [self button:@"菜单自动精简" frame:CGRectMake(10,280,145,30)action:@selector(autoMenus:)];[self button:@"重置本形态" frame:CGRectMake(164,280,145,30)action:@selector(reset)];[self button:@"导出诊断 / 原设置" frame:CGRectMake(10,312,298,30)action:@selector(diagnostics)];
  [self button:@"监测 → 游戏右上" frame:CGRectMake(10,344,145,30)action:@selector(monitorGame)];[self button:@"监测 → 键盘右上" frame:CGRectMake(164,344,145,30)action:@selector(monitorDeck)];
  for(UIView *v in self.panel.subviews)if([v isKindOfClass:UIButton.class]&&v.frame.origin.x>200&&v.frame.origin.y==248)[(UIButton *)v setTitle:@[@"倒影：关闭",@"倒影：均衡",@"倒影：流畅"][reflectionQuality]forState:UIControlStateNormal];
  self.selection=[CAShapeLayer layer];self.selection.fillColor=UIColor.clearColor.CGColor;self.selection.strokeColor=[UIColor colorWithRed:.8 green:.9 blue:1 alpha:.9].CGColor;self.selection.lineWidth=1;self.selection.lineDashPattern=nil;[self.layer insertSublayer:self.selection atIndex:0];
 }
 return self;
}
- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)picker{return 1;}
- (NSInteger)pickerView:(UIPickerView *)picker numberOfRowsInComponent:(NSInteger)component{return EditableControls().count;}
- (NSAttributedString *)pickerView:(UIPickerView *)picker attributedTitleForRow:(NSInteger)row forComponent:(NSInteger)component{NSInteger control=[EditableControls()[row]integerValue];NSString *name=ControlName(control);if([LayoutEntry(self.pad,control)[@"hidden"]boolValue])name=[name stringByAppendingString:@"（隐藏）"];return [[NSAttributedString alloc]initWithString:name attributes:@{NSForegroundColorAttributeName:UIColor.whiteColor,NSFontAttributeName:[UIFont systemFontOfSize:14]}];}
- (CGFloat)pickerView:(UIPickerView *)picker rowHeightForComponent:(NSInteger)component{return 27;}
- (void)pickerView:(UIPickerView *)picker didSelectRow:(NSInteger)row inComponent:(NSInteger)component{self.selected=[EditableControls()[row]integerValue];[self refresh];}
- (void)refresh{
 NSDictionary *e=LayoutEntry(self.pad,self.selected);self.sizeSlider.value=[e[@"size"]floatValue];self.alphaSlider.value=[e[@"alpha"]floatValue];self.values.text=[NSString stringWithFormat:@"%@  尺寸 %.0f · 透明度 %.0f%%",LayoutProfile(),self.sizeSlider.value,self.alphaSlider.value*100];
 CGRect r=UnifiedRect(self.pad,self.selected);self.selection.hidden=self.selected==14;self.selection.path=[UIBezierPath bezierPathWithRoundedRect:CGRectInset(r,-4,-4)cornerRadius:10].CGPath;
}
- (void)slider:(UISlider *)slider{NSMutableDictionary *e=LayoutEntry(self.pad,self.selected);e[@"size"]=@(self.sizeSlider.value);e[@"alpha"]=@(self.alphaSlider.value);[self.pad setNeedsDisplay];UpdateMoreButton(self.pad);UpdateFPSWidget(self.pad);[self refresh];}
- (void)save{SaveLayouts();}
- (void)placeMonitor:(BOOL)game{
 CGRect r=[self.pad convertRect:(game||!splitActive?gameRegion:controlsRegion)fromView:gameWindow];CGRect size=UnifiedRect(self.pad,14);CGFloat x=CGRectGetMaxX(r)-size.size.width/2-MAX(12,game?gameWindow.safeAreaInsets.right:0),y=CGRectGetMinY(r)+size.size.height/2+12+(!splitActive?70:0);MoveUnifiedControl(self.pad,14,CGPointMake(x,y));showFPS=YES;[NSUserDefaults.standardUserDefaults setBool:YES forKey:@"DuoFPS018"];self.selected=14;[self.picker selectRow:[EditableControls()indexOfObject:@14]inComponent:0 animated:YES];SaveLayouts();UpdateFPSWidget(self.pad);[self refresh];
}
- (void)monitorGame{[self placeMonitor:YES];}
- (void)monitorDeck{[self placeMonitor:NO];}

- (void)horizontal{AlignSecondary(self.pad,NO);[self refresh];}
- (void)vertical{AlignSecondary(self.pad,YES);[self refresh];}
- (void)toggleSelected{if(self.selected==12||self.selected==13)return;NSMutableDictionary *e=LayoutEntry(self.pad,self.selected);e[@"hidden"]=@(![e[@"hidden"]boolValue]);[self.pad setNeedsDisplay];[self.picker reloadAllComponents];SaveLayouts();}
- (void)fps{showFPS=!showFPS;[NSUserDefaults.standardUserDefaults setBool:showFPS forKey:@"DuoFPS018"];UpdateFPSWidget(self.pad);}
- (void)glass{consoleGlassEnabled=!consoleGlassEnabled;[NSUserDefaults.standardUserDefaults setBool:consoleGlassEnabled forKey:@"DuoConsoleGlass017"];[self.pad setNeedsDisplay];UpdateMoreButton(self.pad);}
- (void)quality:(UIButton *)b{reflectionQuality=(reflectionQuality+1)%3;[NSUserDefaults.standardUserDefaults setInteger:reflectionQuality forKey:@"DuoReflectionQuality018"];[b setTitle:@[@"倒影：关闭",@"倒影：均衡",@"倒影：流畅"][reflectionQuality]forState:UIControlStateNormal];if(!reflectionQuality)reflectedGameImage=nil;UpdateFoldLayout();}
- (void)autoMenus:(UIButton *)b{automaticMenus=!automaticMenus;[NSUserDefaults.standardUserDefaults setBool:automaticMenus forKey:@"DuoAutoMenus018"];[b setTitle:automaticMenus?@"菜单自动精简：开":@"菜单自动精简：关"forState:UIControlStateNormal];[self.pad setNeedsDisplay];}
- (void)reset{[userLayoutProfiles removeObjectForKey:LayoutProfile()];LayoutEntries(self.pad);SaveLayouts();[self.pad setNeedsDisplay];UpdateMoreButton(self.pad);[self refresh];}
- (void)compactPanel{self.compact=!self.compact;for(UIView *v in self.panel.subviews)if(v.frame.origin.y>=35)v.hidden=self.compact;CGRect r=self.panel.frame;r.size.height=self.compact?36:382;self.panel.frame=r;}
- (void)movePanel:(UIPanGestureRecognizer *)p{
 if(p.state==UIGestureRecognizerStateBegan)self.panelStart=self.panel.center;CGPoint t=[p translationInView:self];CGPoint c=CGPointMake(self.panelStart.x+t.x,self.panelStart.y+t.y);c.x=MAX(self.panel.bounds.size.width/2,MIN(self.bounds.size.width-self.panel.bounds.size.width/2,c.x));c.y=MAX(self.panel.bounds.size.height/2,MIN(self.bounds.size.height-self.panel.bounds.size.height/2,c.y));self.panel.center=c;
}
- (void)done{UIView *pad=self.pad;customEditing=NO;SaveLayouts();objc_setAssociatedObject(pad,&editorOverlayKey,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);[self removeFromSuperview];[(id)pad clearKeys];[pad setNeedsDisplay];UpdateMoreButton(pad);UpdateFPSWidget(pad);UpdateFoldLayout();}
- (void)diagnostics{UIView *pad=self.pad;[self done];if(originalSettings)originalSettings(pad,NSSelectorFromString(@"openSettings"));}
- (void)layoutSubviews{[super layoutSubviews];CGRect r=self.panel.frame;r.origin.x=MAX(0,MIN(self.bounds.size.width-r.size.width,r.origin.x));r.origin.y=MAX(0,MIN(self.bounds.size.height-r.size.height,r.origin.y));self.panel.frame=r;if(self.pad)[self refresh];}
- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event{
 UITouch *touch=touches.anyObject;CGPoint p=[touch locationInView:self];NSInteger found=-1;
 for(NSNumber *n in [EditableControls()reverseObjectEnumerator]){NSInteger i=n.integerValue;if(CGRectContainsPoint(CGRectInset(UnifiedRect(self.pad,i),-5,-5),p)){found=i;break;}}
 if(found<0)return;self.selected=found;CGRect r=UnifiedRect(self.pad,found);self.fingerOffset=CGPointMake(CGRectGetMidX(r)-p.x,CGRectGetMidY(r)-p.y);self.dragTouch=touch;[self.picker selectRow:[EditableControls()indexOfObject:@(found)]inComponent:0 animated:YES];[self refresh];
}
- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event{if(!self.dragTouch)return;CGPoint p=[self.dragTouch locationInView:self];MoveUnifiedControl(self.pad,self.selected,CGPointMake(p.x+self.fingerOffset.x,p.y+self.fingerOffset.y));[self refresh];}
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event{if([touches containsObject:self.dragTouch]){self.dragTouch=nil;SaveLayouts();}}
- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event{self.dragTouch=nil;SaveLayouts();}
@end
static void OpenUnifiedEditor(UIView *pad){
 if(customEditing)return;customEditing=YES;[(id)pad clearKeys];DuoLayoutEditor *editor=[[DuoLayoutEditor alloc]initWithFrame:pad.bounds];editor.pad=pad;editor.selected=0;editor.autoresizingMask=UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;[pad addSubview:editor];objc_setAssociatedObject(pad,&editorOverlayKey,editor,OBJC_ASSOCIATION_RETAIN_NONATOMIC);[editor refresh];[pad setNeedsDisplay];UpdateFoldLayout();
}
static void UnifiedSettingsHook(id self,SEL cmd){OpenUnifiedEditor(self);}
static void BringEditorFront(UIView *pad){UIView *editor=objc_getAssociatedObject(pad,&editorOverlayKey);if(editor)[pad bringSubviewToFront:editor];}
static void UpdateEditorGeometry(void){DuoLayoutEditor *editor=objc_getAssociatedObject(activePad,&editorOverlayKey);if(editor){editor.frame=activePad.bounds;[editor refresh];}}
static void UpdateFPSWidget(UIView *pad){
 UILabel *label=objc_getAssociatedObject(pad,&fpsLabelKey);if(!label){label=[UILabel new];label.userInteractionEnabled=NO;label.font=[UIFont monospacedDigitSystemFontOfSize:10 weight:UIFontWeightMedium];label.numberOfLines=5;label.textAlignment=NSTextAlignmentCenter;label.textColor=[UIColor colorWithRed:.85 green:.95 blue:.9 alpha:1];label.backgroundColor=[UIColor colorWithWhite:.02 alpha:.72];label.layer.cornerRadius=7;label.clipsToBounds=YES;[pad addSubview:label];objc_setAssociatedObject(pad,&fpsLabelKey,label,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
 label.frame=UnifiedRect(pad,14);label.font=[UIFont monospacedDigitSystemFontOfSize:MAX(9,MIN(16,MIN(label.bounds.size.width/22,label.bounds.size.height/6.5)))weight:UIFontWeightMedium];label.alpha=[LayoutEntry(pad,14)[@"alpha"]doubleValue];label.hidden=(!showFPS||[LayoutEntry(pad,14)[@"hidden"]boolValue])&&!customEditing;label.text=MonitorText();BringEditorFront(pad);
}
@interface DuoFPSClock : NSObject
- (void)tick:(NSTimer *)timer;
@end
@implementation DuoFPSClock
- (void)tick:(NSTimer *)timer{static CFTimeInterval last;CFTimeInterval now=CACurrentMediaTime();SamplePerformance(last?now-last:0,UIApplication.sharedApplication.applicationState==UIApplicationStateActive);last=now;if(activePad)UpdateFPSWidget(activePad);}
@end
static void StartFPSClock(void){static NSTimer *timer;if(timer)return;DuoFPSClock *clock=[DuoFPSClock new];timer=[NSTimer timerWithTimeInterval:1 target:clock selector:@selector(tick:)userInfo:nil repeats:YES];[[NSRunLoop mainRunLoop]addTimer:timer forMode:NSRunLoopCommonModes];}
