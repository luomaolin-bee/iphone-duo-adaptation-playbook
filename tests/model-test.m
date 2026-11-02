#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <stdatomic.h>
#include <math.h>
#include <assert.h>
typedef struct{CGFloat top,left,bottom,right;}UIEdgeInsets;
static UIEdgeInsets UIEdgeInsetsMake(CGFloat t,CGFloat l,CGFloat b,CGFloat r){return(UIEdgeInsets){t,l,b,r};}
static CGRect UIEdgeInsetsInsetRect(CGRect a,UIEdgeInsets i){return CGRectMake(a.origin.x+i.left,a.origin.y+i.top,a.size.width-i.left-i.right,a.size.height-i.top-i.bottom);}
@interface UIView:NSObject
@property CGRect bounds;
@property UIEdgeInsets safeAreaInsets;
- (CGRect)convertRect:(CGRect)r fromView:(UIView *)v;
- (void)setNeedsDisplay;
@end
@implementation UIView
- (CGRect)convertRect:(CGRect)r fromView:(UIView *)v{return r;}
- (void)setNeedsDisplay{}
@end
@interface FakeDefaults:NSObject
@property NSMutableDictionary *values;
+ (instancetype)standardUserDefaults;
- (NSDictionary *)dictionaryForKey:(NSString *)key;
- (void)setObject:(id)object forKey:(NSString *)key;
@end
@implementation FakeDefaults
+ (instancetype)standardUserDefaults{static FakeDefaults *d;if(!d){d=[FakeDefaults new];d.values=[NSMutableDictionary dictionary];}return d;}
- (NSDictionary *)dictionaryForKey:(NSString *)key{return self.values[key];}
- (void)setObject:(id)object forKey:(NSString *)key{self.values[key]=object;}
@end
#define NSUserDefaults FakeDefaults
static UIView *gameWindow;
static BOOL splitActive,physicalFold;
static CGRect controlsRegion,gameRegion;
static void UpdateMoreButton(UIView *pad){}
#include "LayoutModel.inc"
static void UpdateFPSWidget(UIView *pad){}
static void OpenUnifiedEditor(UIView *pad){}
static void BringEditorFront(UIView *pad){}
static void UpdateEditorGeometry(void){}
static void PollGameContext(void){}
static void SceneFeedbackPulse(void){}
static void EdgeFeedback(NSInteger kind){}
static void PollBranding(NSString *scene,BOOL title,BOOL playing){}
int main(void){@autoreleasepool{
 const double sizes[][2]={{466,678},{678,466},{669,951},{951,669},{669,951}};NSMutableSet *profiles=[NSMutableSet set];UIView *pad=[UIView new];gameWindow=[UIView new];unsigned checked=0;NSMutableArray *snapshots=[NSMutableArray array];
 for(int mode=0;mode<5;mode++){
  gameWindow.bounds=CGRectMake(0,0,sizes[mode][0],sizes[mode][1]);gameWindow.safeAreaInsets=UIEdgeInsetsMake(0,0,34,mode<2?84:0);splitActive=mode==0||mode==2||mode==4;physicalFold=mode==4;CGFloat y=mode==0?423.75:(mode==2?613.395:495.5);controlsRegion=CGRectMake(0,y,gameWindow.bounds.size.width,gameWindow.bounds.size.height-y);gameRegion=splitActive?CGRectMake(0,mode==0?0:82,gameWindow.bounds.size.width,(mode==4?455.5:y)-(mode==0?0:82)):gameWindow.bounds;pad.bounds=gameWindow.bounds;[profiles addObject:LayoutProfile()];
  NSMutableArray *buttons=[NSMutableArray array];for(NSNumber *n in EditableControls()){CGRect r=UnifiedRect(pad,n.integerValue);[buttons addObject:@{@"id":n,@"name":ControlName(n.integerValue),@"rect":@[@(r.origin.x),@(r.origin.y),@(r.size.width),@(r.size.height)]}];}
  [snapshots addObject:@{@"profile":LayoutProfile(),@"width":@(gameWindow.bounds.size.width),@"height":@(gameWindow.bounds.size.height),@"divider":@(splitActive?controlsRegion.origin.y:0),@"buttons":buttons}];
  for(NSUInteger a=0;a<buttons.count;a++)for(NSUInteger b=a+1;b<buttons.count;b++){
   NSDictionary *first=buttons[a],*second=buttons[b];
   NSArray *f=first[@"rect"],*s=second[@"rect"];CGRect r=CGRectMake([f[0]doubleValue],[f[1]doubleValue],[f[2]doubleValue],[f[3]doubleValue]),q=CGRectMake([s[0]doubleValue],[s[1]doubleValue],[s[2]doubleValue],[s[3]doubleValue]);if(CGRectIntersectsRect(r,q)){fprintf(stderr,"Overlap %s: %s / %s\n",LayoutProfile().UTF8String,[first[@"name"]UTF8String],[second[@"name"]UTF8String]);abort();}
  }
  CGRect canvas=LayoutCanvas(pad);assert(canvas.size.width==sizes[mode][0]-16);
  for(NSNumber *n in EditableControls()){
   NSInteger i=n.integerValue;CGRect allowed=ControlCanvas(pad,i);CGRect r=UnifiedRect(pad,i);assert(CGRectContainsRect(allowed,r));
   MoveUnifiedControl(pad,i,CGPointMake(CGRectGetMaxX(allowed),CGRectGetMidY(allowed)));r=UnifiedRect(pad,i);assert(fabs(CGRectGetMaxX(r)-CGRectGetMaxX(allowed))<.001);checked++;
  }
  if(splitActive){MoveUnifiedControl(pad,14,CGPointMake(gameWindow.bounds.size.width-120,gameRegion.origin.y+60));assert(CGRectGetMaxY(UnifiedRect(pad,14))<controlsRegion.origin.y);}
  AlignSecondary(pad,NO);CGRect previous=UnifiedRect(pad,7);for(NSNumber *n in @[@8,@9,@11,@12,@13]){CGRect r=UnifiedRect(pad,n.integerValue);assert(fabs(CGRectGetMidY(r)-CGRectGetMidY(previous))<.01);}
  AlignSecondary(pad,YES);previous=UnifiedRect(pad,7);for(NSNumber *n in @[@8,@9,@11,@12,@13]){CGRect r=UnifiedRect(pad,n.integerValue);assert(fabs(CGRectGetMidX(r)-CGRectGetMidX(previous))<.01);}
  SaveLayouts();menuContext=YES;automaticMenus=YES;customEditing=NO;
  for(NSNumber *n in EditableControls()){NSInteger i=n.integerValue;if(physicalFold)assert(!MenuHides(i));else assert(MenuHides(i)==(i>=1&&i<=9&&i!=2));}
  customEditing=YES;for(NSNumber *n in EditableControls())assert(!MenuHides(n.integerValue));customEditing=NO;menuContext=NO;
 }
 [[NSJSONSerialization dataWithJSONObject:snapshots options:NSJSONWritingPrettyPrinted error:nil]writeToFile:@"build/default-layouts-test.json" atomically:YES];
 assert(profiles.count==5);assert(EditableControls().count==14);assert(![EditableControls()containsObject:@10]);
 NSDictionary *before=[LayoutEntries(pad)copy];userLayoutProfiles=nil;assert([LayoutEntries(pad)isEqual:before]);
 printf("Passed %u full-width edge-drag checks; 5 independent profiles, 14 editable controls, horizontal/vertical alignment, menu/half-fold/editor policies, persistence; monitor can move above keyboard.\n",checked);
}}
