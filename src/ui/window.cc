/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./ui.h"
#include "./view/root.h"
#include "../render/render.h"
#include "./painter.h"
#include "./event.h"
#include "./text/text_blob.h"
#include "./action/action.h"
#include "../render/font/pool.h"
#include "./app.h"
#include "../util/thread/inl.h"

#ifndef PRINT_RENDER_FRAME_TIME
# define PRINT_RENDER_FRAME_TIME 0
#endif

namespace qk {
	static Array<Window*> memberRecycle;

	UILock::UILock(Window *win): _win(win), _lock(true) {
		win->_renderMutex.lock();
	}

	UILock::~UILock() {
		unlock();
	}

	void UILock::lock() {
		if (!_lock) {
			_lock = true;
			_win->_renderMutex.lock();
		}
	}

	void UILock::unlock() {
		if (_lock) {
			_win->_renderMutex.unlock();
			_lock = false;
		}
	}

	Window::Window(Options &opts)
		: Qk_Init_Event(Change)
		, Qk_Init_Event(Background)
		, Qk_Init_Event(Foreground)
		, Qk_Init_Event(Close)
		, _host(shared_app())
		, _painter(nullptr)
		, _lockSize()
		, _size(), _scale(1)
		, _atomPixel(1)
		, _defaultScale(0)
		, _fsp(0)
		, _fspTick(0)
		, _fspTime(0)
		, _beginTime(0)
		, _lastTime(0)
		, _surfaceDisplayRange()
		, _preRender(this)
		, _impl(nullptr)
		, _opts(opts)
		, _debugMode(false)
		, _fspBlob(new TextBlob)
	{
		Qk_CHECK(_host);
		check_is_first_loop();
		_render = Render::Make({ opts.colorType, opts.msaa, opts.fps }, this);
		_dispatch = new EventDispatch(this);
		_painter = new Painter(this);
		_actionCenter = new ActionCenter(this);
		_backgroundColor = opts.backgroundColor;
		_clipRange.push({ Vec2{0,0},Vec2{0,0},Vec2{0,0} });
		_id = _host->_windows.pushBack(this);
		retain(); // strong ref count retain from application
		_root = new Root(this); // new root
		_root->set_background_color(_backgroundColor);
		_root->retain(); // strong ref
		openImpl(opts); // open platform window
		_root->focus();  // set focus
		// sizeof(Window);
	}

	FontPool* Window::fontPool() {
		return _host->fontPool();
	}

	RunLoop* Window::loop() {
		return _host->_loop;
	}

	View* Window::focusView() {
		return _dispatch->focusView();
	}

	Window* Window::Make(Options opts) {
		if (memberRecycle.length()) {
			auto win = memberRecycle.back();
			memberRecycle.pop();
			return new (win) Window(opts);
		} else {
			return new Window(opts);
		}
	}

	void Window::destroy() {
		Qk_CHECK(_render == nullptr);
		// noop, Reserve memory to avoid accidental references to it by subsequent objects
		// and add memory to cache for reuse
		memberRecycle.push(this);
		this->~Window(); // Only call destructor, do not call free memory
	}

	void Window::close() {
		if (tryClose()) {
			// avoid call when process exitting
			if (!is_process_exit()) {
				Qk_Trigger(Close);
			}
			release(); // release ref count from host windows
		}
	}

	bool Window::tryClose() {
		check_is_first_loop();
		UILock lock(this); // lock ui
		if (!_root)
			return false;

		beforeClose();

		// ------------------------
		_host->_windows.erase(_id);
		if (_host->_activeWindow == this) {
			Inl_Application(_host)->setActiveWindow(nullptr);
		}
		// ------------------------

		_root->remove_all_child(); // remove child view
		Releasep(_root); // release root view
		_preRender.flushAsyncCall(); // flush async call
		// ------------------------
		Releasep(_actionCenter);
		Releasep(_dispatch);
		Releasep(_painter);
		_preRender.flushAsyncCall(); // reflush async call
		_preRender.clearTasks(); // clear tasks
		// ------------------------
		lock.unlock(); // Avoid deadlocks with rendering threads
		Releasep(_render); // delete obj and stop render draw
		lock.lock(); // relock
		// ------------------------

		closeImpl(); // close platform window
		return true;
	}

	Vec2 Window::surfaceSize() const {
		return _surfaceDisplayRange.end - _surfaceDisplayRange.begin;
	}

	void Window::nextFrame(cCb& cb) {
		UILock lock(this);
		_nextFrame.pushBack(cb);
	}

	void Window::solveNextFrame() {
		if (_nextFrame.length()) {
			auto cb = new List<Cb>(std::move(_nextFrame));
			_host->loop()->post(Cb([this, cb](auto e) {
				Sp<List<Cb>> handle(cb);
				for ( auto& i : *cb ) {
					i->resolve();
				}
			}));
		}
	}

	void Window::set_size(Vec2 size) {
		float w = size.x(), h = size.y();
		if (w >= 0.0 && h >= 0.0) {
			UILock lock(this);
			if (_lockSize.x() != w || _lockSize.y() != h) {
				_lockSize = { w, h };
				reload(false);
			}
		} else {
			Qk_DLog("Lock size value can not be less than zero\n");
		}
	}

	void Window::set_debugMode(bool v) {
		_debugMode = v;
	}

	void Window::reload(bool isRt) { // Lock before calling
		Vec2 size = surfaceSize();
		float width = size.x();
		float height = size.y();

		if (_lockSize.x() == 0 && _lockSize.y() == 0) { // Use the system default most suitable size
			_size = { width / _defaultScale, height / _defaultScale };
		}
		else if (_lockSize.x() != 0) { // lock width
			_size = { _lockSize.x(), _lockSize.x() / width * height };
		}
		else if (_lockSize.y() != 0) { // lock height
			_size = { _lockSize.y() / height * width, _lockSize.y() };
		}
		else { // Use the system default most suitable size
			_size = { width / _defaultScale, height / _defaultScale };
		}

		_scale = (width + height) / (_size.x() + _size.y());
		_atomPixel = 1.0f / _scale;

		// set default draw region
		_clipRange.front() = {
			Vec2{0, 0},
			Vec2{_size.x(), _size.y()},
			Vec2{_size.x(), _size.y()},
		};

		loop()->post(Cb([this](Cb::Data& e) { // work loop call
			Qk_Trigger(Change); // trigger window change
		}));

		auto region = _surfaceDisplayRange;
		Vec2 start = Vec2(-region.begin.x() / _scale, -region.begin.y() / _scale);
		Vec2 end   = Vec2(region.size.x() / _scale + start.x(), region.size.y() / _scale + start.y());
		auto mat = Mat4::ortho(start.x(), end.x(), start.y(), end.y(), -1.0f, 1.0f);

		if (isRt) {
			_root->reload_rt();
		} else {
			_preRender.async_call([](auto self, auto arg) {
				self->_root->reload_rt();
			}, this, 0);
		}

		Qk_DLog("Display::updateSurface() %f, %f", region.size.x(), region.size.y());

		_render->getCanvas()->setSurface(mat, size, _scale);
	}

	void Window::onRenderBackendReload(Vec2 size) {
		auto defaultScale = getDefaultScale();
		auto range = getDisplayRange(size);
		if (size.x() != 0 && size.y() != 0 && defaultScale != 0) {
			Qk_DLog("Window::onRenderBackendReload defaultScale:%f, w:%f, h: %f",
				size.x(), size.y(), defaultScale);
			UILock lock(this);
			if ( _surfaceDisplayRange.begin != range.begin
				|| _surfaceDisplayRange.end != range.end
				|| _surfaceDisplayRange.size != size
				|| _defaultScale != defaultScale
			) {
				_surfaceDisplayRange = { range.begin, range.end, size };
				_defaultScale = defaultScale;
				reload(true);
			} else {
				_root->reload_rt();
			}
		}
	}

	bool Window::onRenderBackendDisplay() {
		UILock lock(this); // ui render lock
		if (!_root)
			return false;

		if (!_beginTime)
			_beginTime = time_monotonic();
		// if pause play, it just needs to use the _lastTime and also not change the _lastTime
		auto time = time_monotonic() - _beginTime;
		auto deltaTime = time - _lastTime;

		if ( deltaTime > 2e5 ) { // 200ms
			// Restart the timer when the time interval exceeds 200ms,
			// for example, when the application resumes from sleep mode
			int64_t diff = deltaTime - 2e5;
			time -= diff; // Adjust the current time
			_beginTime += diff; // Restart the timer
			deltaTime = 2e5;
		}
		_lastTime = time;

		if (!_preRender.solve(time, deltaTime)) {
			solveNextFrame();
			return false;
		}

		if (_lastTime - _fspTime > 1e6) { // 1ns * 1e6
			if (_debugMode && _fsp != _fspTick) {
				// text blob build fps
				Array<TextBlob> blob;
				TextLines lines(_root, TextAlign::Default, {0,0}, false);
				lines.set_ignore_single_white_space(true);
				TextBlobBuilder builder(&lines, _host->defaultTextOptions(), &blob);
				builder.set_text_size(32.0f);
				builder.make(String::format("%d FPS", _fspTick));
				lines.finish(); // finish lines
				*_fspBlob = std::move(blob.front()); // only one blob
			}
			_fsp = _fspTick;
			_fspTick = 0;
			_fspTime = _lastTime;
		}
		_fspTick++;

		_root->draw(_painter); // start drawing

		if (_debugMode) {
			// draw fps
			Paint paint;
			paint.fill.color = Color4f(1,1,1,1);
			paint.stroke.color = Color4f(0,0,0,1);
			paint.style = Paint::kStrokeAndFill_Style;
			paint.strokeWidth = 2.0f;
			_painter->canvas()->setMatrix(Mat());
			_painter->canvas()->drawTextBlob(&_fspBlob->blob, {70}, 32.0f, paint);
		}

		afterDisplay(); // draw something for platform

		solveNextFrame(); // solve frame

#if DEBUG && PRINT_RENDER_FRAME_TIME
		int64_t st = time_micro();
#endif
		_render->getCanvas()->swapBuffer();

#if DEBUG && PRINT_RENDER_FRAME_TIME
		int64_t ts2 = (time_micro() - st) / 1e3;
		if (ts2 > 16) {
			Qk_Log("Window swapBuffer time: %ld -------------- ", ts2);
		} else {
			Qk_Log("Window swapBuffer time: %ld", ts2);
		}
#endif
		return true;
	}

	void Window::clipRange(Range clip) {
		RangeSize re = {
			Vec2{clip.begin.x(), clip.begin.y()}, Vec2{clip.end.x(), clip.end.y()}, Vec2{0,0}
		};
		RangeSize dre = _clipRange.back();

		// Compute an intersection area
		float x, x2, y, y2;

		y = dre.end.y() > re.end.y() ? re.end.y() : dre.end.y(); // choose a small
		y2 = dre.begin.y() > re.begin.y() ? dre.begin.y() : re.begin.y(); // choose a large
		x = dre.end.x() > re.end.x() ? re.end.x() : dre.end.x(); // choose a small
		x2 = dre.begin.x() > re.begin.x() ? dre.begin.x() : re.begin.x(); // choose a large

		if ( x > x2 ) {
			re.begin.set_x(x2);
			re.end.set_x(x);
		} else {
			re.begin.set_x(x);
			re.end.set_x(x2);
		}

		if ( y > y2 ) {
			re.begin.set_y(y2);
			re.end.set_y(y);
		} else {
			re.begin.set_y(y);
			re.end.set_y(y2);
		}

		re.size = Vec2(re.end.x() - re.begin.x(), re.end.y() - re.begin.y());

		_clipRange.push(re);
	}

	void Window::clipRestore() {
		Qk_ASSERT(_clipRange.length() > 1);
		_clipRange.pop();
	}
}
