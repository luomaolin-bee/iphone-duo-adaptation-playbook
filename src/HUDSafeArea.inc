// Public IL2CPP field reads + UUID-bound, typed Unity accessors. No runtime_invoke or GC handles.
typedef struct{float x,y;}DuoVector2;
typedef struct{float x,y,z;}DuoVector3;
static uintptr_t hudEngineBase;
static NSDictionary *hudSafeReport;
static NSMutableDictionary *hudOffsets;
static struct{
 void *(*domain)(void);const void **(*assemblies)(void *,size_t *);void *(*image)(const void *);
 void *(*className)(void *,const char *,const char *);void *(*objectClass)(void *);const char *(*classGetName)(void *);
 void *(*field)(void *,const char *);uint32_t(*flags)(void *);const void *(*fieldType)(void *);int(*typeKind)(const void *);void(*readStatic)(void *,void *);
 int(*stringLength)(void *);const uint16_t *(*stringChars)(void *);
 void *(*transform)(void *,void *);int(*childCount)(void *,void *);void *(*child)(void *,int,void *);void *(*name)(void *,void *);int(*instanceID)(void *,void *);
 DuoVector2(*anchor)(void *,void *);void(*setAnchor)(void *,DuoVector2,void *);DuoVector3(*position)(void *,void *);void(*setPosition)(void *,DuoVector3,void *);void *(*parent)(void *,void *);DuoVector3(*transformPoint)(void *,DuoVector3,void *);DuoVector3(*screenPoint)(void *,DuoVector3,void *);
}hudAPI;
static BOOL PrepareHudAPI(void){
 static BOOL ready;if(ready)return YES;if(!hudEngineBase)return NO;
#define HUD_LOAD(F,S) hudAPI.F=dlsym(RTLD_DEFAULT,S);if(!hudAPI.F)return NO;
 HUD_LOAD(domain,"il2cpp_domain_get") HUD_LOAD(assemblies,"il2cpp_domain_get_assemblies") HUD_LOAD(image,"il2cpp_assembly_get_image")
 HUD_LOAD(className,"il2cpp_class_from_name") HUD_LOAD(objectClass,"il2cpp_object_get_class") HUD_LOAD(classGetName,"il2cpp_class_get_name")
 HUD_LOAD(field,"il2cpp_class_get_field_from_name") HUD_LOAD(flags,"il2cpp_field_get_flags") HUD_LOAD(fieldType,"il2cpp_field_get_type") HUD_LOAD(typeKind,"il2cpp_type_get_type") HUD_LOAD(readStatic,"il2cpp_field_static_get_value")
 HUD_LOAD(stringLength,"il2cpp_string_length") HUD_LOAD(stringChars,"il2cpp_string_chars")
#undef HUD_LOAD
 hudAPI.transform=(void *)(hudEngineBase+0x917c508);hudAPI.childCount=(void *)(hudEngineBase+0x91c2748);hudAPI.child=(void *)(hudEngineBase+0x91c4300);hudAPI.name=(void *)(hudEngineBase+0x919382c);hudAPI.instanceID=(void *)(hudEngineBase+0x9192fb8);
 hudAPI.anchor=(void *)(hudEngineBase+0x91b8fac);hudAPI.setAnchor=(void *)(hudEngineBase+0x91b9138);hudAPI.position=(void *)(hudEngineBase+0x91b9978);hudAPI.setPosition=(void *)(hudEngineBase+0x91b9b30);hudAPI.parent=(void *)(hudEngineBase+0x91bb490);hudAPI.transformPoint=(void *)(hudEngineBase+0x91b721c);hudAPI.screenPoint=(void *)(hudEngineBase+0x8fd23d4);ready=YES;return YES;
}
static BOOL HudObjectAlive(void *object){return object&&*(uintptr_t *)((char *)object+0x10)!=0;}
static void *HudSingleton(const char *name){
 static NSMutableDictionary *cache;if(!cache)cache=[NSMutableDictionary dictionary];NSString *key=[NSString stringWithUTF8String:name];void *field=[cache[key]pointerValue];
 if(!field){size_t count=0;const void **assemblies=hudAPI.assemblies(hudAPI.domain(),&count);for(size_t i=0;i<count&&!field;i++){void *klass=hudAPI.className(hudAPI.image(assemblies[i]),"",name);if(!klass)continue;for(NSString *n in @[@"instance",@"_instance",@"<instance>k__BackingField"]){void *f=hudAPI.field(klass,n.UTF8String);if(f&&(hudAPI.flags(f)&0x10)&&hudAPI.typeKind(hudAPI.fieldType(f))==0x12){field=f;break;}}}if(field)cache[key]=[NSValue valueWithPointer:field];}
 void *object=NULL;if(field)hudAPI.readStatic(field,&object);return object&&(!strcmp(name,"PlayerData")||HudObjectAlive(object))?object:NULL;
}
static NSString *HudObjectName(void *transform){void *name=hudAPI.name(transform,NULL);if(!name)return @"";int n=hudAPI.stringLength(name);if(n<0||n>160)return @"";return [NSString stringWithCharacters:hudAPI.stringChars(name) length:n];}
static void HudMoveStats(void *transform,void *camera,float pixelShift,NSInteger depth,NSUInteger *budget,NSMutableArray *names,NSMutableArray *moved){
 if(!HudObjectAlive(transform)||!*budget||depth>4)return;(*budget)--;
 NSString *name=HudObjectName(transform);if(names.count<50)[names addObject:name];NSString *lower=name.lowercaseString;
 BOOL stats=([lower containsString:@"health"]||[lower containsString:@"silk"]||[lower containsString:@"soul"]||[lower containsString:@"bind orb"]||[lower containsString:@"bindorb"])&&![lower containsString:@"blank"];
 const char *className=hudAPI.classGetName(hudAPI.objectClass(transform));
 BOOL rect=className&&!strcmp(className,"RectTransform"),plain=className&&!strcmp(className,"Transform");
 if(stats&&(rect||plain)){
  void *parent=hudAPI.parent(transform,NULL);if(!HudObjectAlive(parent))return;
  DuoVector3 a=hudAPI.transformPoint(parent,(DuoVector3){0,0,0},NULL),b=hudAPI.transformPoint(parent,(DuoVector3){1,0,0},NULL);
  a=hudAPI.screenPoint(camera,a,NULL);b=hudAPI.screenPoint(camera,b,NULL);float pixelsPerUnit=b.x-a.x;
  if(!isfinite(pixelsPerUnit)||fabsf(pixelsPerUnit)<.0001f)return;
  float shift=pixelShift/pixelsPerUnit;if(!isfinite(shift)||fabsf(shift)>2000)return;
  NSNumber *key=@(hudAPI.instanceID(transform,NULL));DuoVector3 current;
  if(rect){DuoVector2 v=hudAPI.anchor(transform,NULL);current=(DuoVector3){v.x,v.y,0};}else current=hudAPI.position(transform,NULL);
  if(!isfinite(current.x)||!isfinite(current.y)||!isfinite(current.z))return;
  NSDictionary *saved=hudOffsets[key];float next=DuoHudNextX(current.x,[saved[@"base"]floatValue],[saved[@"written"]floatValue],shift,saved!=nil);
  if(fabsf(current.x-next)>.01f){if(rect)hudAPI.setAnchor(transform,(DuoVector2){next,current.y},NULL);else hudAPI.setPosition(transform,(DuoVector3){next,current.y,current.z},NULL);}
  hudOffsets[key]=@{@"base":@(next-shift),@"written":@(next)};[moved addObject:@{@"name":name,@"shiftLocalUnits":@(shift)}];return;
 }
 int count=hudAPI.childCount(transform,NULL);for(int i=0;i<MIN(128,count)&&*budget;i++)HudMoveStats(hudAPI.child(transform,i,NULL),camera,pixelShift,depth+1,budget,names,moved);
}
static void UpdateHudSafeArea(void){
 id app=UIApplication.sharedApplication.delegate;if(UIApplication.sharedApplication.applicationState!=UIApplicationStateActive||![app respondsToSelector:@selector(engineLoadState)]||[app engineLoadState]<3||!PrepareHudAPI())return;
 CGFloat inset=0;CGRect bounds=gameWindow.bounds;
 // Only the closed outer display in landscape; inner and split framing remain unchanged.
 BOOL outerLandscape=!splitActive&&bounds.size.width>bounds.size.height&&MIN(bounds.size.width,bounds.size.height)<550;
 if(outerLandscape){
  if(controlSafeInsets.left>0)inset=controlSafeInsets.left+12;
  if(@available(iOS 27.1,*))for(UIViewReservedRegion *r in [gameWindow reservedRegionsOfKind:UIViewReservedRegionKind.occlusionRegionKind])if(r.isActive&&r.frame.origin.x<bounds.size.width*.28&&r.frame.origin.y<bounds.size.height*.25)inset=MAX(inset,CGRectGetMaxX(r.frame)+12);
 }
 if(inset<=0&&!hudOffsets.count){hudSafeReport=@{@"status":@"no-left-camera-inset",@"shiftPoints":@0};return;}
 static CFTimeInterval lastCheck;static CGFloat lastInset=-1;CFTimeInterval now=CACurrentMediaTime();if(inset==lastInset&&now-lastCheck<3)return;lastInset=inset;lastCheck=now;
 void *hud=HudSingleton("HudCanvas"),*cameras=HudSingleton("GameCameras");if(!hud||!cameras){hudSafeReport=@{@"status":@"waiting-for-hud-singletons",@"shiftPoints":@(inset)};return;}
 // GameCameras.MoveMenuToHUDCamera passes this exact field to Canvas.worldCamera.
 void *camera=*(void **)((char *)cameras+0x20);
 if(!HudObjectAlive(camera)||strcmp(hudAPI.classGetName(hudAPI.objectClass(camera)),"Camera")){hudSafeReport=@{@"status":@"hud-camera-unavailable",@"shiftPoints":@(inset)};return;}
 int rw=0,rh=0;getRenderingResolution(&rw,&rh);float pixelShift=(float)(inset*rw/MAX(1,gameView.bounds.size.width));
 if(!hudOffsets)hudOffsets=[NSMutableDictionary dictionary];
 NSMutableArray *names=[NSMutableArray array],*moved=[NSMutableArray array];NSUInteger budget=160;
 HudMoveStats(hudAPI.transform(hud,NULL),camera,pixelShift,0,&budget,names,moved);
 hudSafeReport=@{@"status":moved.count?@"stats-groups-offset":@"stats-groups-not-found",@"shiftPoints":@(inset),@"shiftPixels":@(pixelShift),@"movedGroups":moved,@"hudHierarchy":names};
 if(inset<=0)[hudOffsets removeAllObjects];
}
