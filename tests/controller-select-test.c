#include "ControllerSelectPolicy.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned tested;
#define CHECK(x) do{assert(x);tested++;}while(0)
int main(void){
 // 目标映射：自动只对 PlayStation 身份转发背包。
 CHECK(DuoSelectTarget(DuoSelectModeAuto,DuoPadDualShock)==DuoSelectInventoryControl);
 CHECK(DuoSelectTarget(DuoSelectModeAuto,DuoPadDualSense)==DuoSelectInventoryControl);
 CHECK(DuoSelectTarget(DuoSelectModeAuto,DuoPadXbox)==DuoSelectNone);
 CHECK(DuoSelectTarget(DuoSelectModeAuto,DuoPadGeneric)==DuoSelectNone);
 CHECK(DuoSelectTarget(DuoSelectModeAuto,DuoPadOther)==DuoSelectNone);
 for(int p=0;p<=4;p++){
  CHECK(DuoSelectTarget(DuoSelectModeInventory,p)==DuoSelectInventoryControl);
  CHECK(DuoSelectTarget(DuoSelectModeMap,p)==DuoSelectMapControl);
  CHECK(DuoSelectTarget(DuoSelectModeNative,p)==DuoSelectNone);
 }
 // 非法偏好值回退自动。
 CHECK(DuoSelectNormalizeMode(-3)==DuoSelectModeAuto);CHECK(DuoSelectNormalizeMode(99)==DuoSelectModeAuto);
 CHECK(DuoSelectTarget(42,DuoPadDualShock)==DuoSelectInventoryControl);
 // 模式循环。
 int m=DuoSelectModeAuto;for(int i=0;i<DuoSelectModeCount;i++)m=DuoSelectNextMode(m);CHECK(m==DuoSelectModeAuto);
 CHECK(strcmp(DuoSelectModeName(DuoSelectModeNative),"原生（不转发）")==0);

 DuoSelectState s;DuoSelectReset(&s);
 // 接入时已按住：不触发；松开后再按才触发。
 CHECK(DuoSelectStep(&s,1,0.00,7)==DuoSelectNone);
 CHECK(DuoSelectStep(&s,1,0.02,7)==DuoSelectNone);
 CHECK(DuoSelectStep(&s,0,0.04,7)==DuoSelectNone);
 CHECK(DuoSelectStep(&s,1,0.06,7)==7);
 // 按住不重复。
 for(int i=0;i<30;i++)CHECK(DuoSelectStep(&s,1,0.07+i*0.01,7)==DuoSelectNone);
 CHECK(DuoSelectStep(&s,0,0.40,7)==DuoSelectNone);
 // 正常第二次按下。
 CHECK(DuoSelectStep(&s,1,0.50,7)==7);
 // 抖动：过快的再次按下被吞。
 CHECK(DuoSelectStep(&s,0,0.52,7)==DuoSelectNone);
 CHECK(DuoSelectStep(&s,1,0.55,7)==DuoSelectNone);
 CHECK(DuoSelectStep(&s,0,0.70,7)==DuoSelectNone);
 CHECK(DuoSelectStep(&s,1,0.80,7)==7);
 // 目标为空（原生/ Xbox 自动）不触发，也不消耗冷却。
 CHECK(DuoSelectStep(&s,0,1.00,-1)==DuoSelectNone);
 CHECK(DuoSelectStep(&s,1,1.01,-1)==DuoSelectNone);
 CHECK(DuoSelectStep(&s,0,1.02,8)==DuoSelectNone);
 CHECK(DuoSelectStep(&s,1,1.03,8)==8);
 // 回前台/重连复位：按住期间复位不触发。
 DuoSelectReset(&s);CHECK(DuoSelectStep(&s,1,2.0,7)==DuoSelectNone);
 CHECK(DuoSelectStep(&s,0,2.1,7)==DuoSelectNone);CHECK(DuoSelectStep(&s,1,2.2,7)==7);
 // 两个手柄各自独立。
 DuoSelectState a,b;DuoSelectReset(&a);DuoSelectReset(&b);
 CHECK(DuoSelectStep(&a,0,3.0,7)==DuoSelectNone);CHECK(DuoSelectStep(&b,0,3.0,7)==DuoSelectNone);
 CHECK(DuoSelectStep(&a,1,3.1,7)==7);CHECK(DuoSelectStep(&b,1,3.1,7)==7);
 printf("Passed %u controller Select policy assertions.\n",tested);
 return 0;
}
