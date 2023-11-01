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

#import "./mac_app.h"
#import "../../display.h"

using namespace qk;

// ***************** D i s p l a y *****************

typedef Display::StatusBarStyle StatusBarStyle;

extern QkApplicationDelegate *__appDelegate;

/**
 * @func default_atom_pixel
 */
float Display::default_atom_pixel() {
	return 1.0 / UIScreen.mainScreen.backingScaleFactor;
}

/**
 * @func keep_screen(keep)
 */
void Display::keep_screen(bool keep) {
	// TODO
}

/**
 * @func status_bar_height()
 */
float Display::status_bar_height() {
	return 0;
}

/**
 * @func default_status_bar_height
 */
float Display::default_status_bar_height() {
	return 0;
}

/**
 * @func set_visible_status_bar(visible)
 */
void Display::set_visible_status_bar(bool visible) {
	// TODO
}

/**
 * @func set_status_bar_text_color(color)
 */
void Display::set_status_bar_style(StatusBarStyle style) {
	// TODO
}

/**
 * @func request_fullscreen(fullscreen)
 */
void Display::request_fullscreen(bool fullscreen) {
	// TODO
}

/**
 * @func orientation()
 */
Display::Orientation Display::orientation() {
	return ORIENTATION_INVALID;
}

/**
 * @func set_orientation(orientation)
 */
void Display::set_orientation(Orientation orientation) {
	// noop
}
