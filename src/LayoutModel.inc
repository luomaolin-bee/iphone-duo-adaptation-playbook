static BOOL customEditing;
static BOOL menuContext;
static BOOL automaticMenus=YES,showFPS,feedbackEnabled=YES;
static NSInteger reflectionQuality=1;
static atomic_uint_fast64_t gameSubmittedFrames;
static CFTimeInterval lastSceneReveal;
static void SceneFeedbackPulse(void);
static void EdgeFeedback(NSInteger kind);
static void PollBranding(NSString *scene,BOOL title,BOOL playing);
static NSDictionary *brandingReport;
static double measuredGameFPS;
static NSDictionary *gameContextReport;
static __weak UIView *activePad;
static void OpenUnifiedEditor(UIView *pad);
static void BringEditorFront(UIView *pad);
static void UpdateEditorGeometry(void);
static void PollGameContext(void);
static void UpdateFPSWidget(UIView *pad);
static void (*originalSettings)(id,SEL);
static CGRect (*originalControlRect)(id,SEL,NSInteger);
static NSMutableDictionary *userLayoutProfiles;
static NSString *LayoutProfile(void){
 if(splitActive)return physicalFold?@"半折叠018":(gameWindow.bounds.size.width<550?@"外屏竖屏018":@"内屏竖屏018");
 return MIN(gameWindow.bounds.size.width,gameWindow.bounds.size.height)<550?@"外屏横屏018":@"内屏横屏018";
}
static NSArray *EditableControls(void){return @[@0,@1,@2,@3,@4,@5,@6,@7,@8,@9,@11,@12,@13,@14];}
static NSString *ControlName(NSInteger i){return @{@0:@"摇杆",@1:@"跳跃",@2:@"攻击 / 确认",@3:@"冲刺",@4:@"缠丝",@5:@"技能",@6:@"工具",@7:@"背包",@8:@"地图",@9:@"暂停",@11:@"返回",@12:@"布局编辑图标",@13:@"更多",@14:@"帧率监测"}[@(i)]?:@"按键";}
static CGRect LayoutCanvas(UIView *pad){
 CGRect r=splitActive?controlsRegion:gameWindow.bounds;
 // The camera exclusion is a small local region, not a full-height no-drag strip.
 r=UIEdgeInsetsInsetRect(r,UIEdgeInsetsMake(8,8,MAX(8,gameWindow.safeAreaInsets.bottom),8));
 return [pad convertRect:r fromView:gameWindow];
}
static CGRect ControlCanvas(UIView *pad,NSInteger control){
 if(control!=14)return LayoutCanvas(pad);
 CGRect r=UIEdgeInsetsInsetRect(gameWindow.bounds,UIEdgeInsetsMake(MAX(8,gameWindow.safeAreaInsets.top),8,MAX(8,gameWindow.safeAreaInsets.bottom),8));return [pad convertRect:r fromView:gameWindow];
}
static void LoadLayouts(void){
 if(userLayoutProfiles)return;userLayoutProfiles=[NSMutableDictionary dictionary];
 NSDictionary *saved=[NSUserDefaults.standardUserDefaults dictionaryForKey:@"DuoUnifiedLayouts018"];
 [saved enumerateKeysAndObjectsUsingBlock:^(NSString *key,id obj,BOOL *stop){if(![obj isKindOfClass:NSDictionary.class])return;NSMutableDictionary *profile=[NSMutableDictionary dictionary];[obj enumerateKeysAndObjectsUsingBlock:^(NSString *n,id value,BOOL *s){if([value isKindOfClass:NSDictionary.class])profile[n]=[value mutableCopy];}];userLayoutProfiles[key]=profile;}];
}
static void SaveLayouts(void){LoadLayouts();[NSUserDefaults.standardUserDefaults setObject:userLayoutProfiles forKey:@"DuoUnifiedLayouts018"];}
static NSMutableDictionary *LayoutEntries(UIView *pad){
 LoadLayouts();NSString *profile=LayoutProfile();NSMutableDictionary *entries=userLayoutProfiles[profile];if(entries)return entries;
 BOOL split=splitActive,outer=[profile hasPrefix:@"外屏"],fold=physicalFold;
 // Approximate normalized centres/sizes traced from the five user screenshots.
 const double outP[7][3]={{.265,.65,.215},{.78,.55,.082},{.64,.58,.17},{.78,.76,.082},{.50,.78,.080},{.50,.52,.080},{.82,.39,.080}};
 const double inP[7][3]={{.24,.67,.215},{.94,.60,.064},{.77,.67,.20},{.94,.84,.064},{.60,.85,.064},{.60,.54,.064},{.85,.38,.064}};
 const double half[7][3]={{.26,.72,.245},{.94,.64,.086},{.78,.70,.155},{.94,.85,.086},{.63,.85,.086},{.65,.54,.086},{.80,.44,.086}};
 const double outL[7][3]={{.22,.75,.19},{.83,.63,.090},{.68,.75,.145},{.83,.85,.090},{.55,.84,.085},{.55,.57,.085},{.71,.50,.085}};
 const double inL[7][3]={{.19,.80,.15},{.96,.65,.064},{.83,.75,.155},{.96,.90,.064},{.70,.88,.064},{.70,.61,.064},{.84,.54,.064}};
 const double (*p)[3]=split?(fold?half:(outer?outP:inP)):(outer?outL:inL);
 CGRect area=LayoutCanvas(pad);CGFloat w=area.size.width,h=area.size.height;entries=[NSMutableDictionary dictionary];
 for(NSInteger i=0;i<=14;i++){
  double x=.5,y=.5,size=40;
  if(i<=6){x=p[i][0];y=p[i][1];size=MAX(32,MIN(w*p[i][2],h*.70));}
  if(i==11){x=split?.08:.08;y=split?.43:.57;size=40;}
  if(i==14){x=.86;y=split?.06:.20;size=40;}
  if(i==7||i==8||i==9||i==12||i==13){NSInteger col=i<=9?i-7:(i==12?3:4);x=split?.15+col*.175:(.56+col*.085);y=split?.23:.09;size=split?MIN(36,w*.07):MIN(38,w*.05);}
  CGRect raw=[pad convertRect:(splitActive?controlsRegion:gameWindow.bounds)fromView:gameWindow];
  x=(raw.origin.x+x*raw.size.width-area.origin.x)/MAX(1,w);y=(raw.origin.y+y*raw.size.height-area.origin.y)/MAX(1,h);
  entries[[@(i)stringValue]]=[@{@"x":@(x),@"y":@(y),@"size":@(size),@"alpha":@(i==2?.90:.85),@"hidden":@(i==10)}mutableCopy];
 }
 userLayoutProfiles[profile]=entries;return entries;
}
static NSMutableDictionary *LayoutEntry(UIView *pad,NSInteger control){
 NSMutableDictionary *e=LayoutEntries(pad)[[@(control)stringValue]];
 if(control==14&&![e[@"space"]isEqual:@"screen"]){
  // Migrate only the monitoring widget; preserve all existing combat/control coordinates.
  CGRect a=ControlCanvas(pad,14),r=[pad convertRect:gameRegion fromView:gameWindow];CGFloat width=MIN(212,a.size.width),height=76;
  CGFloat right=CGRectGetMaxX(r)-MAX(12,gameWindow.safeAreaInsets.right),top=CGRectGetMinY(r)+12+(splitActive?0:70);
  e[@"x"]=@((right-width/2-a.origin.x)/MAX(1,a.size.width));e[@"y"]=@((top+height/2-a.origin.y)/MAX(1,a.size.height));e[@"space"]=@"screen";
 }return e;
}
static CGRect UnifiedRect(UIView *pad,NSInteger control){
 NSDictionary *entry=LayoutEntry(pad,control);CGRect a=ControlCanvas(pad,control);CGFloat size=MAX(28,MIN(MIN(a.size.width,a.size.height),[entry[@"size"]doubleValue]));
 CGFloat width=control==14?MAX(180,size*5.3):size,height=control==14?MAX(70,size*1.9):size;width=MIN(width,a.size.width);height=MIN(height,a.size.height);
 CGFloat x=CGRectGetMinX(a)+[entry[@"x"]doubleValue]*a.size.width-width/2,y=CGRectGetMinY(a)+[entry[@"y"]doubleValue]*a.size.height-height/2;
 x=MAX(CGRectGetMinX(a),MIN(CGRectGetMaxX(a)-width,x));y=MAX(CGRectGetMinY(a),MIN(CGRectGetMaxY(a)-height,y));return CGRectMake(x,y,width,height);
}
static void MoveUnifiedControl(UIView *pad,NSInteger control,CGPoint point){
 CGRect a=ControlCanvas(pad,control);NSMutableDictionary *e=LayoutEntry(pad,control);e[@"x"]=@(MAX(0,MIN(1,(point.x-a.origin.x)/MAX(1,a.size.width))));e[@"y"]=@(MAX(0,MIN(1,(point.y-a.origin.y)/MAX(1,a.size.height))));
 [pad setNeedsDisplay];UpdateMoreButton(pad);UpdateFPSWidget(pad);
}
static BOOL MenuHides(NSInteger control){return automaticMenus&&menuContext&&!physicalFold&&!customEditing&&control!=0&&control!=2&&control!=11&&control!=12&&control!=13&&control!=14;}
static void AlignSecondary(UIView *pad,BOOL vertical){
 CGRect a=LayoutCanvas(pad);NSArray *ids=@[@7,@8,@9,@11,@12,@13];CGFloat extent=vertical?a.size.height:a.size.width;CGFloat side=MAX(28,MIN(38,extent/8));CGFloat spacing=side+MAX(0,MIN(10,(extent-ids.count*side)/(ids.count-1)));
 CGPoint centre=CGPointMake(CGRectGetMidX(a),CGRectGetMidY(a));CGPoint start=vertical?CGPointMake(CGRectGetMaxX(a)-side/2,centre.y-(ids.count-1)*spacing/2):CGPointMake(centre.x-(ids.count-1)*spacing/2,CGRectGetMinY(a)+side/2+10);
 for(NSInteger i=0;i<(NSInteger)ids.count;i++){NSInteger n=[ids[i]integerValue];LayoutEntry(pad,n)[@"size"]=@(side);MoveUnifiedControl(pad,n,CGPointMake(start.x+(vertical?0:i*spacing),start.y+(vertical?i*spacing:0)));}SaveLayouts();
}
