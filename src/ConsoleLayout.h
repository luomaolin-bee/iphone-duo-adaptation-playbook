#pragma once
#include <CoreGraphics/CoreGraphics.h>
#include <math.h>
static inline CGFloat DuoActionSize(CGFloat w,CGFloat h,int split){return fmin(62,fmin(w*(split?.145:.115),h*.20));}
static inline CGRect DuoActionRect(CGFloat w,CGFloat h,int split,int control){CGFloat s=DuoActionSize(w,h,split),gap=fmax(7,s*.15);int column=(control==4||control==5)?0:((control==1||control==2)?1:2);int top=control==2||control==5||control==6;CGFloat y=h-12-s-(top?(s+gap):0);if(top&&column==1)y-=s*.16;if(!top&&column!=1)y-=s*.04;return CGRectMake(w-12-s-(2-column)*(s+gap),y,s,s);}
static inline float DuoHudNextX(float current,float base,float written,float shift,int hasSaved){return (hasSaved&&fabsf(current-written)<.02f?base:current)+shift;}
