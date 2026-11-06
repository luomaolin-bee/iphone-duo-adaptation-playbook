#pragma once
#include <stdbool.h>
// 0.2.21 手柄 Select 策略（纯 C，可在任意主机上单测）。
// 丝之歌在 PlayStation 身份下把“背包”放在触摸板，Select/Share 一侧无默认动作；
// 盖世小鸡塔可等手柄在 iOS 上按 DS4 身份连接时，物理 Select 只会报告为 Share(buttonOptions)，游戏收不到背包。
// Xbox/MFi 身份下 View(buttonOptions) 本来就是背包，自动模式不干预，避免一次按键开关两次。
enum {DuoSelectModeAuto=0,DuoSelectModeInventory=1,DuoSelectModeMap=2,DuoSelectModeNative=3,DuoSelectModeCount=4};
enum {DuoPadGeneric=0,DuoPadXbox=1,DuoPadDualShock=2,DuoPadDualSense=3,DuoPadOther=4};
enum {DuoSelectNone=-1,DuoSelectInventoryControl=7,DuoSelectMapControl=8};
#define DuoSelectMinInterval 0.18

typedef struct {bool down;bool armed;double lastFire;} DuoSelectState;

static inline int DuoSelectNormalizeMode(long mode){return mode>=0&&mode<DuoSelectModeCount?(int)mode:DuoSelectModeAuto;}
static inline bool DuoPadIsPlayStation(int profile){return profile==DuoPadDualShock||profile==DuoPadDualSense;}
static inline int DuoSelectTarget(int mode,int profile){
 switch(DuoSelectNormalizeMode(mode)){
  case DuoSelectModeInventory:return DuoSelectInventoryControl;
  case DuoSelectModeMap:return DuoSelectMapControl;
  case DuoSelectModeNative:return DuoSelectNone;
  default:return DuoPadIsPlayStation(profile)?DuoSelectInventoryControl:DuoSelectNone;
 }
}
// 接入、回前台、重连后调用：必须先看到一次松开才允许触发，防止按住状态被当成新按下。
static inline void DuoSelectReset(DuoSelectState *s){s->down=false;s->armed=false;s->lastFire=-1e9;}
// 每次采样调用；返回需要短脉冲的触控键编号，或 DuoSelectNone。
static inline int DuoSelectStep(DuoSelectState *s,bool pressed,double now,int target){
 if(!pressed){s->down=false;s->armed=true;return DuoSelectNone;}
 if(s->down||!s->armed){s->down=true;return DuoSelectNone;}
 s->down=true;
 if(target<0||now-s->lastFire<DuoSelectMinInterval)return DuoSelectNone;
 s->lastFire=now;return target;
}
static inline int DuoSelectNextMode(int mode){return (DuoSelectNormalizeMode(mode)+1)%DuoSelectModeCount;}
static inline const char *DuoSelectModeName(int mode){
 static const char *names[]={"自动","背包","地图","原生（不转发）"};
 return names[DuoSelectNormalizeMode(mode)];
}
