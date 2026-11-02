#pragma once
#include <CoreGraphics/CoreGraphics.h>
#include <math.h>
static void DuoStroke(CGContextRef c,double r,double g,double b,double a,double width){CGContextSetRGBStrokeColor(c,r,g,b,a);CGContextSetLineWidth(c,width);}
static void DuoLine(CGContextRef c,double x,double y,double xx,double yy){CGContextMoveToPoint(c,x,y);CGContextAddLineToPoint(c,xx,yy);CGContextStrokePath(c);}
static void DuoDiamond(CGContextRef c,CGFloat x,CGFloat y,CGFloat r){CGContextMoveToPoint(c,x,y-r);CGContextAddLineToPoint(c,x+r*.55,y);CGContextAddLineToPoint(c,x,y+r);CGContextAddLineToPoint(c,x-r*.55,y);CGContextClosePath(c);CGContextStrokePath(c);}
static void DuoConsoleFrame(CGContextRef c,CGRect r){
 CGContextSaveGState(c);CGFloat x=r.origin.x,y=r.origin.y,w=r.size.width,h=r.size.height;
 DuoStroke(c,.54,.44,.28,.50,.7);
 CGPathRef path=CGPathCreateWithRoundedRect(CGRectInset(r,9,9),20,20,NULL);CGContextAddPath(c,path);CGContextStrokePath(c);CGPathRelease(path);
 DuoStroke(c,.93,.83,.61,.16,.5);path=CGPathCreateWithRoundedRect(CGRectInset(r,12,12),18,18,NULL);CGContextAddPath(c,path);CGContextStrokePath(c);CGPathRelease(path);
 // Fine brushed enamel, bounded and deterministic; no animated noise.
 DuoStroke(c,.67,.74,.72,.024,.45);for(int i=0;i<100;i++){CGFloat yy=y+14+(h-28)*i/100.0;DuoLine(c,x+17,yy,x+w-17,yy);}
 for(int side=0;side<2;side++)for(int end=0;end<2;end++){
  CGContextSaveGState(c);CGContextTranslateCTM(c,x+(side?w-15:15),y+(end?h-15:15));CGContextScaleCTM(c,side?-1:1,end?-1:1);
  DuoStroke(c,.72,.58,.36,.65,.8);
  CGContextMoveToPoint(c,0,45);CGContextAddCurveToPoint(c,1,29,18,31,18,17);CGContextAddCurveToPoint(c,18,5,2,4,5,15);CGContextAddCurveToPoint(c,11,23,23,12,27,6);CGContextStrokePath(c);
  CGContextMoveToPoint(c,3,30);CGContextAddCurveToPoint(c,10,19,32,28,43,0);CGContextStrokePath(c);
  DuoDiamond(c,0,47,3);DuoDiamond(c,45,0,3);CGContextRestoreGState(c);
 }
 DuoStroke(c,.82,.70,.47,.54,.7);DuoDiamond(c,x+w/2,y+11,5);DuoDiamond(c,x+w/2,y+h-11,5);
 DuoLine(c,x+55,y+11,x+w/2-12,y+11);DuoLine(c,x+w/2+12,y+11,x+w-55,y+11);
 CGContextRestoreGState(c);
}
static void DuoBezel(CGContextRef c,CGRect rect,BOOL pressed,BOOL joystick){
 CGContextSaveGState(c);CGFloat side=fmin(rect.size.width,rect.size.height),cx=CGRectGetMidX(rect),cy=CGRectGetMidY(rect);
 CGRect ring=CGRectInset(rect,1,1);
 CGContextSetShadowWithColor(c,CGSizeMake(0,2),4,CGColorGetConstantColor(kCGColorBlack));
 CGContextSetRGBFillColor(c,.035,.045,.05,.9);CGContextAddEllipseInRect(c,ring);CGContextAddEllipseInRect(c,CGRectInset(rect,6,6));CGContextEOFillPath(c);CGContextSetShadowWithColor(c,CGSizeZero,0,NULL);
 DuoStroke(c,.18,.15,.10,1,2.5);CGContextStrokeEllipseInRect(c,ring);
 DuoStroke(c,.82,.70,.49,pressed?.95:.62,.85);CGContextStrokeEllipseInRect(c,CGRectInset(rect,2.5,2.5));
 DuoStroke(c,.88,.82,.68,.22,.6);CGContextStrokeEllipseInRect(c,CGRectInset(rect,5,5));
 if(joystick){for(int i=0;i<4;i++){CGFloat angle=i*M_PI_2-M_PI_2;DuoDiamond(c,cx+cos(angle)*side*.48,cy+sin(angle)*side*.48,side*.032);}}
 else {DuoDiamond(c,cx,rect.origin.y+2.5,2.5);}
 CGContextRestoreGState(c);
}

static void DuoNeedleIcon(CGContextRef c,CGRect r){
 CGContextSaveGState(c);CGFloat cx=CGRectGetMidX(r),cy=CGRectGetMidY(r),s=r.size.width;
 CGContextSetRGBFillColor(c,.93,.87,.73,1);CGContextMoveToPoint(c,cx-s*.30,cy+s*.32);CGContextAddLineToPoint(c,cx+s*.47,cy-s*.48);CGContextAddLineToPoint(c,cx+s*.10,cy+s*.11);CGContextClosePath(c);CGContextFillPath(c);
 DuoStroke(c,.93,.87,.73,1,1.4);DuoLine(c,cx-s*.35,cy+s*.08,cx-s*.04,cy+s*.39);DuoLine(c,cx-s*.17,cy+s*.24,cx-s*.42,cy+s*.49);CGContextRestoreGState(c);
}
