// GPU downsample/blur on the presenting command buffer. One small frame in flight.
#include "ReflectionGPU.inc"
static void (*scheduledOriginal)(id,SEL,MTLCommandBufferHandler);
static void (*commitOriginal)(id,SEL);
static char pendingDrawableKey;
static void (*framebufferOriginal)(id,SEL,BOOL);
static CFTimeInterval lastGPUCopy;
static void SetCaptureNote(NSString *note){dispatch_async(dispatch_get_main_queue(),^{reflectionStatus=note;});}
static void CaptureGameDrawable(id<MTLCommandBuffer> command,id drawable){
    if(!atomic_load(&wantsReflection)||![drawable conformsToProtocol:@protocol(CAMetalDrawable)])return;
    if(atomic_exchange(&reflectionInFlight,true))return;
    CFTimeInterval now=CACurrentMediaTime();unsigned fps=MAX(1,atomic_load(&reflectionTargetFPS));
    if(now-lastGPUCopy+.001<1.0/fps){atomic_store(&reflectionInFlight,false);return;}
    id<CAMetalDrawable> metalDrawable=drawable;
    if(metalDrawable.layer!=gameMetalLayer){atomic_store(&reflectionInFlight,false);return;}
    id<MTLTexture> source=metalDrawable.texture;
    BOOL supported=source.pixelFormat==MTLPixelFormatBGRA8Unorm||source.pixelFormat==MTLPixelFormatBGRA8Unorm_sRGB||source.pixelFormat==MTLPixelFormatRGBA8Unorm||source.pixelFormat==MTLPixelFormatRGBA8Unorm_sRGB;
    if(source.framebufferOnly||!supported||source.sampleCount!=1||!source.width||!source.height){SetCaptureNote(@"unsupported-source");atomic_store(&reflectionInFlight,false);return;}
    static DuoReflectionGPU *gpu;if(!gpu)gpu=[DuoReflectionGPU new];
    gpu.maxEdge=atomic_load(&reflectionMaxEdge);
    if(![gpu encodeSource:source command:command]){atomic_store(&reflectionInFlight,false);return;}
    NSUInteger width=gpu.width,height=gpu.height,row=gpu.row;id<MTLBuffer> buffer=gpu.readback;
    lastGPUCopy=now;
    [command addCompletedHandler:^(id<MTLCommandBuffer> completed){
        if(completed.status!=MTLCommandBufferStatusCompleted){SetCaptureNote(@"gpu-reflection-failed");atomic_store(&reflectionInFlight,false);return;}
        dispatch_async(reflectionQueue,^{@autoreleasepool{
            NSData *data=[NSData dataWithBytes:buffer.contents length:row*height];
            CGDataProviderRef provider=CGDataProviderCreateWithCFData((__bridge CFDataRef)data);
            CGColorSpaceRef color=CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
            CGImageRef image=CGImageCreate(width,height,8,32,row,color,kCGBitmapByteOrder32Big|kCGImageAlphaNoneSkipLast,provider,NULL,NO,kCGRenderingIntentDefault);
            UIImage *result=image?[UIImage imageWithCGImage:image]:nil;
            if(image)CGImageRelease(image);CGColorSpaceRelease(color);CGDataProviderRelease(provider);
            dispatch_async(dispatch_get_main_queue(),^{
                if(result&&atomic_load(&wantsReflection)){
                    static CFTimeInterval started;static NSUInteger delivered;
                    CFTimeInterval time=CACurrentMediaTime();if(!started)started=time;
                    delivered++;if(time-started>=1){reflectionDeliveryFPS=delivered/(time-started);started=time;delivered=0;}
                    reflectionLatencyMS=(time-now)*1000;reflectionReadbackBytes=row*height;
                    reflectedGameImage=result;reflectionFrames++;reflectionStatus=@"gpu-downsample-blur-live";
                    for(UIWindow *window in gameWindow.windowScene.windows)RedrawPadsForImage(window);
                }
                atomic_store(&reflectionInFlight,false);
            });
        }});
    }];
}
typedef struct{void *isa;int flags,reserved;void *invoke,*descriptor;__unsafe_unretained id drawable;}DuoPresentBlock;
static void ScheduledHook(id self,SEL cmd,MTLCommandBufferHandler handler){
    const DuoPresentBlock *block=(__bridge const void *)handler;
    if(block&&presentationBlockAddress&&(uintptr_t)block->invoke==presentationBlockAddress){const uintptr_t *descriptor=block->descriptor;if(descriptor&&descriptor[1]>=sizeof(DuoPresentBlock))objc_setAssociatedObject(self,&pendingDrawableKey,block->drawable,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
    scheduledOriginal(self,cmd,handler);
}
static void CommitHook(id self,SEL cmd){id drawable=objc_getAssociatedObject(self,&pendingDrawableKey);if(drawable){if([drawable conformsToProtocol:@protocol(CAMetalDrawable)]&&[(id<CAMetalDrawable>)drawable layer]==gameMetalLayer){atomic_fetch_add(&gameSubmittedFrames,1);SampleMetalTiming(self,drawable);}CaptureGameDrawable(self,drawable);objc_setAssociatedObject(self,&pendingDrawableKey,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}commitOriginal(self,cmd);}
static void FramebufferHook(id self,SEL cmd,BOOL value){framebufferOriginal(self,cmd,self==gameMetalLayer?NO:value);}
static IMP HookOwnMethod(Class cls,SEL selector,IMP replacement){Method method=class_getInstanceMethod(cls,selector);if(!method)return NULL;IMP original=method_getImplementation(method);if(!class_addMethod(cls,selector,replacement,method_getTypeEncoding(method)))method_setImplementation(method,replacement);return original;}
static void InstallMetalReflection(void){
    static BOOL installed;if(installed)return;if(![gameView.layer isKindOfClass:CAMetalLayer.class])return;
    gameMetalLayer=(CAMetalLayer *)gameView.layer;if(!reflectionQueue)reflectionQueue=dispatch_queue_create("dev.silksong.duo.reflection",DISPATCH_QUEUE_SERIAL);
    if(!framebufferOriginal)framebufferOriginal=(void *)HookOwnMethod(object_getClass(gameMetalLayer),@selector(setFramebufferOnly:),(IMP)FramebufferHook);
    gameMetalLayer.framebufferOnly=NO;if(!gameMetalLayer.device)return;
    id<MTLCommandQueue> queue=[gameMetalLayer.device newCommandQueue];id<MTLCommandBuffer> command=[queue commandBuffer];Class cls=object_getClass(command);if(!cls)return;
    scheduledOriginal=(void *)HookOwnMethod(cls,@selector(addScheduledHandler:),(IMP)ScheduledHook);commitOriginal=(void *)HookOwnMethod(cls,@selector(commit),(IMP)CommitHook);
    installed=scheduledOriginal&&commitOriginal;reflectionStatus=installed?@"waiting-for-live-frame":@"present-hook-unavailable";
}
