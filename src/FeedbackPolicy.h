#pragma once
#include <stdbool.h>
static inline int DuoHealthFeedback(bool playing,bool menu,bool changed,double stable,double sampleGap,int previous,int current,double cooldown){
 if(!playing||menu||changed||stable<1||sampleGap>.8||sampleGap<0||previous<=0||previous>200||current<0||current>200||cooldown<.65)return 0;
 return current<previous?1:(current>previous?2:0);
}
static inline bool DuoShouldReveal(bool initialized,bool changed,bool loading,double cooldown){return initialized&&changed&&!loading&&cooldown>.65;}
