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

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#define WINDOW_SIZE 500

int test2_x11(int argc, char *argv[]) {

	// 连接到 X Server，创建到 X Server 的套接字连接
	Display* dpy = XOpenDisplay(NULL);

	XSetWindowAttributes attrs;
	// 创建 200X200 的白色背景窗口
	attrs.background_pixel = XWhitePixel(dpy, 0);

	Window win = XCreateWindow(
		dpy,
		XRootWindow(dpy, 0),
		0,
		0,
		WINDOW_SIZE,
		WINDOW_SIZE,
		0,
		DefaultDepth(dpy, 0),
		InputOutput,
		DefaultVisual(dpy, 0),
		CWBackPixel,
		&attrs
	);

	// 选择输入事件。
	XSelectInput(dpy, win, ExposureMask | KeyPressMask);

	// 创建绘图上下文
	GC gc = XCreateGC(dpy, win, 0, NULL);

	//Map 窗口
	XMapWindow(dpy, win);

	// 事件主循环。主要处理 Expose 事件和 KeyPress 事件
	while(1) {
		XKeyEvent event;
		XNextEvent(dpy,(XEvent*)&event);

		switch(event.type) {
			case Expose: // 处理 Expose 事件
				// 绘制 100 个点
				XWindowAttributes attrs;
				XGetWindowAttributes(dpy, win, &attrs);
				printf("%s,width: %d, height: %d\n", "draw", attrs.width, attrs.height);
				for (int i = 0; i < WINDOW_SIZE / 2; i++)
					XDrawPoint(dpy, win, gc, WINDOW_SIZE / 4 + i, WINDOW_SIZE / 2);
				break;
			case KeyPress: // 处理按键事件
				XFreeGC(dpy, gc);
				XCloseDisplay(dpy);
				exit(0);
				break;
			default: break;
		}
	}

	return 0;
}
