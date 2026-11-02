#import <ImageIO/ImageIO.h>
// These three typed calls are bound to the same exact engine UUID as the proven HUD bridge.
static void *(*brandStringNew)(const char *);
static void *(*brandFind)(void *,void *);
static void *(*brandTransform)(void *,void *);
static void (*brandScale)(void *,DuoVector3,void *);
static NSUInteger brandingIntroMatches,brandingTitleMatches;
static BOOL brandingIntroPlayed,brandingSearchFinished;
static CFTimeInterval brandingIntroTime,brandingLastProbe;
static UIImage *BrandImage(void){
 NSURL *url=[NSBundle.mainBundle URLForResource:@"FakeLuStudio" withExtension:@"jpg"];if(!url)return nil;CGImageSourceRef source=CGImageSourceCreateWithURL((__bridge CFURLRef)url,NULL);if(!source)return nil;
 NSDictionary *options=@{(id)kCGImageSourceCreateThumbnailFromImageAlways:@YES,(id)kCGImageSourceThumbnailMaxPixelSize:@720,(id)kCGImageSourceCreateThumbnailWithTransform:@YES};CGImageRef cg=CGImageSourceCreateThumbnailAtIndex(source,0,(__bridge CFDictionaryRef)options);UIImage *image=cg?[UIImage imageWithCGImage:cg]:nil;if(cg)CGImageRelease(cg);CFRelease(source);return image;
}
static void BrandAnimate(CALayer *layer,NSString *key,NSArray *values,NSArray *times,CGFloat duration,CGFloat delay){
 CAKeyframeAnimation *a=[CAKeyframeAnimation animationWithKeyPath:key];a.values=values;a.keyTimes=times;a.duration=duration;a.beginTime=CACurrentMediaTime()+delay;a.fillMode=kCAFillModeBackwards;[layer addAnimation:a forKey:key];
}
@interface DuoBrandIntro:UIView
@property(nonatomic,strong)UIView *art;
@property(nonatomic,strong)UIImageView *image;
@property(nonatomic,strong)NSArray<CALayer *> *eyes;
@property(nonatomic,strong)NSArray<CALayer *> *bars;
@property(nonatomic,strong)CATextLayer *caption;
@property(nonatomic)BOOL animating;
- (void)start;
@end
@implementation DuoBrandIntro
- (instancetype)initWithFrame:(CGRect)frame{
 if((self=[super initWithFrame:frame])){self.userInteractionEnabled=NO;self.backgroundColor=[UIColor colorWithRed:.008 green:.014 blue:.027 alpha:1];self.clipsToBounds=YES;
 self.art=[UIView new];self.art.userInteractionEnabled=NO;self.art.clipsToBounds=YES;[self addSubview:self.art];self.image=[[UIImageView alloc]initWithImage:BrandImage()];self.image.contentMode=UIViewContentModeScaleAspectFit;[self.art addSubview:self.image];NSMutableArray *eyes=[NSMutableArray array],*bars=[NSMutableArray array];
 for(int i=0;i<2;i++){CALayer *eye=[CALayer layer];eye.backgroundColor=[UIColor colorWithRed:.005 green:.008 blue:.022 alpha:1].CGColor;eye.opacity=0;eye.cornerRadius=3;[self.art.layer addSublayer:eye];[eyes addObject:eye];}self.eyes=eyes;
 for(int i=0;i<6;i++){CALayer *bar=[CALayer layer];bar.backgroundColor=(i%2?[UIColor colorWithRed:1 green:.04 blue:.82 alpha:.65]:[UIColor colorWithRed:.03 green:.9 blue:1 alpha:.7]).CGColor;bar.opacity=0;[self.art.layer addSublayer:bar];[bars addObject:bar];}self.bars=bars;
 self.caption=[CATextLayer layer];self.caption.string=@"FAKE LU  //  GAME STUDIO ONLINE";self.caption.fontSize=12;self.caption.alignmentMode=kCAAlignmentCenter;self.caption.foregroundColor=[UIColor colorWithRed:.3 green:.95 blue:1 alpha:1].CGColor;self.caption.contentsScale=UIScreen.mainScreen.scale;[self.layer addSublayer:self.caption];
 }return self;
}
- (void)layoutSubviews{
 [super layoutSubviews];CGFloat w=MIN(self.bounds.size.width,self.bounds.size.height*1.7927),h=w/1.7927;self.art.bounds=CGRectMake(0,0,w,h);self.art.center=CGPointMake(self.bounds.size.width/2,self.bounds.size.height/2-8);self.image.frame=self.art.bounds;
 self.eyes[0].frame=CGRectMake(w*.452,h*.279,w*.050,h*.080);self.eyes[1].frame=CGRectMake(w*.514,h*.279,w*.048,h*.080);
 for(int i=0;i<6;i++)self.bars[i].frame=CGRectMake(w*(.18+.09*i),h*(.12+.13*i),w*(.10+.02*(i%3)),MAX(1,h*.004));self.caption.frame=CGRectMake(8,MIN(self.bounds.size.height-26,CGRectGetMaxY(self.art.frame)+8),self.bounds.size.width-16,18);
}
- (void)start{
 if(self.animating)return;self.animating=YES;[self setNeedsLayout];[self layoutIfNeeded];
 BrandAnimate(self.layer,@"opacity",@[@0,@1],@[@0,@1],.45,0);
 if(UIAccessibilityIsReduceMotionEnabled())return;
 BrandAnimate(self.art.layer,@"transform.translation.x",@[@0,@0,@(-4),@3,@0,@0,@2,@(-2),@0],@[@0,@.20,@.215,@.23,@.25,@.60,@.615,@.63,@1],3.4,0);
 BrandAnimate(self.art.layer,@"opacity",@[@1,@1,@.60,@1,@1,@.78,@1],@[@0,@.20,@.215,@.24,@.60,@.615,@1],3.4,0);
 for(int i=0;i<2;i++)BrandAnimate(self.eyes[i],@"opacity",@[@0,@0,@.95,@0,@0,@.75,@0],@[@0,@.18,@.20,@.24,@.64,@.67,@1],2.8,i*.14);
 for(int i=0;i<6;i++){BrandAnimate(self.bars[i],@"opacity",@[@0,@0,@.9,@0,@0,@.7,@0],@[@0,@.19,@.21,@.25,@.58,@.61,@1],3.1,i*.045);BrandAnimate(self.bars[i],@"transform.translation.x",@[@0,@(-12),@18,@0],@[@0,@.21,@.24,@1],3.1,i*.045);}
 CALayer *mask=[CALayer layer];mask.anchorPoint=CGPointZero;mask.position=CGPointZero;mask.backgroundColor=UIColor.whiteColor.CGColor;mask.bounds=self.caption.bounds;self.caption.mask=mask;CABasicAnimation *type=[CABasicAnimation animationWithKeyPath:@"bounds.size.width"];type.fromValue=@0;type.toValue=@(self.caption.bounds.size.width);type.duration=1.7;type.timingFunction=[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionLinear];[mask addAnimation:type forKey:@"typeIn"];
}
@end
static DuoBrandIntro *brandIntro;
static UIView *brandTitleTag;
static BOOL HideBrandObject(const char *name){
 void *str=brandStringNew(name);if(!str)return NO;void *object=brandFind(str,NULL);if(!HudObjectAlive(object))return NO;void *transform=brandTransform(object,NULL);if(!HudObjectAlive(transform))return NO;brandScale(transform,(DuoVector3){0,0,0},NULL);return YES;
}
static void PollBranding(NSString *scene,BOOL title,BOOL playing){
 if(!hudEngineBase||!PrepareHudAPI())return;CFTimeInterval now=CACurrentMediaTime();BOOL introScene=[scene.lowercaseString containsString:@"pre_menu_intro"];
 if(!brandStringNew){brandStringNew=dlsym(RTLD_DEFAULT,"il2cpp_string_new");brandFind=(void *)(hudEngineBase+0x91881d4);brandTransform=(void *)(hudEngineBase+0x9185bc8);brandScale=(void *)(hudEngineBase+0x91bd748);}if(!brandStringNew)return;
 if(playing)brandingSearchFinished=YES;BOOL lookIntro=(!brandingSearchFinished&&!brandingIntroPlayed)||introScene;BOOL introFound=NO,titleFound=NO;
 if((lookIntro||title)&&now-brandingLastProbe>(lookIntro?.15:.8)){brandingLastProbe=now;if(lookIntro)introFound=HideBrandObject("1 - Team Cherry Logo");if(title)titleFound=HideBrandObject("TeamCherryLogo");if(introFound)brandingIntroMatches++;if(titleFound){brandingTitleMatches++;brandingSearchFinished=YES;}}
 if((introFound||introScene)&&!brandingIntroPlayed){brandingIntroPlayed=YES;brandingIntroTime=now;brandIntro=[[DuoBrandIntro alloc]initWithFrame:gameRegion];[gameWindow addSubview:brandIntro];[brandIntro start];}
 if(brandIntro){brandIntro.frame=gameRegion;[gameWindow bringSubviewToFront:brandIntro];if(now-brandingIntroTime>4.2||(!introScene&&title&&now-brandingIntroTime>2.4)){DuoBrandIntro *ending=brandIntro;brandIntro=nil;[UIView animateWithDuration:.28 animations:^{ending.alpha=0;}completion:^(BOOL finished){[ending removeFromSuperview];}];}}
 if(title&&brandingTitleMatches){
  if(!brandTitleTag){UIView *tag=[UIView new];tag.userInteractionEnabled=NO;tag.backgroundColor=UIColor.clearColor;UILabel *label=[UILabel new];label.frame=CGRectMake(0,0,155,19);label.text=@"FAKE LU";label.font=[UIFont monospacedSystemFontOfSize:18 weight:UIFontWeightBold];label.textColor=[UIColor colorWithRed:.3 green:.95 blue:1 alpha:1];label.layer.shadowColor=[UIColor magentaColor].CGColor;label.layer.shadowRadius=3;label.layer.shadowOpacity=.6;label.layer.shadowOffset=CGSizeMake(1,0);[tag addSubview:label];UILabel *studio=[UILabel new];studio.frame=CGRectMake(0,21,155,14);studio.text=@"GAME STUDIO";studio.font=[UIFont monospacedSystemFontOfSize:9 weight:UIFontWeightMedium];studio.textColor=[UIColor colorWithWhite:.9 alpha:.9];[tag addSubview:studio];[gameWindow addSubview:tag];brandTitleTag=tag;BrandAnimate(tag.layer,@"opacity",@[@0,@1,@.45,@1],@[@0,@.65,@.72,@1],.9,0);}
  CGFloat scale=MIN(1,gameRegion.size.width/678);brandTitleTag.bounds=CGRectMake(0,0,155,36);brandTitleTag.transform=CGAffineTransformMakeScale(scale,scale);brandTitleTag.center=CGPointMake(gameRegion.origin.x+MAX(20,gameRegion.size.width*.065)+155*scale/2,CGRectGetMaxY(gameRegion)-MAX(24,gameRegion.size.height*.11));brandTitleTag.hidden=NO;
 }else brandTitleTag.hidden=YES;
 brandingReport=@{@"asset":@"FakeLuStudio.jpg",@"introMatched":@(brandingIntroMatches),@"titleMatched":@(brandingTitleMatches),@"introPlayed":@(brandingIntroPlayed),@"method":@"Exact logo object names from supplied resource bundle; visual-only transform scale suppression; native finite animation"};
}
