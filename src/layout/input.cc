/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, xuewen.chu
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

#include "./input.h"
#include "../app.inl"
#include "../pre_render.h"
#include "../util/codec.h"
#include <math.h>

namespace noug {

	N_DEFINE_INLINE_MEMBERS(Input, Inl) {
	public:

		void touchstart_handle(UIEvent& evt) {
			TouchEvent* e = static_cast<TouchEvent*>(&evt);
			start_handle(Vec2(e->changed_touches()[0].x, e->changed_touches()[0].y));
		}

		void touchmove_handle(UIEvent& evt) {
			TouchEvent* e = static_cast<TouchEvent*>(&evt);
			move_handle(evt, Vec2(e->changed_touches()[0].x, e->changed_touches()[0].y));
		}

		void touchend_handle(UIEvent& evt) {
			TouchEvent* e = static_cast<TouchEvent*>(&evt);
			end_handle(Vec2(e->changed_touches()[0].x, e->changed_touches()[0].y));
		}

		void mousedown_handle(UIEvent& evt) {
			MouseEvent* e = static_cast<MouseEvent*>(&evt);
			start_handle(Vec2(e->x(), e->y()));
		}

		void mousemove_handle(UIEvent& evt) {
			MouseEvent* e = static_cast<MouseEvent*>(&evt);
			move_handle(evt, Vec2(e->x(), e->y()));
		}

		void mouseup_handle(UIEvent& evt) {
			MouseEvent* e = static_cast<MouseEvent*>(&evt);
			end_handle(Vec2(e->x(), e->y()));
		}

		int64_t activate_touchmove_selectd_timeout() {
			if ( _is_multiline ) {
				return 1e6;
			} else {
				return 0;
			}
		}
		
		void click_handle(UIEvent& evt) {
			ClickEvent* e = static_cast<ClickEvent*>(&evt);
			if ( _editing ) {
				_inl_app(pre_render()->host())->ime_keyboard_open({ false, _type, _return_type, input_spot_location() });
			} else {
				if ( _flag == 8 ) { // 禁用点击聚焦
					_flag = 0;
				} else {
					focus();
					set_cursor_with_screen_coord(Vec2(e->x(), e->y()));
				}
			}
		}
		
		int has_can_activate_auto_selectd(Vec2 point) {
			Vec2 pos = position();
			
			int i = 0;

			auto size = content_size();
			
			if (point.x() < pos.x() + 16 || point.x() > pos.x() + size.x() - 16) {
				i++;
			}
			if ( point.y() < pos.y() || point.y() > pos.y() + size.y() ) {
				i++;
			}
			return i;
		}

		void start_handle(Vec2 point) {
			if ( _editing ) {
				_point = point;
				/*
				* _flag
				* 0.未激活
				* 1.等待超时激活光标定位
				* 2.禁用激活光标定位
				* 3.光标定位
				* 4.自动光标定位
				* 5.范围选择
				* 6.自动范围选择
				* 7.开始检测文本移动聚焦
				* 8.禁用文本点击聚焦
				*/
				if ( !_flag ) { // 开始超时激活光标定位
					_flag = 1;
					int64_t timeout = activate_touchmove_selectd_timeout();
					if ( timeout ) {
						pre_render()->host()->loop()->post(Cb([this](CbData& evt) { // delay
							UILock lock;
							if ( _flag == 1 ) {
								_flag = 3; // 激活光标定位
								set_cursor_with_screen_coord(_point);
							}
						}, this), N_MIN(timeout, 1e6/*1s*/));
					} else { // 立即激活
						_flag = 3;
					}
				}
			} else {
				if ( _flag == 0 ) {
					if ( _is_multiline ) { // 多行移动后禁用焦点
						_point = point;
						_flag = 7;      // 开始检测点击聚焦
					}
				}
			}
		}
		
		void move_handle(UIEvent& evt, Vec2 point) {
			if ( _editing ) {
				_point = point;
				
				switch (_flag) {
					default: break;
					case 1:       // 等待激活光标定位
						_flag = 2;  // 禁用
						break;
					case 3: // 光标定位
						if ( has_can_activate_auto_selectd(_point) ) {
							_flag = 4;
						} else {
							set_cursor_with_screen_coord(_point);
						}
						evt.return_value = 0;
						break;
					case 4: {// 自动光标定位
						int i = has_can_activate_auto_selectd(_point);
						if ( !i ) {
							_flag = 3;
						} else {
							if ( i == 1 ) {
								set_cursor_with_screen_coord(_point);
							}
						}
						evt.return_value = 0;
						break;
					}
					case 5: // 范围选择
						break;
					case 6: // 自动范围选择
						break;
				}
			} else {
				if ( _flag == 7 ) { // 已经开始检测
					// TODO
					//Textarea* textarea = as_textarea();
					//if ( textarea ) { // 多行移动后禁用点击聚焦
					/*
					if ( _is_multiline ) { // 多行移动后禁用点击聚焦
						if ( textarea->scroll_x() != 0 || textarea->scroll_y() != 0 ) {
							// 计算移动距离
							float d = sqrtf(powf(point.x() - _point.x(), 2) + powf(point.y() - _point.y(), 2));
							if ( d > 5 ) { // 移动超过5禁用点击聚焦
								_flag = 8;
							}
						}
					}*/
				}
			}
		}
		
		void end_handle(Vec2 point) {
			if ( _editing ) {
				if ( _flag == 1 || _flag == 3 ) {
					set_cursor_with_screen_coord(point);
				}
				_flag = 0;
			}
		}
		
		void focus_handle(UIEvent& evt) {
			_editing = true;
			_cursor_twinkle_status = 0;
			_flag = 0;
			mark(M_INPUT_STATUS);
			register_task();
		}
		
		void blur_handle(UIEvent& evt) {
			_editing = false;
			_flag = 0;
			if ( _marked_text.length() ) {
				input_unmark_text(_marked_text);
			} else {
				mark(M_INPUT_STATUS);
			}
			unregister_task();
		}

		Vec2 spot_location() {
			Vec2 offset = input_text_offset();

			float y = _lines->line(_cursor_linenum).baseline - _text_ascent + offset.y();
			float x = _cursor_x + offset.x();

			auto origin = origin_value();

			Vec2 cursor_offset(x - origin.x(), y + _text_height - origin.y());
			Vec2 location = matrix() * cursor_offset;

			N_DEBUG("input_spot_location,x:%f,y:%f", location.x(), location.y());
			
			return location;
		}
		
		void keydown_handle(UIEvent& evt) { // keyboard event
			if ( _editing && _flag == 0 ) {
				
				switch ( static_cast<KeyEvent*>(&evt)->keycode() ) {
					default: break;
					case KEYCODE_LEFT:
						_cursor = N_MAX(0, int(_cursor - 1));
						break;
					case KEYCODE_UP: {
						Vec2 location = spot_location();
						Vec2 coord(location.x(), location.y() - (_text_height * 1.5));
						set_cursor_with_screen_coord(coord);
						break;
					}
					case KEYCODE_RIGHT:
						_cursor = N_MIN(text_length(), _cursor + 1);
						break;
					case KEYCODE_DOWN: {
						Vec2 location = spot_location();
						Vec2 coord(location.x(), location.y() + (_text_height * 0.5));
						set_cursor_with_screen_coord(coord);
						break;
					}
					case KEYCODE_PAGE_UP: /* TODO */
						break;
					case KEYCODE_PAGE_DOWN: /* TODO */
						break;
					case KEYCODE_MOVE_HOME:
						_cursor = 0;
						break;
					case KEYCODE_MOVE_END:
						_cursor = text_length();
						break;
				}
				
				limit_cursor_in_marked_text();
				reset_cursor_twinkle_task_timeout();
				mark(M_INPUT_STATUS);
			}
		}
		
		void auto_selectd() {

			if ( !_editing || _blob.length() == 0 ) return;
			if ( _flag != 4 ) return; // 非自动光标定位

			Vec2 pos = position();
			Vec2 point = _point;
			Vec2 offset = input_text_offset();
			
			int direction_x = 0, direction_y = 0; // 0 表示没有方向,不做处理
			auto size = content_size();
			
			if ( point.x() < pos.x() + 16 ) {
				direction_x = -1; // left
			} else if ( point.x() > pos.x() + size.x() - 16 ) {
				direction_x = 1; // right
			}
			
			if ( point.y() < pos.y() ) {
				direction_y = -1; // left
			} else if ( point.y() > pos.y() + size.y() ) {
				direction_y = 1; // right
			}

			if ( direction_y ) {
				int linenum = _cursor_linenum;
				linenum += direction_y;
				linenum = N_MIN(_lines->last()->line, N_MAX(linenum, 0));
				point.set_y(pos.y() + _lines->line(linenum).baseline + offset.y());
			}
			
			if ( direction_x ) {
				
				int begin = _blob.length() - 1;
				
				for ( ; begin >=0; begin-- ) {
					if ( _blob[begin].line == _cursor_linenum ) break;
				}

				int cursor = _cursor + (direction_x > 0 ? 1: -1);
				cursor = N_MIN(text_length(), N_MAX(cursor, 0));
				
				for ( int j = begin; j >= 0; j-- ) {
					auto cell = &_blob[j];
					if ( cell->line == _cursor_linenum ) {
						if ( int(cell->index) <= cursor ) {
							float x = cell->origin + cell->offset[N_MIN(cursor - cell->index, cell->glyphs.length())].x();
							x += _lines->line(cell->line).origin;
							point.set_x(pos.x() + offset.x() + x);
							break;
						}
					} else {
						cell = &_blob[j+1];
						float x = cell->origin + cell->offset.front().x();
						x += _lines->line(cell->line).origin;
						point.set_x(pos.x() + offset.x() + x);
						break;
					}
				}
			}
			
			set_cursor_with_screen_coord(point);
		}
		
		void set_cursor_with_screen_coord(Vec2 screen_coord) {
			
			if ( !_editing || text_length() == 0 ) {
				return;
			}

			auto &mat = matrix();
			Vec2 pos = Vec2(mat[2], mat[5]) - origin_value();
			
			// find line
			
			float x = screen_coord.x() - pos.x();
			float y = screen_coord.y() - pos.y();
			Vec2 offset = input_text_offset();
			
			const TextLines::Line* line = nullptr;
			
			if ( y < offset.y() ) {
				line = &_lines->line(0);
			} else if ( y > offset.y() + _lines->max_height() ) {
				line = _lines->last();
			} else {
				for ( int j = 0; j < _lines->length(); j++ ) {
					auto& item = _lines->line(j);
					if (y >= offset.y() + item.start_y &&
							y <= offset.y() + item.end_y ) {
						line = &item; break;
					}
				}
			}
			
			N_ASSERT(line);
			
			// find cell start and end
			int cell_begin = -1, cell_end = -1;
			
			for ( uint32_t i = 0; i < _blob.length(); i++ ) {
				if ( _blob[i].line == line->line ) { // 排除小余目标行cell
					if (cell_begin == -1)
						cell_begin = i;
					cell_end = i;
				} else {
					if (cell_begin != -1)
						break;
				}
			}
			
			if ( cell_begin == -1 || cell_end == -1 ) { // 所在行没有cell,选择最后行尾
				_cursor = text_length();
			} else {
				float offset_start = offset.x() + line->origin;

				if ( x <= offset_start ) { // 行开始位置
					_cursor = _blob[cell_begin].index;
				} else if ( x >= offset_start + line->width ) { // 行结束位置
					_cursor = _blob[cell_end].index + _blob[cell_end].glyphs.length(); // end
				} else {
					// 通过在cells中查询光标位置
					for ( int i = cell_begin; i <= cell_end; i++ ) {
						auto& cell = _blob[i];
						float offset_s = offset_start + cell.origin;
						float offset0 = offset_s + cell.offset.front().x();

						for ( int j = 1, l = cell.offset.length(); j < l; j++ ) {
							float offset = offset_s + cell.offset[j].x();
							
							if ( (offset0 <= x && x <= offset) || (offset <= x && x <= offset0) ) {
								if ( fabs(x - offset0) < fabs(x - offset) ) {
									_cursor = cell.index + j - 1;
								} else {
									_cursor = cell.index + j;
								}
								goto end;
							} else {
								offset0 = offset;
							}
						}
					}
				}
				// if ( x <= offset_start )
			}
		end:
			
			limit_cursor_in_marked_text();
			reset_cursor_twinkle_task_timeout();
			mark(M_INPUT_STATUS);
		}
		
		void limit_cursor_in_marked_text() {
			if ( _marked_text.length() ) { // 限制在marked中
				if ( _cursor < _marked_text_idx ) {
					_cursor = _marked_text_idx;
				} else if ( _cursor > _marked_text_idx + _marked_text.length() ) {
					_cursor = _marked_text_idx + _marked_text.length();
				}
			}
		}
		
		void reset_cursor_twinkle_task_timeout() {
			_cursor_twinkle_status = 1;
			if ( _flag == 4 || _flag == 6 ) {
				set_task_timeout(time_monotonic() + 10000);
			} else {
				set_task_timeout(time_monotonic() + 700000);
			}
		}

		String4 delete_line_feed_format(cString& text) {
			String s = text;
			if ( !is_multiline() ) {
				if ( s.length() > 1 ) {
					s = s.replace_all('\n', String());
				} else if ( s.length() == 1 ) {
					if ( s[0] == '\n' )
						return String4();
				} else {
					return String4();
				}
			}
			return Codec::decode_to_uint32(kUTF8_Encoding, s);
		}

		void input_insert_text(cString4& text) {
			if ( !text.length() ) {
				if ( _cursor < text_length() ) { // insert
					String4 old = _text_value_u4;
					_text_value_u4 = String4(*old, _cursor, *text, text.length());
					_text_value_u4.append(*old + _cursor, old.length() - _cursor);
				} else { // append
					_text_value_u4.append( text );
				}
				_cursor += text.length();
				mark(kLayout_Typesetting); // 标记内容变化
			}
		}
		
		void input_marked_text(cString4& text) {
			if ( _marked_text.length() == 0 ) {
				_marked_text_idx = _cursor;
			}
			String4 old = _text_value_u4;
			_text_value_u4 = String4(*old, _marked_text_idx, *text, text.length());
			_text_value_u4.append(*old + _marked_text_idx + _marked_text.length(),
												old.length() - _marked_text_idx - _marked_text.length());
			
			_cursor += text.length() - _marked_text.length();
			_cursor = N_MAX(_marked_text_idx, _cursor);
			_marked_text = text;
			mark(kLayout_Typesetting); // 标记内容变化
		}
		
		void input_unmark_text(cString4& text) {
			input_marked_text(text);
			_cursor = _marked_text_idx + _marked_text.length();
			_marked_text = String4();
		}
		
		void trigger_change() {
			pre_render()->host()->loop()->post(Cb([this](CbData& e){
				Handle<UIEvent> evt = New<UIEvent>(this);
				trigger(UIEvent_Change, **evt); // trigger event
			}, this));
		}
	};

	Input::Input()
		: _is_multiline(false)
		, _security(false)
		, _text_align(TextAlign::LEFT)
		, _type(KeyboardType::NORMAL)
		, _return_type(KeyboardReturnType::NORMAL)
		, _placeholder_color(150, 150, 150)
		, _text_margin(6)
		, _marked_color(0, 160, 255, 100)
		, _marked_text_idx(0), _cursor(0), _cursor_linenum(0)
		, _marked_cell_begin(0), _marked_cell_end(0)
		, _cursor_x(0), _input_text_offset_x(0)
		, _text_ascent(0), _text_height(0)
		, _editing(false), _cursor_twinkle_status(true), _flag(0)
	{
		set_is_clip(true);
		set_receive(true);
		set_text_word_break(TextWordBreak::BREAK_WORD);
		// bind events
		add_event_listener(UIEvent_Click, &Input::Inl::click_handle, Inl_Input(this));
		add_event_listener(UIEvent_TouchStart, &Input::Inl::touchstart_handle, Inl_Input(this));
		add_event_listener(UIEvent_TouchMove, &Input::Inl::touchmove_handle, Inl_Input(this));
		add_event_listener(UIEvent_TouchEnd, &Input::Inl::touchend_handle, Inl_Input(this));
		add_event_listener(UIEvent_TouchCancel, &Input::Inl::touchend_handle, Inl_Input(this));
		add_event_listener(UIEvent_MouseDown, &Input::Inl::mousedown_handle, Inl_Input(this));
		add_event_listener(UIEvent_MouseMove, &Input::Inl::mousemove_handle, Inl_Input(this));
		add_event_listener(UIEvent_MouseUp, &Input::Inl::mouseup_handle, Inl_Input(this));
		add_event_listener(UIEvent_Focus, &Input::Inl::focus_handle, Inl_Input(this));
		add_event_listener(UIEvent_Blur, &Input::Inl::blur_handle, Inl_Input(this));
		add_event_listener(UIEvent_KeyDown, &Input::Inl::keydown_handle, Inl_Input(this));
	}

	void Input::set_is_multiline(bool val) {
		if (_is_multiline != val) {
			_is_multiline = val;
			mark(kLayout_Typesetting);
		}
	}

	void Input::set_text_align(TextAlign val) {
		if(_text_align != val) {
			_text_align = val;
			mark(kLayout_Typesetting);
		}
	}

	void Input::set_text_value_u4(String4 val) {
		if (_text_value_u4 != val) {
			_text_value_u4 = val;
			mark(kLayout_Typesetting);
		}
	}

	String Input::text_value() const {
		return String(Codec::encode(kUTF8_Encoding, _text_value_u4));
	}

	String Input::placeholder() const {
		return String(Codec::encode(kUTF8_Encoding, _placeholder_u4));
	}

	void Input::set_text_value(String val) {
		set_text_value_u4(String4(Codec::decode_to_uint32(kUTF8_Encoding, val)));
	}

	void Input::set_placeholder(String val) {
		set_placeholder_u4(String4(Codec::decode_to_uint32(kUTF8_Encoding, val)));
	}

	bool Input::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			if (!is_ready_layout_typesetting()) return true; // continue iteration

			Vec2 size = content_size();
			_lines = new TextLines(this, _text_align, size, layout_wrap_x());
			TextConfig cfg(this, pre_render()->host()->default_text_options());

			FontMetrics metrics;
			FontGlyphs::get_metrics(&metrics, text_family().value, font_style(), text_size().value);

			_lines->set_metrics(&metrics, text_line_height().value);
			_text_ascent = -metrics.fAscent;
			_text_height =  metrics.fDescent + metrics.fLeading;

			_blob_visible.clear();
			_blob.clear();

			auto str = _text_value_u4.length() ? &_text_value_u4: &_placeholder_u4;

			if (str->length()) {
				TextBlobBuilder tbb(*_lines, this, &_blob);
				auto lines = string4_to_unichar(*str, false, false, !_is_multiline);
				tbb.set_disable_overflow(true);
				tbb.make(lines);
			}

			_lines->finish();

			Vec2 new_size(
				layout_wrap_x() ? _lines->max_width(): size.x(),
				layout_wrap_y() ? _lines->max_height(): size.y()
			);

			if (new_size != size) {
				set_content_size(new_size);
				parent()->onChildLayoutChange(this, kChild_Layout_Size);
			}

			unmark(kLayout_Typesetting);

			// check transform_origin change
			solve_origin_value();
		}

		return false;
	}

	bool Input::solve_visible_region() {
		if (!Box::solve_visible_region())
			return false;
		_lines->solve_visible_region();
		_lines->solve_visible_region_blob(&_blob, &_blob_visible);
		return true;
	}

	void Input::set_visible(bool val) {
		View::set_visible(val);
		_text_flags = 0xffffffff;
	}

	void Input::set_parent(View *val) {
		View::set_parent(val);
		_text_flags = 0xffffffff;
	}

	bool Input::is_allow_append_child() {
		return false;
	}

	bool Input::can_become_focus() {
		return true;
	}

	TextInput* Input::as_text_input() {
		return this;
	}

	void Input::input_delete(int count) {

		if ( _editing ) {
			int cursor = _cursor;
			if ( !_marked_text.length() ) {
				if ( count < 0 ) {
					count = N_MIN(cursor, -count);
					if ( count ) {
						String4 old = _text_value_u4;
						_text_value_u4 = String4(*old, cursor - count, *old + cursor, int(old.length()) - cursor);
						_cursor -= count;
						mark(kLayout_Typesetting); // 标记内容变化
					}
				} else if ( count > 0 ) {
					count = N_MIN(int(text_length()) - cursor, count);
					if ( count ) {
						String4 old = _text_value_u4;
						_text_value_u4 = String4(*old, cursor,
																	*old + cursor + count,
																	int(old.length()) - cursor - count);
						mark(kLayout_Typesetting); // 标记内容变化
					}
				}
			} else {
				// TODO..
			}
			
			Inl_Input(this)->trigger_change();
			Inl_Input(this)->reset_cursor_twinkle_task_timeout();
		}
		
	}

	void Input::input_insert(cString& text) {
		if ( _editing ) {
			Inl_Input(this)->input_insert_text(Inl_Input(this)->delete_line_feed_format(text));
			Inl_Input(this)->trigger_change();
			Inl_Input(this)->reset_cursor_twinkle_task_timeout();
		}
	}

	void Input::input_marked(cString& text) {
		if ( _editing ) {
			Inl_Input(this)->input_marked_text(Inl_Input(this)->delete_line_feed_format(text));
			Inl_Input(this)->trigger_change();
			Inl_Input(this)->reset_cursor_twinkle_task_timeout();
		}
	}

	void Input::input_unmark(cString& text) {
		if ( _editing ) {
			Inl_Input(this)->input_unmark_text(Inl_Input(this)->delete_line_feed_format(text));
			Inl_Input(this)->trigger_change();
			Inl_Input(this)->reset_cursor_twinkle_task_timeout();
		}
	}

	void Input::input_control(KeyboardKeyName name) {
		if ( _editing && _flag == 0 ) {
			// LOG("input_control,%d", name);
		}
	}

	bool Input::input_can_delete() {
		return _editing && _cursor < text_length();
	}

	bool Input::input_can_backspace() {
		return _editing && _cursor;
	}

	Vec2 Input::input_spot_location() {
		if (_editing) {
			return Inl_Input(this)->spot_location();
		} else {
			return Vec2();
		}
	}

	KeyboardType Input::input_keyboard_type() {
		return _type;
	}

	KeyboardReturnType Input::input_keyboard_return_type() {
		return _return_type;
	}

	void Input::onTextChange(uint32_t value) {
		value ? mark(value): mark_none();
	}

	void Input::set_type(KeyboardType value) {
		if (value != _type) {
			_type = value;
			if ( _editing ) {
				_inl_app(pre_render()->host())->ime_keyboard_open({ false, _type, _return_type, input_spot_location() });
			}
		}
	}

	void Input::set_return_type(KeyboardReturnType value) {
		if (value != _return_type) {
			_return_type = value;
			if ( _editing ) {
				_inl_app(pre_render()->host())->ime_keyboard_open({ false, _type, _return_type, input_spot_location() });
			}
		}
	}

	void Input::set_placeholder_u4(String4 value) {
		_placeholder_u4 = value;
		mark(kLayout_Typesetting);
	}

	void Input::set_placeholder_color(Color value) {
		if (value != _placeholder_color) {
			_placeholder_color = value;
			mark_none(kLayout_None);
		}
	}

	void Input::set_security(bool value) {
		if (_security != value) {
			_security = value;
			mark(kLayout_Typesetting);
		}
	}

	void Input::set_text_margin(float value) {
		if (_text_margin != value) {
			_text_margin = N_MAX(0, value);
			mark(kLayout_Typesetting);
		}
	}

	uint32_t Input::text_length() const {
		return _text_value_u4.length();
	}

	Vec2 Input::input_text_offset() const {
		return Vec2(_input_text_offset_x, 0);
	}

	void Input::set_input_text_offset(Vec2 val) {
		_input_text_offset_x = val.x();
	}

	bool Input::run_task(int64_t sys_time) {
		if ( _flag > 2 ) {
			// direction
			_cursor_twinkle_status = 1;
			if ( _flag == 4 || _flag == 6 ) {
				Inl_Input(this)->auto_selectd();
			}
			set_task_timeout(sys_time + 100000); /* 100ms */
		} else {
			_cursor_twinkle_status = !_cursor_twinkle_status;
			set_task_timeout(sys_time + 700000); /* 700ms */
			return true;
		}
		return false;
	}

	void Input::refresh_cursor_screen_position() {
		
		if ( _editing ) {
			auto size = content_size();
			auto m_final_width = size.x();
			auto m_final_height = size.y();
		
			Vec2  text_offset = input_text_offset();
			TextBlob* cell = nullptr;
			
			if ( text_length() ) {
				int i = 0;
			
				for ( int j = 0; j < _blob.length(); j++ ) {
					auto& i = _blob[j];
					uint32_t index = i.index;
					
					if ( _cursor == index ) {
						cell = &i; break;
					} else if ( _cursor > index ) {
						uint32_t end = index + i.glyphs.length();
						
						if ( _cursor < end ) {
							cell = &i; break;
						} else {
							if ( _cursor == end ) {
								if ( uint32_t(j + 1) == i.glyphs.length() ) { // last cell
									cell = &i; break;
								}
							}
						}
					}
				}
			}
			
			// 计算光标的具体偏移位置与文本编辑状态下最适合的显示的文本偏移量
			
			TextLines::Line* line = nullptr;
			
			if ( cell ) { // set cursor pos
				float offset = cell->offset[_cursor - cell->index].x();
				_cursor_linenum = cell->line;
				line = &_lines->line(_cursor_linenum);
				_cursor_x = line->origin + cell->origin + offset;
			} else { // 找不到cell定位到最后行
				switch ( _text_align ) {
					default:
					// case TextAlign::LEFT_REVERSE:
						_cursor_x = _text_margin; break;
					// case TextAlign::CENTER_REVERSE:
					case TextAlign::CENTER:
						_cursor_x = m_final_width / 2.0; break;
					// case TextAlign::RIGHT_REVERSE:
					case TextAlign::RIGHT:
						_cursor_x = m_final_width - _text_margin; break;
				}
				_cursor_linenum = _lines->last()->line;
				line = &_lines->line(_cursor_linenum);
			}

			float max_width = _lines->max_width();

			// y
			if ( is_multiline() ) {
				if ( _lines->max_height() < m_final_height) {
					text_offset.set_y(0);
				} else {
					float offset = line->start_y + text_offset.y();
					
					if ( offset < 0 ) { // top cursor
						text_offset.set_y(-line->start_y);
					} else { // bottom cursor
						offset = line->end_y + text_offset.y();
						if ( offset > m_final_height ) {
							text_offset.set_y(m_final_height - line->end_y);
						}
					}
					
					if ( text_offset.y() > 0 ) { // top
						text_offset.set_y(0); goto x;
					}
					offset = text_offset.y() + _lines->max_height();
					if ( offset < m_final_height ) { // bottom
						text_offset.set_y(m_final_height - _lines->max_height());
					}
				}
			}
			
		x:
			
			// x
			if ( max_width <= m_final_width - _text_margin - _text_margin ) {
				text_offset.set_x(0);
			} else {
				
				// 让光标x轴始终在可见范围
				
				float offset = _cursor_x + text_offset.x();
				
				if ( offset < _text_margin ) { // left cursor
					text_offset.set_x(_text_margin - _cursor_x);
				} else if ( offset > m_final_width - _text_margin )  { // right cursor
					text_offset.set_x(m_final_width - _text_margin - _cursor_x);
				}
				
				// 检测文本x轴两端是在非法显示区域
				
				switch ( _text_align ) {
					default:
					// case TextAlign::LEFT_REVERSE:
						offset = text_offset.x(); break;
					// case TextAlign::CENTER_REVERSE:
					case TextAlign::CENTER:
						offset = text_offset.x() + (m_final_width - max_width) / 2.0; break;
					// case TextAlign::RIGHT_REVERSE:
					case TextAlign::RIGHT:
						offset = text_offset.x() + m_final_width - max_width; break;
				}

				if ( offset > _text_margin ) { // left
					text_offset.set_x(text_offset.x() - offset + _text_margin); goto end;
				}
				offset += max_width;
				if ( offset < m_final_width - _text_margin ) { // right
					text_offset.set_x(text_offset.x() - offset - _text_margin + m_final_width);
				}
			}
		end:
			
			set_input_text_offset(text_offset);
		} else {
			if ( !is_multiline() ) { // 
				set_input_text_offset(Vec2());
			}
		}
	}

}