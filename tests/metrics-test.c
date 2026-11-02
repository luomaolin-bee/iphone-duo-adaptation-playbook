#include <mach/mach.h>
#include <mach/mach_time.h>
#include <assert.h>
#include <stdio.h>
static uint64_t cpu(void){task_absolutetime_info_data_t data;mach_msg_type_number_t n=TASK_ABSOLUTETIME_INFO_COUNT;assert(task_info(mach_task_self(),TASK_ABSOLUTETIME_INFO,(task_info_t)&data,&n)==KERN_SUCCESS);return data.total_user+data.total_system;}
int main(void){mach_timebase_info_data_t tb;assert(mach_timebase_info(&tb)==KERN_SUCCESS);uint64_t a=cpu(),start=mach_absolute_time();volatile double v=1;double seconds=0;do{for(int i=0;i<10000;i++)v=v*1.00000001+.00001;seconds=(mach_absolute_time()-start)*(double)tb.numer/tb.denom/1e9;}while(seconds<.25);double use=(cpu()-a)*(double)tb.numer/tb.denom/1e9;assert(use>.05&&use<seconds*1.5);task_vm_info_data_t vm;mach_msg_type_number_t n=TASK_VM_INFO_COUNT;assert(task_info(mach_task_self(),TASK_VM_INFO,(task_info_t)&vm,&n)==KERN_SUCCESS);assert(vm.resident_size>0&&vm.phys_footprint>0);printf("Self-task CPU clock verified: %.3fs CPU / %.3fs wall; resident and physical-footprint queries succeeded.\n",use,seconds);}
