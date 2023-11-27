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

#include "./window.h"
#include "./app.h"
#include "./layout/root.h"
#include "../render/render.h"
#include "./view_render.h"
#include "./event.h"
#include "./task.h"
#include "./text/text_opts.h"

#ifndef PRINT_RENDER_FRAME_TIME
# define PRINT_RENDER_FRAME_TIME 0
#endif

namespace qk {

	UILock::UILock(Window* win): _win(win), _lock(false) {
		lock();
	}

	UILock::~UILock() {
		unlock();
	}

	void UILock::lock() {
		if (!_lock) {
			_lock = true;
			_win->_render_mutex.lock();
		}
	}

	void UILock::unlock() {
		if (_lock) {
			_win->_render_mutex.unlock();
			_lock = false;
		}
	}


	Window::Window(Options &opts)
		: Qk_Init_Event(Change)
		, Qk_Init_Event(Background)
		, Qk_Init_Event(Foreground)
		, _host(shared_app())
		, _viewRender(nullptr)
		, _lockSize()
		, _size(), _scale(1)
		, _atomPixel(1)
		, _defaultScale(0)
		, _fsp(0)
		, _nextFsp(0)
		, _nextFspTime(0), _surfaceRegion()
		, _mark_total(0)
		, _marks(0)
		, _is_render(false)
	{
		Qk_STRICT_ASSERT(_host);
		_clipRegion.push({ Vec2{0,0},Vec2{0,0},Vec2{0,0} });
		_render = Render::Make({ opts.colorType, opts.msaa, opts.fps }, this);
		_dispatch = new EventDispatch(this);
		_viewRender = new ViewRender(this);
		_backgroundColor = opts.backgroundColor;
		{
			ScopeLock lock(_host->_mutex);
			_id = _host->_windows.pushBack(this);
		}
		retain(); // strong ref count retain
		_root = new Root(this); // new root
		_root->retain(); // strong ref
		openImpl(opts); // open platform window
		_root->focus();  // set focus
	}

	Window::~Window() {
		destroy();
	}

	void Window::close() {
		if (destroy()) {
			release(); // release ref count from host windows
		}
	}

	bool Window::destroy() {
		UILock lock; // lock ui
		if (!_render) return false;
		lock.unlock(); // Avoid deadlocks with rendering threads
		Release(_render); _render = nullptr; // delete obj and stop render draw
		lock.lock(); // relock
		_root->remove(); // remove child view
		Release(_root);      _root = nullptr;
		Release(_dispatch); _dispatch = nullptr;
		Release(_viewRender); _viewRender = nullptr;
		for (auto t: _tasks) {
			t->_win = nullptr; // clear task
		}
		_host->_windows.erase(_id);
		if (_host->_activeWindow == this) {
			Inl_Application(_host)->setActiveWindow(nullptr);
		}
		closeImpl(); // close platform window
		return true;
	}

	void Window::clipRegion(Region clip) {
		RegionSize re = {
			Vec2{clip.origin.x(), clip.origin.y()}, Vec2{clip.end.x(), clip.end.y()}, Vec2{0,0}
		};
		RegionSize dre = _clipRegion.back();

		// Compute an intersection area
		float x, x2, y, y2;

		y = dre.end.y() > re.end.y() ? re.end.y() : dre.end.y(); // choose a small
		y2 = dre.origin.y() > re.origin.y() ? dre.origin.y() : re.origin.y(); // choose a large
		x = dre.end.x() > re.end.x() ? re.end.x() : dre.end.x(); // choose a small
		x2 = dre.origin.x() > re.origin.x() ? dre.origin.x() : re.origin.x(); // choose a large

		if ( x > x2 ) {
			re.origin.set_x(x2);
			re.end.set_x(x);
		} else {
			re.origin.set_x(x);
			re.end.set_x(x2);
		}

		if ( y > y2 ) {
			re.origin.set_y(y2);
			re.end.set_y(y);
		} else {
			re.origin.set_y(y);
			re.end.set_y(y2);
		}

		re.size = Vec2(re.end.x() - re.origin.x(), re.end.y() - re.origin.y());

		_clipRegion.push(re);
	}

	void Window::clipRestore() {
		Qk_ASSERT( _clipRegion.length() > 1 );
		_clipRegion.pop();
	}

	void Window::nextFrame(cCb& cb) {
		UILock lock(this);
		_nextFrame.pushBack(cb);
	}

	void Window::solveNextFrame() {
		if (_nextFrame.length()) {
			List<Cb>* cb = new List<Cb>(std::move(_nextFrame));
			_host->loop()->post(Cb([this, cb](Cb::Data& e) {
				UILock lock(this);
				Handle<List<Cb>> handle(cb);
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
				reload();
			}
		} else {
			Qk_DEBUG("Lock size value can not be less than zero\n");
		}
	}

	void Window::reload() { // Lock before calling
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
		_clipRegion.front() = {
			Vec2{0, 0},
			Vec2{_size.x(), _size.y()},
			Vec2{_size.x(), _size.y()},
		};

		_host->loop()->post(Cb([this](Cb::Data& e) { // main loop call
			Qk_Trigger(Change); // trigger window change
		}));

		auto region = _surfaceRegion;
		Vec2 start = Vec2(-region.origin.x() / _scale, -region.origin.y() / _scale);
		Vec2 end   = Vec2(region.size.x() / _scale + start.x(), region.size.y() / _scale + start.y());
		auto mat = Mat4::ortho(start.x(), end.x(), start.y(), end.y(), -1.0f, 1.0f);

		_root->reload();

		Qk_DEBUG("Display::updateSurface() %f, %f", region.size.x(), region.size.y());

		_render->getCanvas()->setSurface(mat, size, _scale);
	}

	void Window::onRenderBackendReload(Region region, Vec2 size, float defaultScale) {
		if (size.x() != 0 && size.y() != 0 && defaultScale != 0) {
			Qk_DEBUG("Window::onDeviceReload");
			UILock lock(this);
			if ( _surfaceRegion.origin != region.origin
				|| _surfaceRegion.end != region.end
				|| _surfaceRegion.size != size
				|| _defaultScale != defaultScale
			) {
				_surfaceRegion = { region.origin, region.end, size };
				_defaultScale = defaultScale;
				reload();
			} else {
				_root->reload();
			}
		}
	}

	bool Window::onRenderBackendDisplay() {
		UILock lock(this); // ui render lock

		if (!preRender()) {
			solveNextFrame();
			return false;
		}
		int64_t now_time = time_second();

		if (now_time - _nextFspTime >= 1) { // 1s
			_fsp = _nextFsp;
			_nextFsp = 0;
			_nextFspTime = now_time;
			Qk_DEBUG("fps: %d", _fsp);
		}
		_nextFsp++;

		_root->accept(_viewRender); // start drawing

		solveNextFrame(); // solve frame

#if DEBUG && PRINT_RENDER_FRAME_TIME
		int64_t st = time_micro();
#endif

		_render->getCanvas()->swapBuffer();

#if DEBUG && PRINT_RENDER_FRAME_TIME
		int64_t ts2 = (time_micro() - st) / 1e3;
		if (ts2 > 16) {
			Qk_LOG("ts: %ld -------------- ", ts2);
		} else {
			Qk_LOG("ts: %ld", ts2);
		}
#endif

		return true;
	}

	// ----------------------------- pre render -----------------------------

	void Window::mark_layout(Layout *layout, uint32_t level) {
		Qk_ASSERT(level);
		_marks.extend(level + 1);
		auto& arr = _marks[level];
		layout->_mark_index = arr.length();
		arr.push(layout);
		_mark_total++;
	}

	void Window::unmark_layout(Layout *layout, uint32_t level) {
		Qk_ASSERT(level);
		auto& arr = _marks[level];
		auto last = arr[arr.length() - 1];
		if (last != layout) {
			arr[layout->_mark_index] = last;
		}
		arr.pop();
		layout->_mark_index = -1;
		_mark_total--;
		_is_render = true;
	}

	void Window::mark_render() {
		_is_render = true;
	}

	void Window::addtask(Task* task) {
		if ( task->task_id() == Task::ID() ) {
			Task::ID id = _tasks.pushBack(task);
			task->_task_id = id;
			task->_win = this;
		}
	}

	void Window::untask(Task* task) {
		Task::ID id = task->task_id();
		if ( id != Task::ID() ) {
			(*id)->_win = nullptr;
			(*id)->_task_id = Task::ID();
			(*id) = nullptr;
		}
	}

	void Window::solveMarks() {
		TextConfig cfg(_host->defaultTextOptions(), _host->defaultTextOptions()->base());

		do {
			{ // forward iteration
				for (auto& levelMarks: _marks) {
					for (auto& layout: levelMarks) {
						if (layout) {
							if ( layout->layout_forward(layout->layout_mark()) ) {
								// simple delete mark
								layout->_mark_index = -1;
								layout = nullptr;
								_mark_total--;
							}
						}
					}
				}
			}
			if (_mark_total > 0) { // reverse iteration
				for (int i = _marks.length() - 1; i >= 0; i--) {
					auto& levelMarks = _marks[i];
					for (auto& layout: levelMarks) {
						if (layout) {
							if ( layout->layout_reverse(layout->layout_mark()) ) {
								// simple delete mark recursive
								layout->_mark_index = -1;
								layout = nullptr;
								_mark_total--;
							}
						}
					}
				}
			}
			Qk_ASSERT(_mark_total >= 0);
		} while (_mark_total);

		for (auto& levelMarks: _marks) {
			levelMarks.clear();
		}
		_is_render = true;
	}

	/**
	 * Work around flagging views that need to be updated
	 */
	bool Window::preRender() {
		int64_t now_time = time_monotonic();
		// _host->action_direct()->advance(now_time); // advance action

		if ( _tasks.length() ) { // solve task
			auto i = _tasks.begin(), end = _tasks.end();
			while ( i != end ) {
				Task* task = *i;
				if ( task ) {
					if ( now_time > task->task_timeout() ) {
						if ( task->run_task(now_time) ) {
							_is_render = true;
						}
					}
					i++;
				} else {
					_tasks.erase(i++);
				}
			}
		}

		if (_mark_total) { // solve marks
			solveMarks();
		}

		if (_is_render) {
			_is_render = false;
			return true;
		} else {
			return false;
		}
	}

	Window::Task::~RenderTask() {
		if (_win) {
			_win->untask(this);
		}
	}

	void Window::Task::set_task_timeout(int64_t timeout_us) {
		_task_timeout = timeout_us;
	}

}
