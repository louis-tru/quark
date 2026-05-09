#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <simd/simd.h>

struct Vertex {
	vector_float2 pos;
	vector_float4 color;
};

@interface MetalView : NSView
@end

@implementation MetalView {
	id<MTLDevice> _device;
	id<MTLCommandQueue> _queue;
	id<MTLRenderPipelineState> _pipeline;
	id<MTLBuffer> _vertexBuffer;
}

// + (Class)layerClass {
// 	return [CAMetalLayer class];
// }

- (instancetype)initWithFrame:(NSRect)frame {
	self = [super initWithFrame:frame];
	if (!self) return nil;

	_device = MTLCreateSystemDefaultDevice();
	_queue = [_device newCommandQueue];

	CAMetalLayer *metalLayer = [CAMetalLayer layer];
	metalLayer.device = _device;
	metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	metalLayer.framebufferOnly = YES;
	metalLayer.contentsScale = NSScreen.mainScreen.backingScaleFactor;

	self.wantsLayer = YES;
	self.layer = metalLayer;

	[self buildPipeline];
	[self buildGeometry];

	CVDisplayLinkRef displayLink;
	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
	CVDisplayLinkSetOutputCallback(displayLink, displayCallback, (__bridge void *)self);
	CVDisplayLinkStart(displayLink);

	return self;
}

static CVReturn displayCallback(
	CVDisplayLinkRef,
	const CVTimeStamp *,
	const CVTimeStamp *,
	CVOptionFlags,
	CVOptionFlags *,
	void *ctx
) {
	@autoreleasepool {
		MetalView *view = (__bridge MetalView *)ctx;
		dispatch_async(dispatch_get_main_queue(), ^{
			[view drawFrame];
		});
	}
	return kCVReturnSuccess;
}

- (void)buildPipeline {
	NSString *src =
		@"#include <metal_stdlib>\n"
		"using namespace metal;\n"
		"\n"
		"struct VertexIn {\n"
		"    float2 pos;\n"
		"    float4 color;\n"
		"};\n"
		"\n"
		"struct VertexOut {\n"
		"    float4 position [[position]];\n"
		"    float4 color;\n"
		"};\n"
		"\n"
		"vertex VertexOut vs_main(\n"
		"    const device VertexIn *vertices [[buffer(0)]],\n"
		"    uint vid [[vertex_id]]\n"
		") {\n"
		"    VertexIn v = vertices[vid];\n"
		"    VertexOut out;\n"
		"    out.position = float4(v.pos, 0.0, 1.0);\n"
		"    out.color = v.color;\n"
		"    return out;\n"
		"}\n"
		"\n"
		"fragment float4 fs_main(VertexOut in [[stage_in]]) {\n"
		"    return in.color;\n"
		"}\n";

	NSError *err = nil;
	id<MTLLibrary> lib = [_device newLibraryWithSource:src options:nil error:&err];
	if (!lib) {
		NSLog(@"Metal shader error: %@", err);
		abort();
	}

	id<MTLFunction> vs = [lib newFunctionWithName:@"vs_main"];
	id<MTLFunction> fs = [lib newFunctionWithName:@"fs_main"];

	MTLRenderPipelineDescriptor *desc = [MTLRenderPipelineDescriptor new];
	desc.vertexFunction = vs;
	desc.fragmentFunction = fs;
	desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

	_pipeline = [_device newRenderPipelineStateWithDescriptor:desc error:&err];
	if (!_pipeline) {
		NSLog(@"Pipeline error: %@", err);
		abort();
	}
}

- (void)buildGeometry {
	Vertex vertices[] = {
		{{ 0.0f,  0.7f}, {1, 0, 0, 1}},
		{{-0.7f, -0.7f}, {0, 1, 0, 1}},
		{{ 0.7f, -0.7f}, {0, 0, 1, 1}},
	};

	_vertexBuffer = [_device newBufferWithBytes:vertices
										 length:sizeof(vertices)
										options:MTLResourceStorageModeShared];
}

- (void)drawFrame {
	CAMetalLayer *layer = (CAMetalLayer *)self.layer;
	layer.drawableSize = CGSizeMake(
		self.bounds.size.width * self.window.backingScaleFactor,
		self.bounds.size.height * self.window.backingScaleFactor
	);

	id<CAMetalDrawable> drawable = [layer nextDrawable];
	if (!drawable) return;

	MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
	pass.colorAttachments[0].texture = drawable.texture;
	pass.colorAttachments[0].loadAction = MTLLoadActionClear;
	pass.colorAttachments[0].storeAction = MTLStoreActionStore;
	pass.colorAttachments[0].clearColor = MTLClearColorMake(0.08, 0.08, 0.10, 1.0);

	id<MTLCommandBuffer> cmd = [_queue commandBuffer];
	id<MTLRenderCommandEncoder> enc = [cmd renderCommandEncoderWithDescriptor:pass];

	[enc setRenderPipelineState:_pipeline];
	[enc setVertexBuffer:_vertexBuffer offset:0 atIndex:0];
	[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
	[enc endEncoding];

	[cmd presentDrawable:drawable];
	[cmd commit];
}

@end

int main(int argc, const char *argv[]) {
	@autoreleasepool {
		NSApplication *app = [NSApplication sharedApplication];
		[app setActivationPolicy:NSApplicationActivationPolicyRegular];

		NSRect frame = NSMakeRect(100, 100, 800, 600);

		NSWindow *win = [[NSWindow alloc] initWithContentRect:frame
																									styleMask:NSWindowStyleMaskTitled |
																														NSWindowStyleMaskClosable |
																														NSWindowStyleMaskResizable
																										backing:NSBackingStoreBuffered
																											defer:NO];

		MetalView *view = [[MetalView alloc] initWithFrame:NSMakeRect(0, 0, 800, 600)];
		win.contentView = view;

		[win center];
		[win makeKeyAndOrderFront:nil];
		[NSApp activateIgnoringOtherApps:YES];

		[app run];
	}

	return 0;
}

// clang++ test-metal.mm -std=c++17 -fobjc-arc \
//   -framework Cocoa \
//   -framework Metal \
//   -framework QuartzCore \
//   -framework CoreVideo \
//   -o test-demo