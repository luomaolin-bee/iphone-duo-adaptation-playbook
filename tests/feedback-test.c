#include "FeedbackPolicy.h"
#include <assert.h>
#include <stdio.h>
int main(void){
 unsigned tested=0;
 for(int previous=1;previous<=12;previous++)for(int current=0;current<=12;current++){
  int expected=current<previous?1:(current>previous?2:0);
  assert(DuoHealthFeedback(1,0,0,2,.2,previous,current,1)==expected);tested++;
  assert(!DuoHealthFeedback(0,0,0,2,.2,previous,current,1));tested++;
  assert(!DuoHealthFeedback(1,1,0,2,.2,previous,current,1));tested++;
  assert(!DuoHealthFeedback(1,0,1,2,.2,previous,current,1));tested++;
  assert(!DuoHealthFeedback(1,0,0,.5,.2,previous,current,1));tested++;
  assert(!DuoHealthFeedback(1,0,0,2,2,previous,current,1));tested++;
  assert(!DuoHealthFeedback(1,0,0,2,.2,previous,current,.2));tested++;
 }
 assert(!DuoHealthFeedback(1,0,0,2,.2,-1,4,1));assert(!DuoHealthFeedback(1,0,0,2,.2,4,-1,1));assert(!DuoHealthFeedback(1,0,0,2,.2,4,999,1));
 for(int initial=0;initial<2;initial++)for(int change=0;change<2;change++)for(int loading=0;loading<2;loading++){
  assert(DuoShouldReveal(initial,change,loading,1)==(initial&&change&&!loading));assert(!DuoShouldReveal(initial,change,loading,.1));tested+=2;
 }
 printf("Passed %u feedback policy cases plus invalid-health guards.\n",tested);
}
