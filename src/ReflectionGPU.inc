#import <Metal/Metal.h>
#import <MetalPerformanceShaders/MetalPerformanceShaders.h>
@interface DuoReflectionGPU : NSObject
@property(nonatomic,strong)id<MTLDevice> device;
@property(nonatomic,strong)id<MTLTexture> scaled,blurred;
@property(nonatomic,strong)id<MTLBuffer> readback;
@property(nonatomic,strong)MPSImageBilinearScale *resize;
@property(nonatomic,strong)MPSImageGaussianBlur *blur;
@property(nonatomic)NSUInteger width,height,row,maxEdge;
- (BOOL)encodeSource:(id<MTLTexture>)source command:(id<MTLCommandBuffer>)command;
@end
@implementation DuoReflectionGPU
- (BOOL)encodeSource:(id<MTLTexture>)source command:(id<MTLCommandBuffer>)command{
 if(!MPSSupportsMTLDevice(command.device))return NO;
 double scale=MIN(1,(self.maxEdge?:256.0)/MAX(source.width,source.height));NSUInteger width=MAX(1,lround(source.width*scale)),height=MAX(1,lround(source.height*scale));
 if(self.device!=command.device||self.width!=width||self.height!=height||!self.scaled||!self.blurred||!self.readback){
  self.device=command.device;self.width=width;self.height=height;self.row=((width*4+255)/256)*256;
  MTLTextureDescriptor *d=[MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:width height:height mipmapped:NO];d.usage=MTLTextureUsageShaderRead|MTLTextureUsageShaderWrite;d.storageMode=MTLStorageModePrivate;
  self.scaled=[command.device newTextureWithDescriptor:d];self.blurred=[command.device newTextureWithDescriptor:d];self.readback=[command.device newBufferWithLength:self.row*height options:MTLResourceStorageModeShared];
  self.resize=[[MPSImageBilinearScale alloc]initWithDevice:command.device];self.blur=[[MPSImageGaussianBlur alloc]initWithDevice:command.device sigma:6];self.blur.edgeMode=MPSImageEdgeModeClamp;
 }
 if(!self.scaled||!self.blurred||!self.readback||!self.resize||!self.blur)return NO;
 [self.resize encodeToCommandBuffer:command sourceTexture:source destinationTexture:self.scaled];
 [self.blur encodeToCommandBuffer:command sourceTexture:self.scaled destinationTexture:self.blurred];
 id<MTLBlitCommandEncoder> blit=[command blitCommandEncoder];if(!blit)return NO;
 [blit copyFromTexture:self.blurred sourceSlice:0 sourceLevel:0 sourceOrigin:MTLOriginMake(0,0,0) sourceSize:MTLSizeMake(width,height,1) toBuffer:self.readback destinationOffset:0 destinationBytesPerRow:self.row destinationBytesPerImage:self.row*height];[blit endEncoding];return YES;
}
@end
