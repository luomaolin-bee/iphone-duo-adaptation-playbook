#include "FeedbackPolicy.h"
static void (*contextRead)(void *,void *,void *);
static void *(*contextClassFromType)(const void *);
static void *(*contextFields)(void *,void **);
static const char *(*contextFieldName)(void *);
static bool (*contextIsEnum)(void *);
static const void *(*contextEnumBase)(void *);
static void *ContextField(void *object,const char *name){
 if(!object)return NULL;static NSMutableDictionary *classes;if(!classes)classes=[NSMutableDictionary dictionary];NSValue *key=[NSValue valueWithPointer:hudAPI.objectClass(object)];NSMutableDictionary *fields=classes[key];if(!fields){fields=[NSMutableDictionary dictionary];classes[key]=fields;}NSString *n=[NSString stringWithUTF8String:name];id cached=fields[n];if(cached)return cached==NSNull.null?NULL:[cached pointerValue];void *field=hudAPI.field(key.pointerValue,name);fields[n]=field?[NSValue valueWithPointer:field]:NSNull.null;return field;
}
static NSNumber *ContextNumber(void *object,const char *name,NSString **enumName){
 void *field=ContextField(object,name);if(!field||hudAPI.flags(field)&0x10)return nil;const void *type=hudAPI.fieldType(field);int kind=hudAPI.typeKind(type);void *klass=NULL;
 if(kind==0x11){klass=contextClassFromType(type);if(!klass||!contextIsEnum(klass))return nil;kind=hudAPI.typeKind(contextEnumBase(klass));}
 if(kind!=2&&kind!=8&&kind!=9)return nil;int value=0;contextRead(object,field,&value);
 if(klass&&enumName){
  static NSMutableDictionary *cache;if(!cache)cache=[NSMutableDictionary dictionary];NSValue *key=[NSValue valueWithPointer:field];NSDictionary *names=cache[key];
  if(!names){NSMutableDictionary *map=[NSMutableDictionary dictionary];void *it=NULL,*f;while((f=contextFields(klass,&it))){if(!(hudAPI.flags(f)&0x40))continue;int constant=0;hudAPI.readStatic(f,&constant);const char *n=contextFieldName(f);if(n)map[@(constant)]=[NSString stringWithUTF8String:n];}names=map;cache[key]=names;}
  *enumName=names[@(value)];
 }return @(value);
}
static void *ContextObject(void *object,const char *name){void *field=ContextField(object,name);if(!field||hudAPI.flags(field)&0x10)return NULL;int k=hudAPI.typeKind(hudAPI.fieldType(field));if(k!=0x12&&k!=0x15&&k!=0x1c)return NULL;void *value=NULL;contextRead(object,field,&value);return value;}
static NSString *ContextString(void *object,const char *name){
 void *field=ContextField(object,name);if(!field||hudAPI.flags(field)&0x10||hudAPI.typeKind(hudAPI.fieldType(field))!=0xe)return nil;void *value=NULL;contextRead(object,field,&value);if(!value)return nil;int length=hudAPI.stringLength(value);return length>=0&&length<=256?[NSString stringWithCharacters:hudAPI.stringChars(value)length:length]:nil;
}
static NSUInteger contextTransitionEvents;
static void PollGameContext(void){
 static CFTimeInterval last,stableSince,lastEdge;static NSString *previousScene,*previousSignature,*previousGame;static NSNumber *previousHealth,*previousGeo;static BOOL initialized,inputRevealSent;static NSNumber *previousAccepting;static CFTimeInterval inputSuspendedSince;
 CFTimeInterval now=CACurrentMediaTime();id app=UIApplication.sharedApplication.delegate;
 if(UIApplication.sharedApplication.applicationState!=UIApplicationStateActive){previousHealth=nil;previousGeo=nil;previousSignature=nil;previousGame=nil;initialized=NO;previousAccepting=nil;inputRevealSent=NO;stableSince=now;last=0;return;}
 if(![app respondsToSelector:@selector(engineLoadState)]||[app engineLoadState]<3||!PrepareHudAPI())return;if(now-last<.15)return;double gap=last?now-last:0;last=now;
 if(!contextRead){contextRead=dlsym(RTLD_DEFAULT,"il2cpp_field_get_value");contextClassFromType=dlsym(RTLD_DEFAULT,"il2cpp_class_from_type");contextFields=dlsym(RTLD_DEFAULT,"il2cpp_class_get_fields");contextFieldName=dlsym(RTLD_DEFAULT,"il2cpp_field_get_name");contextIsEnum=dlsym(RTLD_DEFAULT,"il2cpp_class_is_enum");contextEnumBase=dlsym(RTLD_DEFAULT,"il2cpp_class_enum_basetype");}
 if(!contextRead||!contextClassFromType||!contextFields||!contextFieldName||!contextIsEnum||!contextEnumBase)return;
 void *gm=HudSingleton("GameManager"),*uiObject=HudSingleton("UIManager"),*pd=HudSingleton("PlayerData");
 NSString *game=nil,*ui=nil,*panel=nil;ContextNumber(gm,"<GameState>k__BackingField",&game);ContextNumber(uiObject,"uiState",&ui);NSNumber *panelNumber=ContextNumber(uiObject,"menuState",&panel),*inventory=ContextNumber(pd,"isInventoryOpen",NULL),*transition=ContextNumber(gm,"<IsInSceneTransition>k__BackingField",NULL);
 NSNumber *accepting=ContextNumber(ContextObject(HudSingleton("HeroController"),"inputHandler"),"acceptingInput",NULL),*paneIndex=ContextNumber(pd,"currentInvPane",NULL);
 NSString *scene=ContextString(gm,"sceneName");BOOL sceneChanged=previousScene&&scene&&![previousScene isEqual:scene];BOOL playing=[game isEqual:@"PLAYING"];
 BOOL next=inventory.boolValue||[@[@"MAIN_MENU",@"PAUSED"]containsObject:game?:@""]||[@[@"MAIN_MENU_HOME",@"OPTIONS",@"PAUSED"]containsObject:ui?:@""];
 if(sceneChanged||![previousGame isEqual:game]||transition.boolValue||gap>.8)stableSince=now;
 NSNumber *health=ContextNumber(pd,"health",NULL),*blue=ContextNumber(pd,"healthBlue",NULL),*geo=ContextNumber(pd,"geo",NULL);NSNumber *total=health?@(health.intValue+MAX(0,blue.intValue)):nil;
 int feedback=previousHealth&&total?DuoHealthFeedback(playing,next,sceneChanged||transition.boolValue,now-stableSince,gap,previousHealth.intValue,total.intValue,now-lastEdge):0;
 if(feedback){EdgeFeedback(feedback);lastEdge=now;}
 else if(initialized&&playing&&!transition.boolValue&&!sceneChanged&&previousGeo&&geo&&![previousGeo isEqual:geo]&&now-lastEdge>.9){EdgeFeedback(3);lastEdge=now;}
 BOOL inputTransition=NO;
 if(accepting&&!accepting.boolValue){if(!previousAccepting||previousAccepting.boolValue){inputSuspendedSince=now;inputRevealSent=NO;}if(!inputRevealSent&&now-inputSuspendedSince>.35&&playing&&now-lastEdge>1){inputTransition=YES;inputRevealSent=YES;}}
 else if(accepting.boolValue&&previousAccepting&&!previousAccepting.boolValue){inputTransition=inputRevealSent&&now-lastEdge>1;inputRevealSent=NO;}
 NSString *signature=[NSString stringWithFormat:@"%@|%@|%@|%@|%@|%@",scene?:@"?",game?:@"?",ui?:@"?",panel?:panelNumber?:@"?",inventory?:@"?",paneIndex?:@"?"];
 BOOL changed=(previousSignature&&![previousSignature isEqual:signature])||inputTransition;BOOL loading=transition.boolValue||[@[@"LOADING",@"ENTERING_LEVEL",@"EXITING_LEVEL",@"INACTIVE"]containsObject:game?:@""];
 if(DuoShouldReveal(initialized,changed,loading,now-lastSceneReveal)){contextTransitionEvents++;dispatch_async(dispatch_get_main_queue(),^{SceneFeedbackPulse();});}
 if(menuContext!=next){menuContext=next;for(UIWindow *window in gameWindow.windowScene.windows)RedrawPads(window);}
 PollBranding(scene,[game isEqual:@"MAIN_MENU"]||[ui isEqual:@"MAIN_MENU_HOME"],playing);
 gameContextReport=@{@"gameState":game?:@"unknown",@"uiState":ui?:@"unknown",@"menuState":panel?:panelNumber?:[NSNull null],@"inventoryOpen":inventory?:[NSNull null],@"menu":@(next),@"halfFoldException":@(physicalFold),@"scene":scene?:@"unknown",@"healthTotal":total?:[NSNull null],@"sceneTransition":transition?:[NSNull null],@"acceptingInput":accepting?:[NSNull null],@"inventoryPane":paneIndex?:[NSNull null],@"transitions":@(contextTransitionEvents)};
 previousHealth=total;previousGeo=geo;previousScene=scene;previousSignature=signature;previousGame=game;previousAccepting=accepting;initialized=game||ui||inventory;
}
