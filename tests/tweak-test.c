#include <stdint.h>
#include <assert.h>
#include <stdio.h>
extern void DuoTweakTest(uint32_t orientation,uint32_t *state,int32_t *requested);
int main(void){
 uint32_t state[3]={0};unsigned cases=0;
 int32_t sizes[][2]={{2007,1120},{1398,1272},{2007,1594},{1280,720},{720,1280},{0,0},{-1,80},{2,3},{3,2}};
 for(unsigned split=0;split<2;split++)for(unsigned orient=0;orient<7;orient++)for(unsigned i=0;i<9;i++){
  int32_t requested[2]={sizes[i][0],sizes[i][1]},expected[2]={sizes[i][0],sizes[i][1]};state[2]=split;
  int portrait=orient==1||orient==2,landscape=orient==3||orient==4;
  if(!split&&expected[0]>0&&expected[1]>0&&((portrait&&expected[0]>expected[1])||(landscape&&expected[0]<expected[1]))){int t=expected[0];expected[0]=expected[1];expected[1]=t;}
  DuoTweakTest(orient,state,requested);assert(requested[0]==expected[0]&&requested[1]==expected[1]);cases++;
 }
 printf("Passed %u ARM64 split/native orientation cases.\n",cases);
}
