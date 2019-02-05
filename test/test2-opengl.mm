/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#ifdef __APPLE__
#include <TargetConditionals.h>
# if TARGET_OS_MAC && !TARGET_OS_IPHONE

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl3.h>
#import <string>

@interface GameView: NSOpenGLView {
	CVDisplayLinkRef displayLink;
}
@end

@implementation GameView

const unsigned WIDTH = 1024;
const unsigned HEIGHT = 1024;

static GLuint spriteShader;

static const char vert_shad[] =
R"(
attribute vec2 pos;
void main() {
	gl_Position = vec4(pos.xy, 0.0, 1.0);
}
)";

static const char frag_shad[] =
R"(
void main() {
	gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
}
)";

void draw_sprites() {
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void draw_frame() {
	glViewport(0, 0, (int) WIDTH, HEIGHT);
	glClearColor(1.0, 0.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	draw_sprites();
	glFlush();
}

static void check_gl_error(const char* op) {
	std::string str_error;
	for (GLenum error = glGetError(); error; error = glGetError()) {
		switch(error) {
			case GL_NO_ERROR: str_error = "GL_NO_ERROR";break;
			case GL_INVALID_ENUM: str_error = "GL_INVALID_ENUM";break;
			case GL_INVALID_VALUE: str_error = "GL_INVALID_VALUE";break;
			case GL_INVALID_OPERATION: str_error = "GL_INVALID_OPERATION";break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: str_error = "GL_INVALID_FRAMEBUFFER_OPERATION";break;
			case GL_OUT_OF_MEMORY: str_error = "GL_OUT_OF_MEMORY";break;
			default: str_error = "unknown";
		}
			NSLog(@"after %s glError %s\n", op, str_error.c_str());
		}
}

GLuint load_shader(GLenum type, const char *shaderSrc) {
	GLuint shader;
	GLint compiled;

	shader = glCreateShader(type);
	if (shader == 0) return 0;

	glShaderSource(shader, 1, &shaderSrc, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (!compiled) {
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1) {
			char *infoLog = static_cast<char *>(malloc(sizeof(char) * static_cast<unsigned long>(infoLen)));

			glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			NSLog(@"Error compiling %s\n\n%s", infoLog, shaderSrc);
			free(infoLog);
		}
		glDeleteShader(shader);
		exit(0);
	}
	return shader;
}

void check_link_error(
	GLint linked,
	GLuint programId,
	const char *vertexshader,
	const char *fragmentShader)
{
	if(!linked) {
		GLint infoLen = 0;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLen);
		if(infoLen > 1) {
			char* infoLog = static_cast<char *>(malloc(sizeof(char) * static_cast<unsigned long>(infoLen)));
			glGetProgramInfoLog(programId, infoLen, NULL, infoLog);
			NSLog(@"Error linking program:\n%s", infoLog);
			free(infoLog);
		}
		glDeleteProgram(GLuint(linked));
		NSLog(@"Error when linking:\n %s\n\n\n%s", vertexshader, fragmentShader);
		exit(2);
	}
}

GLuint load_gl_program(const char *vertexShader, const char *fragmentShader) {
	GLuint vs, fs;
	vs = load_shader(GL_VERTEX_SHADER, vertexShader);
	fs = load_shader(GL_FRAGMENT_SHADER, fragmentShader);

	GLuint programId = glCreateProgram();
	glAttachShader(programId, vs);
	check_gl_error("Attach vertex shader");
	glAttachShader(programId, fs);
	check_gl_error("Attach fragment shader");
	glLinkProgram(programId);

	GLint linked = GL_FALSE;
	glGetProgramiv(programId, GL_LINK_STATUS, &linked);
	check_link_error(linked, programId, vertexShader, fragmentShader);
	return programId;
}

void setup_opengl() {
	const unsigned VERTICES = 4;
	static GLfloat vertices[] = {
		-1.0, -1.0, // vertex coord
		1.0, -1.0,
		-1.0, 1.0,
		1.0, 1.0,
	};
	GLuint vbuffer;
	glGenBuffers(1, &vbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	glBufferData(GL_ARRAY_BUFFER,
		VERTICES * 2 * sizeof(GLfloat),
		vertices, GL_STATIC_DRAW
	); 

	check_gl_error("bind buffer");

	spriteShader = load_gl_program(vert_shad, frag_shad);
	GLuint position = glGetAttribLocation(spriteShader, "pos");
	glEnableVertexAttribArray(position);
	glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glUseProgram(spriteShader);
}

static CVReturn display_link_callback(
	CVDisplayLinkRef displayLink,
	const CVTimeStamp* now,
	const CVTimeStamp* outputTime,
	CVOptionFlags flagsIn,
	CVOptionFlags* flagsOut,
	void* displayLinkContext)
{
	CVReturn result = [(__bridge GameView*)displayLinkContext getFrameForTime:outputTime];
	return result;
}

- (void) prepareOpenGL {
	
	// Synchronize buffer swaps with vertical refresh rate
	GLint swapInt = 1;
	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];

	// Create a display link capable of being used with all active displays
	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);

	// Set the renderer output callback function
	CVDisplayLinkSetOutputCallback(displayLink, &display_link_callback, (__bridge void*)self);
	
	// Set the display link for the current renderer
	CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
	CGLPixelFormatObj cglPixelFormat = (__bridge CGLPixelFormatObj)([self createPixelFormat]);
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);

	// Activate the display link
	CVDisplayLinkStart(displayLink);

	// -------------- setup opengl ----------------
	setup_opengl();
}

- (CVReturn) getFrameForTime:(const CVTimeStamp*)outputTime {
	// Add your drawing codes here
	NSOpenGLContext *currentContext = [self openGLContext];
	[currentContext makeCurrentContext];

	// must lock GL context because display link is threaded
	CGLLockContext(static_cast<CGLContextObj>([currentContext CGLContextObj]));

	// Add your drawing codes here
	draw_frame();

	[currentContext flushBuffer];

	CGLUnlockContext(static_cast<CGLContextObj>([currentContext CGLContextObj]));
	return kCVReturnSuccess;
}

- (NSOpenGLPixelFormat*) createPixelFormat {
	NSOpenGLPixelFormat *pixelFormat;

	NSOpenGLPixelFormatAttribute attribs[] =
	{
		NSOpenGLPFANoRecovery,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize,        32,
		NSOpenGLPFAAlphaSize,        8,
		NSOpenGLPFAScreenMask,
		CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
		0
	};

	pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
	return pixelFormat;
}

- (void) dealloc {
	// Release the display link
	CVDisplayLinkRelease(displayLink);
}

@end

int test2_opengl(int argc, char *argv[]) {
	@autoreleasepool {
		[NSApplication sharedApplication];
		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

		CGRect rect = NSMakeRect(0, 0, WIDTH, HEIGHT);

		id window = [[NSWindow alloc]
			initWithContentRect:rect
				styleMask:NSWindowStyleMaskTitled |
								 NSWindowStyleMaskClosable |
								 NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
					backing:NSBackingStoreBuffered
						defer:NO];

		[window setTitle:@"OpenGL Test"];
		[window makeKeyAndOrderFront:nil];
		[NSApp activateIgnoringOtherApps:YES];

		GameView *view = [[GameView alloc] initWithFrame:rect];
		[view setWantsLayer:YES];

		[[window contentView] addSubview:view];

		[NSApp run];
	}
	return 0;
}

# else
int test2_opengl(int argc, char *argv[]) { return 0; }
# endif
#else
int test2_opengl(int argc, char *argv[]) { return 0; }
#endif
