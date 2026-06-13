/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#include "src/ui/app.h"
#include "src/platforms/apple/apple_app.h"
#include "../test.h"
#include "./mtl_compute_aa_prototype.h"

#if Qk_MacOS

#import <AppKit/AppKit.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#include <math.h>

using namespace qk;

static Path make_compute_aa_path(float t) {
	Path path;
	Vec2 center(260, 260);
	float rot = t * 0.9f;
	for (int i = 0; i < 10; i++) {
		float r = (i & 1) ? 84.0f : 190.0f;
		float a = rot + float(i) * Qk_PI * 0.2f - Qk_PI_2_1;
		Vec2 p(center.x() + cosf(a) * r, center.y() + sinf(a) * r);
		if (i == 0) path.moveTo(p);
		else path.lineTo(p);
	}
	path.close();

	path.moveTo({110, 260});
	path.cubicTo({160, 35}, {360, 485}, {410, 260});
	path.cubicTo({360, 35}, {160, 485}, {110, 260});
	path.close();
	return path;
}

@interface ComputeAAView : NSView
@end

@implementation ComputeAAView {
	CAMetalLayer *_layer;
	id<MTLDevice> _device;
	id<MTLCommandQueue> _queue;
	id<MTLComputePipelineState> _uniformPipeline;
	id<MTLComputePipelineState> _coveragePipeline;
	id<MTLRenderPipelineState> _compositePipeline;
	LinearAllocator _alloc;
	NSTimer *_timer;
	uint64_t _frame;
	double _gpuTimeTotal;
	uint32_t _gpuTimeSamples;
}

- (CALayer*)makeBackingLayer {
	return [CAMetalLayer layer];
}

- (instancetype)initWithFrame:(NSRect)frame {
	self = [super initWithFrame:frame];
	if (!self) return nil;
	self.wantsLayer = YES;
	_device = MTLCreateSystemDefaultDevice();
	_queue = [_device newCommandQueue];
	_layer = (CAMetalLayer*)self.layer;
	_layer.device = _device;
	_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	_layer.framebufferOnly = NO;
	_layer.contentsScale = NSScreen.mainScreen.backingScaleFactor;

	NSError *err = nil;
	NSString *shaderPath = [NSBundle.mainBundle pathForResource:@"compute_aa_prototype" ofType:@"metal"];
	NSString *shaderSource = [NSString stringWithContentsOfFile:shaderPath
																										 encoding:NSUTF8StringEncoding
																												error:&err];
	Qk_ASSERT(shaderSource, "Read compute AA shader resource failed: %s", err.localizedDescription.UTF8String);

	id<MTLLibrary> lib = [_device newLibraryWithSource:shaderSource options:nil error:&err];
	if (!lib) {
		NSLog(@"Compute AA Metal shader error: %@", err);
		return self;
	}
	_uniformPipeline = [_device newComputePipelineStateWithFunction:[lib newFunctionWithName:@"qk_compute_aa_uniform_tiles"] error:&err];
	_coveragePipeline = [_device newComputePipelineStateWithFunction:[lib newFunctionWithName:@"qk_compute_aa_coverage"] error:&err];
	MTLRenderPipelineDescriptor *compositeDesc = [MTLRenderPipelineDescriptor new];
	compositeDesc.label = @"Compute AA Composite Pipeline";
	compositeDesc.vertexFunction = [lib newFunctionWithName:@"qk_compute_aa_composite_vertex"];
	compositeDesc.fragmentFunction = [lib newFunctionWithName:@"qk_compute_aa_composite_fragment"];
	auto colorAttachment = compositeDesc.colorAttachments[0];
	colorAttachment.pixelFormat = _layer.pixelFormat;
	colorAttachment.blendingEnabled = YES;
	colorAttachment.sourceRGBBlendFactor = MTLBlendFactorOne;
	colorAttachment.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
	colorAttachment.sourceAlphaBlendFactor = MTLBlendFactorOne;
	colorAttachment.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
	_compositePipeline = [_device newRenderPipelineStateWithDescriptor:compositeDesc error:&err];
	if (!_uniformPipeline || !_coveragePipeline || !_compositePipeline) {
		NSLog(@"Compute AA pipeline error: %@", err);
	}

	_timer = [NSTimer scheduledTimerWithTimeInterval:1.0 / 60.0
		target:self selector:@selector(drawFrame) userInfo:nil repeats:YES];
	return self;
}

- (void)setFrameSize:(NSSize)newSize {
	[super setFrameSize:newSize];
	CGFloat scale = self.window.backingScaleFactor ?: NSScreen.mainScreen.backingScaleFactor;
	_layer.drawableSize = CGSizeMake(newSize.width * scale, newSize.height * scale);
}

- (void)viewDidMoveToWindow {
	[super viewDidMoveToWindow];
	[self setFrameSize:self.bounds.size];
}

- (void)drawFrame {
	if (!_device || !_queue || !_uniformPipeline || !_coveragePipeline || !_compositePipeline)
		return;

	id<CAMetalDrawable> drawable = [_layer nextDrawable];
	if (!drawable)
		return;

	_alloc.reset();
	AllocatorScope scope(&_alloc); // 这个测试里所有的临时数据都放在 _alloc 里

	float time = float(_frame++) / 5000.0f;
	// float time = float(_frame++) / 60.0f;
	// float time = 0;
	Path path = make_compute_aa_path(time);
	ComputeAADrawData data = MetalComputeAAPrototype::buildDrawData(path, Mat(0,{3},0,0), 1.0f);
	if (!data.edges.length() || data.atlasSize.is_zero_axis())
		return;

	MTLTextureDescriptor *coverageDesc = [MTLTextureDescriptor
		texture2DDescriptorWithPixelFormat:MTLPixelFormatR8Unorm
		width:uint32_t(data.atlasSize.x())
		height:uint32_t(data.atlasSize.y())
		mipmapped:NO];
	coverageDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
	id<MTLTexture> coverage = [_device newTextureWithDescriptor:coverageDesc];

	id<MTLCommandBuffer> cmd = [_queue commandBuffer];
	cmd.label = @"Compute AA Frame";
	MetalComputeAAPrototype::encodeCoverage(_device, cmd, _uniformPipeline,
		_coveragePipeline, coverage, data, kComputeAAEvenOdd_FillRule);
	Vec2 origin(
		(float(drawable.texture.width) - data.atlasSize.x()) * 0.5f,
		(float(drawable.texture.height) - data.atlasSize.y()) * 0.5f
	);
	MetalComputeAAPrototype::encodeSolidComposite(cmd, _compositePipeline, coverage,
		drawable.texture, Vec4(0.5f, 0.09f, 0.10f, 1.0f),
		Vec4(0.2f, 0.8f, 0.50f, 1.0f), origin, data);

	[cmd addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
		double gpuMs = (buffer.GPUEndTime - buffer.GPUStartTime) * 1000.0;
		if (gpuMs > 0.0) {
			_gpuTimeTotal += gpuMs;
			_gpuTimeSamples++;
			if (_gpuTimeSamples == 120) {
				NSLog(@"Compute AA GPU average: %.3f ms over %u frames",
					_gpuTimeTotal / double(_gpuTimeSamples), _gpuTimeSamples);
				_gpuTimeTotal = 0.0;
				_gpuTimeSamples = 0;
			}
		}
	}];
	[cmd presentDrawable:drawable];
	[cmd commit];

	// self.window.title = [NSString stringWithFormat:
	// 	@"Compute AA prototype - edges:%u boundary:%u uniform:%u atlas:%ux%u",
	// 	data.edges.length(), data.boundaryTiles.length(), data.uniformTiles.length(),
	// 	uint32_t(data.atlasSize.x()), uint32_t(data.atlasSize.y())];
}

@end

void test_compute_aa_new_window(Cb::Data &e) {
	NSRect rect = NSMakeRect(0, 0, 760, 760);
	NSWindow *window = [[NSWindow alloc] initWithContentRect:rect
		styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
			NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
		backing:NSBackingStoreBuffered
		defer:NO];
	window.title = @"Compute AA prototype";

	ComputeAAView *view = [[ComputeAAView alloc] initWithFrame:rect];
	window.contentView = view;
	[[NSNotificationCenter defaultCenter] addObserverForName:NSWindowWillCloseNotification
		object:window queue:nil usingBlock:^(NSNotification*) {
			[NSApp stop:nil];
		}];
	[window center];
	[window makeKeyAndOrderFront:nil];
}

Qk_TEST_Func(compute_aa) {
	App app;
	post_message_main(Cb(test_compute_aa_new_window), false);
	app.run();
}

#endif // Qk_MacOS
