#include "ConsoleDrawing.h"
static BOOL consoleGlassEnabled=YES;
static char consoleFacesKey,consoleSeenKey,consoleDeckKey,buttonGlassKey,buttonGlassEnabledKey,consoleBornKey,buttonGlyphKey;
static UIImage *ConsoleIcon(NSString *label,CGFloat size){
 NSDictionary *icons=@{@"跳跃":@"arrow.up",@"攻击":@"pencil.tip",@"冲刺":@"wind",@"缠丝":@"hurricane",@"缝丝":@"hurricane",@"技能":@"sparkles",@"工具":@"wrench.and.screwdriver",@"地图":@"map",@"背包":@"bag",@"暂停":@"pause.fill",@"设置":@"gearshape",@"返回":@"arrow.uturn.backward",@"更多":@"ellipsis"};
 return [UIImage systemImageNamed:icons[label]?:@"circle" withConfiguration:[UIImageSymbolConfiguration configurationWithPointSize:size weight:UIImageSymbolWeightLight]];
}
static UIVisualEffect *ConsoleGlassEffect(void){
 if(!consoleGlassEnabled||UIAccessibilityIsReduceTransparencyEnabled())return nil;
 if(@available(iOS 26.0,*)){UIGlassEffect *effect=[UIGlassEffect effectWithStyle:UIGlassEffectStyleRegular];effect.interactive=YES;effect.tintColor=[UIColor colorWithRed:.13 green:.18 blue:.18 alpha:.20];return effect;}
 return [UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemUltraThinMaterialDark];
}
@interface DuoFaceArtwork : UIView
@property(nonatomic,copy)NSString *label;
@property(nonatomic)BOOL pressed,joystick,menuMode;
@property(nonatomic)CGPoint knob;
@end
@implementation DuoFaceArtwork
- (void)drawRect:(CGRect)rect{
 CGContextRef c=UIGraphicsGetCurrentContext();CGFloat side=MIN(self.bounds.size.width,self.bounds.size.height);
 if(consoleGlassEnabled){
  DuoStroke(c,.90,.95,.96,self.pressed?.75:.42,.8);CGContextStrokeEllipseInRect(c,CGRectInset(self.bounds,2,2));DuoStroke(c,1,1,1,.16,.5);CGContextStrokeEllipseInRect(c,CGRectInset(self.bounds,5,5));DuoStroke(c,1,1,1,.55,1.1);CGContextAddArc(c,side/2,side/2,side/2-2,M_PI,2*M_PI,0);CGContextStrokePath(c);
 }else DuoBezel(c,self.bounds,self.pressed,self.joystick);
 UIColor *ink=[UIColor colorWithRed:.93 green:.87 blue:.73 alpha:1];
 if(self.joystick){
  CGFloat cx=side/2,cy=side/2;
  for(int i=0;i<4;i++){CGFloat a=i*M_PI_2-M_PI_2,dx=cos(a),dy=sin(a);UIBezierPath *p=[UIBezierPath bezierPath];[p moveToPoint:CGPointMake(cx+dx*side*.39,cy+dy*side*.39)];[p addLineToPoint:CGPointMake(cx+dx*side*.31-dy*side*.025,cy+dy*side*.31-dx*side*.025)];[p addLineToPoint:CGPointMake(cx+dx*side*.31+dy*side*.025,cy+dy*side*.31+dx*side*.025)];[p closePath];[[ink colorWithAlphaComponent:.8]setFill];[p fill];}
  CGRect knob=CGRectMake(side*.30+self.knob.x,side*.30+self.knob.y,side*.40,side*.40);
  UIBezierPath *cap=[UIBezierPath bezierPathWithOvalInRect:knob];
  [[UIColor colorWithRed:.20 green:.24 blue:.25 alpha:consoleGlassEnabled?.55:.95]setFill];[cap fill];[[ink colorWithAlphaComponent:.50]setStroke];cap.lineWidth=.8;[cap stroke];
  [[ink colorWithAlphaComponent:.15]setStroke];UIBezierPath *inner=[UIBezierPath bezierPathWithOvalInRect:CGRectInset(knob,4,4)];inner.lineWidth=.6;[inner stroke];
  DuoStroke(c,.95,.90,.76,.55,.6);DuoDiamond(c,CGRectGetMidX(knob),CGRectGetMidY(knob),2);
 }else{
  BOOL action=[@[@"攻击",@"跳跃",@"冲刺",@"缠丝",@"缝丝",@"技能",@"工具"]containsObject:self.label];
  CGFloat iconSide=side*(action?.43:.45);CGRect iconRect=CGRectMake((side-iconSide)/2,side*(action?.18:.27),iconSide,iconSide);
  UIImage *image=[[ConsoleIcon(self.label,iconSide) imageWithTintColor:ink renderingMode:UIImageRenderingModeAlwaysOriginal]imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
  if([self.label isEqual:@"攻击"])DuoNeedleIcon(c,iconRect);else [image drawInRect:iconRect];
  if(action){NSString *caption=[self.label isEqual:@"攻击"]?(menuContext&&!physicalFold?@"确认":@"攻 / 确"):self.label;CGFloat font=MAX(8,MIN(10,side*.16));NSDictionary *a=@{NSFontAttributeName:[UIFont systemFontOfSize:font weight:UIFontWeightRegular],NSForegroundColorAttributeName:[ink colorWithAlphaComponent:.78]};CGSize s=[caption sizeWithAttributes:a];[caption drawAtPoint:CGPointMake((side-s.width)/2,side*.65)withAttributes:a];}
 }
}
@end
@interface DuoControlFace : UIView
@property(nonatomic,strong)UIVisualEffectView *glass;
@property(nonatomic,strong)DuoFaceArtwork *art;
@property(nonatomic)BOOL glassOn;
@end
@implementation DuoControlFace
- (instancetype)initWithFrame:(CGRect)frame{
 if((self=[super initWithFrame:frame])){self.userInteractionEnabled=NO;self.backgroundColor=UIColor.clearColor;self.opaque=NO;self.glass=[[UIVisualEffectView alloc]initWithEffect:ConsoleGlassEffect()];self.glass.userInteractionEnabled=NO;self.glass.clipsToBounds=YES;[self addSubview:self.glass];self.art=[DuoFaceArtwork new];self.art.opaque=NO;self.art.backgroundColor=UIColor.clearColor;self.art.userInteractionEnabled=NO;[self addSubview:self.art];self.glassOn=consoleGlassEnabled;}
 return self;
}
- (void)layoutSubviews{[super layoutSubviews];self.glass.frame=CGRectInset(self.bounds,6,6);self.glass.layer.cornerRadius=MIN(self.glass.bounds.size.width,self.glass.bounds.size.height)/2;if(!CGRectEqualToRect(self.art.frame,self.bounds)){self.art.frame=self.bounds;[self.art setNeedsDisplay];}}
- (void)drawRect:(CGRect)rect{
 CGContextRef c=UIGraphicsGetCurrentContext();CGColorSpaceRef space=CGColorSpaceCreateDeviceRGB();CGFloat colors[]={.12,.15,.15,consoleGlassEnabled?.22:.96,.025,.04,.045,consoleGlassEnabled?.38:.98};CGGradientRef g=CGGradientCreateWithColorComponents(space,colors,NULL,2);CGContextSaveGState(c);CGContextAddEllipseInRect(c,CGRectInset(self.bounds,4,4));CGContextClip(c);CGContextDrawLinearGradient(c,g,CGPointZero,CGPointMake(0,self.bounds.size.height),0);CGContextRestoreGState(c);CGGradientRelease(g);CGColorSpaceRelease(space);
}
@end
@interface DuoDeckEtching : UIView @end
@implementation DuoDeckEtching
- (void)drawRect:(CGRect)r{CGContextRef c=UIGraphicsGetCurrentContext();DuoConsoleFrame(c,self.bounds);DuoStroke(c,.70,.60,.42,.20,1);CGFloat width=MIN(46,r.size.width*.09);for(int side=0;side<2;side++)for(int i=0;i<4;i++){CGFloat x=side?r.size.width-28-width:28;DuoLine(c,x,23+i*3,x+width,23+i*3);}}
@end
@interface DuoConsoleDeck : UIView
@property(nonatomic,strong)UIImage *reflection;
@property(nonatomic,strong)UIImageView *liveImage;
@property(nonatomic,strong)CAGradientLayer *base,*shade;
@property(nonatomic,strong)UIView *shadeView;
@property(nonatomic,strong)DuoDeckEtching *etch;
@end
@implementation DuoConsoleDeck
- (instancetype)initWithFrame:(CGRect)frame{if((self=[super initWithFrame:frame])){self.base=[CAGradientLayer layer];self.base.colors=@[(id)[UIColor colorWithRed:.055 green:.075 blue:.080 alpha:1].CGColor,(id)[UIColor colorWithRed:.012 green:.025 blue:.03 alpha:1].CGColor];[self.layer addSublayer:self.base];self.liveImage=[UIImageView new];self.liveImage.contentMode=UIViewContentModeScaleAspectFill;self.liveImage.clipsToBounds=YES;self.liveImage.alpha=.53;self.liveImage.transform=CGAffineTransformMakeScale(1,-1);[self addSubview:self.liveImage];self.shadeView=[UIView new];self.shadeView.userInteractionEnabled=NO;[self addSubview:self.shadeView];self.shade=[CAGradientLayer layer];self.shade.colors=@[(id)[UIColor colorWithWhite:.02 alpha:.28].CGColor,(id)[UIColor colorWithWhite:.02 alpha:.80].CGColor];[self.shadeView.layer addSublayer:self.shade];self.etch=[DuoDeckEtching new];self.etch.backgroundColor=UIColor.clearColor;self.etch.opaque=NO;self.etch.userInteractionEnabled=NO;[self addSubview:self.etch];}return self;}
- (void)setReflection:(UIImage *)reflection{_reflection=reflection;self.liveImage.image=reflection;}
- (void)layoutSubviews{[super layoutSubviews];[CATransaction begin];[CATransaction setDisableActions:YES];self.base.frame=self.bounds;self.liveImage.bounds=(CGRect){CGPointZero,self.bounds.size};self.liveImage.center=CGPointMake(CGRectGetMidX(self.bounds),CGRectGetMidY(self.bounds));self.shadeView.frame=self.bounds;self.shade.frame=self.bounds;if(!CGRectEqualToRect(self.etch.frame,self.bounds)){self.etch.frame=self.bounds;[self.etch setNeedsDisplay];}[CATransaction commit];}
@end
static NSMutableDictionary *ConsoleFaces(UIView *pad){NSMutableDictionary *d=objc_getAssociatedObject(pad,&consoleFacesKey);if(!d){d=[NSMutableDictionary dictionary];objc_setAssociatedObject(pad,&consoleFacesKey,d,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}return d;}
static void ConsoleUpdateDeck(UIView *pad){
 DuoConsoleDeck *deck=objc_getAssociatedObject(pad,&consoleDeckKey);
 if(!deck){deck=[DuoConsoleDeck new];deck.userInteractionEnabled=NO;deck.opaque=YES;deck.clipsToBounds=YES;[pad insertSubview:deck atIndex:0];objc_setAssociatedObject(pad,&consoleDeckKey,deck,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
 deck.hidden=!splitActive||[(id)pad editingLayout]||[(id)pad collapsed];
 if(!deck.hidden){CGRect background=controlsRegion;if(physicalFold){background.origin.y=CGRectGetMaxY(gameRegion);background.size.height=CGRectGetMaxY(gameWindow.bounds)-background.origin.y;}CGRect frame=[pad convertRect:background fromView:gameWindow];if(!CGRectEqualToRect(deck.frame,frame)){deck.frame=frame;[deck setNeedsDisplay];}
 UIImage *image=reflectedGameImage;if(deck.reflection!=image){deck.reflection=image;}}
}
static DuoControlFace *ConsoleFace(UIView *pad,NSString *key,CGRect rect,BOOL pressed,BOOL joystick){
 NSMutableDictionary *faces=ConsoleFaces(pad);DuoControlFace *face=faces[key];
 if(!face){face=[[DuoControlFace alloc]initWithFrame:rect];faces[key]=face;[pad addSubview:face];[face.art setNeedsDisplay];if(!customEditing)objc_setAssociatedObject(pad,&consoleBornKey,@YES,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
 if(!CGRectEqualToRect(face.frame,rect)){face.frame=rect;[face setNeedsLayout];}
 face.hidden=NO;if(![face.art.label isEqualToString:key]||face.art.joystick!=joystick){face.art.label=key;face.art.joystick=joystick;[face.art setNeedsDisplay];}
 BOOL simpleMenu=menuContext&&!physicalFold;if(face.art.menuMode!=simpleMenu){face.art.menuMode=simpleMenu;[face.art setNeedsDisplay];}
 if(face.glassOn!=consoleGlassEnabled){face.glassOn=consoleGlassEnabled;face.glass.effect=ConsoleGlassEffect();[face setNeedsDisplay];}
 if(face.art.pressed!=pressed){if(pressed&&([@[@"背包",@"地图",@"暂停"]containsObject:key]||(menuContext&&[key isEqual:@"攻击"])))dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(.12*NSEC_PER_SEC)),dispatch_get_main_queue(),^{SceneFeedbackPulse();});face.art.pressed=pressed;[face.art setNeedsDisplay];if(!joystick&&!UIAccessibilityIsReduceMotionEnabled()){CABasicAnimation *a=[CABasicAnimation animationWithKeyPath:@"sublayerTransform.scale"];a.fromValue=@(pressed?1:.95);a.toValue=@(pressed?.95:1);a.duration=.12;face.layer.sublayerTransform=CATransform3DMakeScale(pressed?.95:1,pressed?.95:1,1);[face.layer addAnimation:a forKey:@"press"];}}
 [objc_getAssociatedObject(pad,&consoleSeenKey)addObject:key];return face;
}
static void ConsoleBegin(UIView *pad){activePad=pad;ConsoleUpdateDeck(pad);objc_setAssociatedObject(pad,&consoleSeenKey,[NSMutableSet set],OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
static BOOL ConsolePaint(UIView *pad,CGRect rect,NSString *label,BOOL pressed){
 if([(id)pad editingLayout])return NO;CGFloat size=MIN(rect.size.width,rect.size.height);CGRect r=CGRectMake(CGRectGetMidX(rect)-size/2,CGRectGetMidY(rect)-size/2,size,size);DuoControlFace *face=ConsoleFace(pad,label,r,pressed,NO);
 NSDictionary *indices=@{@"跳跃":@1,@"攻击":@2,@"冲刺":@3,@"缠丝":@4,@"缝丝":@4,@"技能":@5,@"工具":@6,@"背包":@7,@"地图":@8,@"暂停":@9,@"设置":@12};
 NSNumber *index=indices[label];if(index){NSDictionary *entry=[(id)pad entry:index.integerValue];NSNumber *alpha=entry[@"alpha"];if(alpha)face.alpha=MAX(.1,MIN(1,alpha.doubleValue));}
 return YES;
}
static void ConsoleGather(UIView *pad){
 if(customEditing||UIApplication.sharedApplication.applicationState!=UIApplicationStateActive)return;
 NSMutableArray<UIView *> *faces=[ConsoleFaces(pad).allValues mutableCopy];for(UIView *v in pad.subviews)if([v isKindOfClass:UIButton.class])[faces addObject:v];
 for(UIView *face in faces){if(face.hidden)continue;CABasicAnimation *fade=[CABasicAnimation animationWithKeyPath:@"opacity"];fade.fromValue=@0;fade.toValue=@1;fade.duration=.32;[face.layer addAnimation:fade forKey:@"entryFade"];}
 if(UIAccessibilityIsReduceMotionEnabled()||!faces.count)return;
 CALayer *particles=[CALayer layer];particles.frame=pad.bounds;[pad.layer addSublayer:particles];
 for(int i=0;i<18;i++){UIView *target=faces[i%faces.count];if(target.hidden)continue;CALayer *dot=[CALayer layer];dot.bounds=CGRectMake(0,0,1.8,1.8);dot.cornerRadius=.9;dot.backgroundColor=[UIColor colorWithRed:.90 green:.95 blue:.96 alpha:.85].CGColor;CGPoint end=target.center;CGFloat angle=i*2.399963;CGPoint start=CGPointMake(end.x+cos(angle)*55,end.y+sin(angle)*55);dot.position=end;[particles addSublayer:dot];CABasicAnimation *move=[CABasicAnimation animationWithKeyPath:@"position"];move.fromValue=[NSValue valueWithCGPoint:start];move.toValue=[NSValue valueWithCGPoint:end];CABasicAnimation *fade=[CABasicAnimation animationWithKeyPath:@"opacity"];fade.fromValue=@.9;fade.toValue=@0;CAAnimationGroup *group=[CAAnimationGroup animation];group.animations=@[move,fade];group.duration=.42;group.timingFunction=[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];dot.opacity=0;[dot addAnimation:group forKey:@"gather"];}
 dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(.46*NSEC_PER_SEC)),dispatch_get_main_queue(),^{[particles removeFromSuperlayer];});
}
static void ConsoleEnd(UIView *pad){
 if(![(id)pad editingLayout]&&![(id)pad collapsed]&&![(id)pad hiddenControl:0]){
  CGRect rect=[(id)pad padRect];DuoControlFace *face=ConsoleFace(pad,@"joystick",rect,NO,YES);face.alpha=MAX(.1,MIN(1,[[(id)pad entry:0][@"alpha"]doubleValue]));CGPoint offset=CGPointZero;id finger=objc_getAssociatedObject(pad,&joystickFingerKey);NSValue *value=[(id)pad fingers][finger?:@""];
  if(value){CGPoint p=value.CGPointValue;CGFloat x=p.x-CGRectGetMidX(rect),y=p.y-CGRectGetMidY(rect),scale=MIN(1,rect.size.width*.23/MAX(1,hypot(x,y)));offset=CGPointMake(x*scale,y*scale);}
  if(!CGPointEqualToPoint(face.art.knob,offset)){face.art.knob=offset;[face.art setNeedsDisplay];}
 }
 BringEditorFront(pad);
 NSSet *seen=objc_getAssociatedObject(pad,&consoleSeenKey);[ConsoleFaces(pad)enumerateKeysAndObjectsUsingBlock:^(NSString *key,UIView *face,BOOL *stop){face.hidden=![seen containsObject:key];}];
 if([objc_getAssociatedObject(pad,&consoleBornKey)boolValue]){objc_setAssociatedObject(pad,&consoleBornKey,@NO,OBJC_ASSOCIATION_RETAIN_NONATOMIC);ConsoleGather(pad);}
}
static void ConsoleStyleButton(UIButton *button){
 UIVisualEffectView *effect=objc_getAssociatedObject(button,&buttonGlassKey);
 if(!effect){effect=[[UIVisualEffectView alloc]initWithEffect:ConsoleGlassEffect()];effect.userInteractionEnabled=NO;effect.clipsToBounds=YES;[button insertSubview:effect atIndex:0];objc_setAssociatedObject(button,&buttonGlassKey,effect,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
 effect.frame=CGRectInset(button.bounds,2,2);effect.layer.cornerRadius=effect.bounds.size.width/2;
 NSNumber *old=objc_getAssociatedObject(button,&buttonGlassEnabledKey);if(!old||old.boolValue!=consoleGlassEnabled){effect.effect=ConsoleGlassEffect();objc_setAssociatedObject(button,&buttonGlassEnabledKey,@(consoleGlassEnabled),OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
 UIImageView *glyph=objc_getAssociatedObject(button,&buttonGlyphKey);if(!glyph){glyph=[UIImageView new];glyph.userInteractionEnabled=NO;glyph.contentMode=UIViewContentModeScaleAspectFit;[button addSubview:glyph];objc_setAssociatedObject(button,&buttonGlyphKey,glyph,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
 glyph.image=[UIImage systemImageNamed:[button.accessibilityLabel isEqualToString:@"返回"]?@"arrow.uturn.backward":@"ellipsis" withConfiguration:[UIImageSymbolConfiguration configurationWithPointSize:18 weight:UIImageSymbolWeightMedium]];glyph.tintColor=[UIColor colorWithWhite:.95 alpha:1];glyph.frame=CGRectInset(button.bounds,button.bounds.size.width*.25,button.bounds.size.height*.25);[button bringSubviewToFront:glyph];button.imageView.hidden=YES;button.layer.cornerRadius=MIN(button.bounds.size.width,button.bounds.size.height)/2;
 button.backgroundColor=[UIColor colorWithRed:.05 green:.075 blue:.08 alpha:consoleGlassEnabled?.2:.95];
 button.layer.borderColor=[UIColor colorWithRed:.72 green:.60 blue:.40 alpha:.75].CGColor;button.layer.borderWidth=.8;
}
