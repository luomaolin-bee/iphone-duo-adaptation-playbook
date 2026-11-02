#include <assert.h>
#include <stdio.h>
extern int MenuKeysTest(int,int,int,int,int);
int main(void){
 for(int mask=0;mask<32;mask++){
  int submit=mask&1,cancel=(mask>>1)&1,jump=(mask>>2)&1,attack=(mask>>3)&1,cast=(mask>>4)&1;
  int expected=submit||attack?1:cancel?2:0;
  assert(MenuKeysTest(submit,cancel,jump,attack,cast)==expected);
 }
 puts("Passed 32 actual patched ARM64 menu-input combinations: Attack confirms, Jump/Cast neutral, Escape returns.");
}
