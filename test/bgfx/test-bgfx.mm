
#import "./test.h"
#import <src/ui/app.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#import <AppKit/AppKit.h>
#import <bgfx/bgfx.h>
#import <bgfx/platform.h>
#import <src/platforms/apple/apple_app.h>
#import "./bgfx/shader_data.h"

using namespace qk;

@interface BgfxView : NSView
@end

@implementation BgfxView

+ (Class)layerClass {
	return [CAMetalLayer class];
}

- (instancetype)initWithFrame:(NSRect)frame {
	self = [super initWithFrame:frame];
	if (self) {
		self.wantsLayer = YES;
		// self.wantsLayer = NO; // gl
		self.layer = [CAMetalLayer layer];
		self.layer.opaque = YES;
	}
	return self;
}
@end

@interface BGFXWindowDelegate : NSObject<NSWindowDelegate>
@property NSWindow* window;
@property BgfxView* view;
@property NSTimer* timer;
@property BOOL bgfxReady;
@property bgfx::VertexBufferHandle vbh;
@property bgfx::ProgramHandle program;
@end

struct PosColorVertex {
	float x, y, z;
	uint32_t abgr;
	static bgfx::VertexLayout layout;
	static void init() {
		layout.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();
	}
};

bgfx::VertexLayout PosColorVertex::layout;

static PosColorVertex s_triangleVertices[] = {
	{  0.0f,  0.5f, 0.0f, 0xff0000ff }, // 红
	{ -0.5f, -0.5f, 0.0f, 0xff00ff00 }, // 绿
	{  0.5f, -0.5f, 0.0f, 0xffff0000 }, // 蓝
};

@implementation BGFXWindowDelegate

+ (NSScreen *)screenUnderMouse {
	NSPoint mouseLocation = [NSEvent mouseLocation];
	for (NSScreen *screen in [NSScreen screens]) {
		if (NSPointInRect(mouseLocation, [screen frame]))
			return screen;
	}
	return [NSScreen mainScreen];
}

- (id) init {
	if ( !(self = [super init]) )
		return nil;

	NSWindowStyleMask style =
		//NSWindowStyleMaskBorderless |
		NSWindowStyleMaskBorderless | NSWindowStyleMaskTitled |
		NSWindowStyleMaskClosable |
		NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;
	NSScreen *screen = BGFXWindowDelegate.screenUnderMouse;
	NSRect screenFrame = screen.visibleFrame;

	float w = screenFrame.size.width / 2;
	float h = screenFrame.size.height / 2;
	NSRect rect = NSMakeRect(NSMidX(screenFrame) - w / 2, NSMidY(screenFrame) - h / 2, w, h);

	self.window = [[UIWindow alloc] initWithContentRect:rect
																						styleMask:style
																							backing:NSBackingStoreBuffered
																								defer:NO
																								screen:screen];
	self.window.title = @"Qk bgfx native macOS demo";
	self.window.delegate = self;
	//self.window.opaque = NO;
	self.window.backgroundColor = [NSColor clearColor];
	[self.window setFrame:rect display:NO];

	self.view = [[BgfxView alloc] initWithFrame:self.window.contentView.bounds];
	self.view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
	[self.window.contentView addSubview:self.view];
	[self.window makeKeyAndOrderFront:nil];
	[self initBgfx];

	self.timer = [NSTimer scheduledTimerWithTimeInterval:1.0 / 60.0
		target:self
		selector:@selector(renderFrame)
		userInfo:nil
		repeats:YES];

	return self;
}

uint32_t _width, _height;

- (void)initBgfx {
	NSSize size = self.view.bounds.size;
	CGFloat scale = self.window.backingScaleFactor;
	_width  = (uint32_t)(size.width  * scale);
	_height = (uint32_t)(size.height * scale);
	CAMetalLayer* layer = (CAMetalLayer*)self.view.layer;
	layer.contentsScale = scale;
	layer.drawableSize = CGSizeMake(_width, _height);
	bgfx::PlatformData pd{};
	pd.nwh = (__bridge void*)layer;
	// pd.nwh = (__bridge void*)self.view; // gl
	pd.ndt = nullptr;
	pd.context = nullptr;
	pd.backBuffer = nullptr;
	pd.backBufferDS = nullptr;
	bgfx::Init init{};
	init.type = bgfx::RendererType::Metal;
	// init.type = bgfx::RendererType::OpenGL;
	init.platformData = pd;
	init.resolution.width = _width;
	init.resolution.height = _height;
	init.resolution.reset = BGFX_RESET_VSYNC;

	if (!bgfx::init(init)) {
		NSLog(@"bgfx init failed");
		[NSApp terminate:nil];
		return;
	}

	bgfx::setDebug(BGFX_DEBUG_TEXT);
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x880000ff, 1.0f, 0);
	bgfx::reset(_width, _height, BGFX_RESET_VSYNC);
	bgfx::setViewRect(0, 0, 0, (uint16_t)_width, (uint16_t)_height);

	PosColorVertex::init();

	bgfx::ShaderHandle vsh = bgfx::createShader(bgfx::makeRef(test_v_sc, test_v_sc_len));
	bgfx::ShaderHandle fsh = bgfx::createShader(bgfx::makeRef(test_f_sc, test_f_sc_len));

	self.program = bgfx::createProgram(vsh, fsh, true);

	self.vbh = bgfx::createVertexBuffer(
		bgfx::makeRef(s_triangleVertices, sizeof(s_triangleVertices)),
		PosColorVertex::layout
	);

	self.bgfxReady = YES;
}

- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)size {
	CGFloat scale = self.window.backingScaleFactor;
	_width  = (uint32_t)(size.width  * scale);
	_height = (uint32_t)(size.height * scale);
	CAMetalLayer* layer = (CAMetalLayer*)self.view.layer;
	layer.contentsScale = scale;
	layer.drawableSize = CGSizeMake(_width, _height);
	bgfx::reset(_width, _height, BGFX_RESET_VSYNC);
	bgfx::setViewRect(0, 0, 0, (uint16_t)_width, (uint16_t)_height);
	return size;
}

- (void)renderFrame {
	if (!self.bgfxReady)
		return;

	bgfx::touch(0);

	bgfx::setVertexBuffer(0, self.vbh);
	bgfx::setState(BGFX_STATE_DEFAULT);
	bgfx::submit(0, self.program);

	bgfx::dbgTextClear();
	bgfx::dbgTextPrintf(0, 0, 0x4f, "Hello bgfx on native macOS");
	bgfx::dbgTextPrintf(0, 1, 0x6f, "Renderer: %s", bgfx::getRendererName(bgfx::getRendererType()));
	bgfx::dbgTextPrintf(0, 2, 0x2f, "Size: %u x %u", _width, _height);

	bgfx::frame();
}

- (void)windowWillClose:(NSNotification*)notification {
	[self.timer invalidate];
	self.timer = nil;
	if (self.bgfxReady) {
		self.bgfxReady = NO;
		bgfx::destroy(self.vbh);
		bgfx::destroy(self.program);
		bgfx::shutdown();
	}
	[NSApp terminate:nil];
}

- (BOOL) acceptsFirstResponder {
	return YES;
}

@end

Qk_TEST_Func(bgfx) {
	post_message_main(Cb([](auto e) {
		[BGFXWindowDelegate new];
	}), false);
	App app;
	app.run();
}
