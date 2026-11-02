#import <mach/mach.h>
#import <mach/mach_time.h>
static atomic_bool metricsEnabled;
static atomic_uint_fast64_t presentedFrames,gpuMicroseconds,gpuSamples;
static double residentMB,footprintMB,cpuPercent=-1,gpuMS=-1,frameMS;
static BOOL measuredPresented;
static NSString *ThermalName(void){switch(NSProcessInfo.processInfo.thermalState){case NSProcessInfoThermalStateNominal:return @"正常";case NSProcessInfoThermalStateFair:return @"温热";case NSProcessInfoThermalStateSerious:return @"较热";case NSProcessInfoThermalStateCritical:return @"过热";}return @"未知";}
static void SampleMetalTiming(id<MTLCommandBuffer> command,id<CAMetalDrawable> drawable){
 if(!atomic_load(&metricsEnabled))return;
 [drawable addPresentedHandler:^(id<MTLDrawable> d){if(d.presentedTime>0)atomic_fetch_add(&presentedFrames,1);}];
 static atomic_uint sequence;unsigned number=atomic_fetch_add(&sequence,1);
 if(number%4)return;
 [command addCompletedHandler:^(id<MTLCommandBuffer> c){double start=c.GPUStartTime,end=c.GPUEndTime;if(c.status==MTLCommandBufferStatusCompleted&&start>0&&end>=start&&end-start<1){atomic_fetch_add(&gpuMicroseconds,(uint64_t)llround((end-start)*1e6));atomic_fetch_add(&gpuSamples,1);}}];
}
static void SamplePerformance(double elapsed,BOOL active){
 uint64_t submitted=atomic_exchange(&gameSubmittedFrames,0),presented=atomic_exchange(&presentedFrames,0),samples=atomic_exchange(&gpuSamples,0),us=atomic_exchange(&gpuMicroseconds,0);
 if(active&&elapsed>0){measuredPresented=presented>0;measuredGameFPS=(presented?:submitted)/elapsed;frameMS=measuredGameFPS>0?1000/measuredGameFPS:0;}
 if(samples)gpuMS=(double)us/samples/1000;
 BOOL enabled=showFPS||customEditing;atomic_store(&metricsEnabled,enabled&&active);
 static uint64_t previousCPU;static BOOL previousValid;
 if(!enabled||!active){previousValid=NO;return;}
 task_vm_info_data_t memory;mach_msg_type_number_t n=TASK_VM_INFO_COUNT;
 if(task_info(mach_task_self(),TASK_VM_INFO,(task_info_t)&memory,&n)==KERN_SUCCESS){residentMB=memory.resident_size/1048576.0;footprintMB=memory.phys_footprint/1048576.0;}
 task_absolutetime_info_data_t cpu;n=TASK_ABSOLUTETIME_INFO_COUNT;
 if(task_info(mach_task_self(),TASK_ABSOLUTETIME_INFO,(task_info_t)&cpu,&n)==KERN_SUCCESS){uint64_t total=cpu.total_user+cpu.total_system;static mach_timebase_info_data_t timebase;if(!timebase.denom)mach_timebase_info(&timebase);if(previousValid&&total>=previousCPU&&elapsed>0){double seconds=(total-previousCPU)*(double)timebase.numer/timebase.denom/1e9;cpuPercent=MAX(0,MIN(NSProcessInfo.processInfo.activeProcessorCount*100.0,seconds/elapsed*100));}previousCPU=total;previousValid=YES;}
}
static NSString *MonitorText(void){
 CGSize render=gameMetalLayer.drawableSize;NSString *cpu=cpuPercent>=0?[NSString stringWithFormat:@"%.0f%%",cpuPercent]:@"--",*gpu=gpuMS>=0?[NSString stringWithFormat:@"%.1fms",gpuMS]:@"--";
 return [NSString stringWithFormat:@"%@ %.0f · Frame %.1fms\nCPU %@ · GPU %@\nRender %.0f×%.0f\nMEM %.0fMB · 热状态 %@\n倒影 %.0fFPS · %.0fms",measuredPresented?@"FPS":@"提交FPS",measuredGameFPS,frameMS,cpu,gpu,render.width,render.height,footprintMB,ThermalName(),reflectionQuality?reflectionDeliveryFPS:0,reflectionQuality?reflectionLatencyMS:0];
}
