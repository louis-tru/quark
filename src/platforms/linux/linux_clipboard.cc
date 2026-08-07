/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include <X11/Xatom.h>
#include "./linux_clipboard.h"
#include "../../ui/clipboard.h"

namespace qk {

	LinuxClipboard::LinuxClipboard(XDisplay* display, XWindow window)
		: _display(display)
		, _window(window)
		, _selection(XInternAtom(display, "CLIPBOARD", False))
		, _targets(XInternAtom(display, "TARGETS", False))
		, _utf8(XInternAtom(display, "UTF8_STRING", False))
		, _property(XInternAtom(display, "QK_CLIPBOARD", False))
		, _owned(false) {
	}

	bool LinuxClipboard::handleClipboardEvent(XEvent& event) {
		if (event.type == SelectionNotify &&
			event.xselection.requestor == _window &&
			event.xselection.selection == _selection &&
			event.xselection.target == _utf8) {
			ScopeLock lock(_mutex);
			// Ignore an old response after setText()/clear() has taken ownership.
			if (_owned)
				return true;
			if (event.xselection.property) {
				Atom type = 0;
				int format = 0;
				unsigned long count = 0, remaining = 0;
				unsigned char* value = nullptr;
				auto status = XGetWindowProperty(_display, _window,
					event.xselection.property, 0, 0x100000, True, AnyPropertyType,
					&type, &format, &count, &remaining, &value);
				if (status == Success && format == 8 && !remaining && type == _utf8) {
					_text = String(reinterpret_cast<char*>(value), uint32_t(count));
					_cond.notify_all();
				}
				if (value)
					XFree(value);
			}
			return true;
		}
		if (event.type == SelectionClear &&
			event.xselectionclear.selection == _selection) {
			ScopeLock lock(_mutex);
			_owned = XGetSelectionOwner(_display, _selection) == _window;
			if (!_owned)
				_text = String();
			return true;
		}
		if (event.type != SelectionRequest ||
			event.xselectionrequest.selection != _selection)
			return false;

		ScopeLock lock(_mutex);
		auto &request = event.xselectionrequest;
		Atom property = request.property ? request.property: request.target;
		bool supported = false;

		if (request.target == _targets) {
			Atom targets[] = {_targets, _utf8};
			XChangeProperty(_display, request.requestor, property,
				XA_ATOM, 32, PropModeReplace,
				reinterpret_cast<unsigned char*>(targets),
				sizeof(targets) / sizeof(targets[0]));
			supported = true;
		} else if (request.target == _utf8) {
			auto maxRequest = XExtendedMaxRequestSize(_display);
			if (!maxRequest)
				maxRequest = XMaxRequestSize(_display);
			if (maxRequest > 64 &&
				_text.length() <= uint32_t((maxRequest - 64) * 4)) {
				XChangeProperty(_display, request.requestor, property,
					_utf8, 8, PropModeReplace,
					reinterpret_cast<const unsigned char*>(_text.c_str()),
					_text.length());
				supported = true;
			}
		}

		XEvent notify{};
		notify.xselection.type = SelectionNotify;
		notify.xselection.display = request.display;
		notify.xselection.requestor = request.requestor;
		notify.xselection.selection = request.selection;
		notify.xselection.target = request.target;
		notify.xselection.property = supported ? property: 0;
		notify.xselection.time = request.time;
		XSendEvent(_display, request.requestor, False, NoEventMask, &notify);
		XFlush(_display);
		return true;
	}

	String LinuxClipboard::getText() {
		if (_owned)
			return _text;
		Lock lock(_mutex);
		if (_owned)
			return _text;
		_text = String();
		XDeleteProperty(_display, _window, _property);
		XConvertSelection(_display, _selection, _utf8, _property, _window, CurrentTime);
		XFlush(_display);
		if (_cond.wait_for(lock, std::chrono::seconds(1)) != std::cv_status::timeout) {
			XSetSelectionOwner(_display, _selection, _window, CurrentTime);
			XFlush(_display);
			_owned = true;
		}
		return _text;
	}

	void LinuxClipboard::setText(cString& text) {
		ScopeLock lock(_mutex);
		_text = text;
		XSetSelectionOwner(_display, _selection, _window, CurrentTime);
		XFlush(_display);
		_owned = true;
		_cond.notify_all();
	}

	bool LinuxClipboard::hasText() {
		return !getText().isEmpty();
	}

	void LinuxClipboard::clear() {
		setText(String());
	}

	String Clipboard::get_text() {
		return linux_clipboard()->getText();
	}

	void Clipboard::set_text(cString& text) {
		linux_clipboard()->setText(text);
	}

	bool Clipboard::has_text() {
		return linux_clipboard()->hasText();
	}

	void Clipboard::clear() {
		linux_clipboard()->clear();
	}
}
