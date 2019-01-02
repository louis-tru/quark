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

#include "input.h"
#include "textarea.h"
#include "app-1.h"

XX_NS(ngui)

XX_DEFINE_INLINE_MEMBERS(Input, Inl) {
 public:
	
	/**
	 * @func activate_touchmove_selectd_timeout
	 */
	int64 activate_touchmove_selectd_timeout() {
		if ( is_multi_line_input() ) {
			return 1e6;
		} else {
			return 0;
		}
	}
	
	void click_handle(GUIEvent& evt) {
		GUIClickEvent* e = static_cast<GUIClickEvent*>(&evt);
		if ( editing_ ) {
			_inl_app(app())->ime_keyboard_open({ false, type_, return_type_, input_spot_location() });
		} else {
			if ( flag_ == 8 ) { // 禁用点击聚焦
				flag_ = 0;
			} else {
				focus();
				set_cursor_with_screen_coord(Vec2(e->x(), e->y()));
			}
		}
	}
	
	int has_can_activate_auto_selectd(Vec2 point) {
		Vec2 pos = position();
		
		int i = 0;
		
		if (point.x() < pos.x() + 16 || point.x() > pos.x() + m_final_width - 16) {
			i++;
		}
		if ( point.y() < pos.y() || point.y() > pos.y() + m_final_height ) {
			i++;
		}
		return i;
	}

	void touchstart_handle(GUIEvent& evt) {
		GUITouchEvent* e = static_cast<GUITouchEvent*>(&evt);
		start_handle(Vec2(e->changed_touches()[0].x, e->changed_touches()[0].y));
	}
	
	void touchmove_handle(GUIEvent& evt) {
		GUITouchEvent* e = static_cast<GUITouchEvent*>(&evt);
		move_handle(evt, Vec2(e->changed_touches()[0].x, e->changed_touches()[0].y));
	}
	
	void touchend_handle(GUIEvent& evt) {
		GUITouchEvent* e = static_cast<GUITouchEvent*>(&evt);
		end_handle(Vec2(e->changed_touches()[0].x, e->changed_touches()[0].y));
	}

	void mousedown_handle(GUIEvent& evt) {
		GUIMouseEvent* e = static_cast<GUIMouseEvent*>(&evt);
		start_handle(Vec2(e->x(), e->y()));
	}

	void mousemove_handle(GUIEvent& evt) {
		GUIMouseEvent* e = static_cast<GUIMouseEvent*>(&evt);
		move_handle(evt, Vec2(e->x(), e->y()));
	}

	void mouseup_handle(GUIEvent& evt) {
		GUIMouseEvent* e = static_cast<GUIMouseEvent*>(&evt);
		end_handle(Vec2(e->x(), e->y()));
	}
	
	void start_handle(Vec2 point) {
		if ( editing_ ) {
			point_ = point;
			/*
			 * flag_
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
			if ( !flag_ ) { // 开始超时激活光标定位
				flag_ = 1;
				int64 timeout = activate_touchmove_selectd_timeout();
				if ( timeout ) {
					main_loop()->post(Cb([this](Se& evt) { // delay
						GUILock lock;
						if ( flag_ == 1 ) {
							flag_ = 3; // 激活光标定位
							set_cursor_with_screen_coord(point_);
						}
					}, this), XX_MIN(timeout, 1e6/*1s*/));
				} else { // 立即激活
					flag_ = 3;
				}
			}
		} else {
			if ( flag_ == 0 ) {
				if ( as_textarea() ) { // 多行移动后禁用焦点
					point_ = point;
					flag_ = 7;      // 开始检测点击聚焦
				}
			}
		}
	}
	
	void move_handle(GUIEvent& evt, Vec2 point) {
		if ( editing_ ) {
			point_ = point;
			
			switch (flag_) {
				default: break;
				case 1:       // 等待激活光标定位
					flag_ = 2;  // 禁用
					break;
				case 3: // 光标定位
					if ( has_can_activate_auto_selectd(point_) ) {
						flag_ = 4;
					} else {
						set_cursor_with_screen_coord(point_);
					}
					evt.return_value = 0;
					break;
				case 4: {// 自动光标定位
					int i = has_can_activate_auto_selectd(point_);
					if ( !i ) {
						flag_ = 3;
					} else {
						if ( i == 1 ) {
							set_cursor_with_screen_coord(point_);
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
			if ( flag_ == 7 ) { // 已经开始检测
				Textarea* textarea = as_textarea();
				if ( textarea ) { // 多行移动后禁用点击聚焦
					if ( textarea->scroll_x() != 0 || textarea->scroll_y() != 0 ) {
						// 计算移动距离
						float d = sqrtf(powf(point.x() - point_.x(), 2) + powf(point.y() - point_.y(), 2));
						if ( d > 5 ) { // 移动超过5禁用点击聚焦
							flag_ = 8;
						}
					}
				}
			}
		}
	}
	
	void end_handle(Vec2 point) {
		if ( editing_ ) {
			if ( flag_ == 1 || flag_ == 3 ) {
				set_cursor_with_screen_coord(point);
			}
			flag_ = 0;
		}
	}
	
	void focus_handle(GUIEvent& evt) {
		editing_ = true;
		cursor_twinkle_status_ = 0;
		flag_ = 0;
		mark(M_INPUT_STATUS);
		register_task();
	}
	
	void blur_handle(GUIEvent& evt) {
		editing_ = false;
		flag_ = 0;
		if ( marked_text_.length() ) {
			input_unmark_text(marked_text_);
		} else {
			mark(M_INPUT_STATUS);
		}
		unregister_task();
	}

	Vec2 spot_location() {
		Vec2 offset = input_text_offset();

		float y = m_rows[cursor_linenum_].baseline - m_data.text_hori_bearing + offset.y();
		float x = cursor_x_ + offset.x();

		Vec2 cursor_offset(x - m_origin.x(), y + m_data.text_height - m_origin.y());
		Vec2 location = m_final_matrix * cursor_offset;

		DLOG("input_spot_location,x:%f,y:%f", location.x(), location.y());
		
		return location;
	}
	
	void keydown_handle(GUIEvent& evt) { // keyboard event
		if ( editing_ && flag_ == 0 ) {
			
			switch ( static_cast<GUIKeyEvent*>(&evt)->keycode() ) {
				default: break;
				case KEYCODE_LEFT:
					cursor_ = XX_MAX(0, int(cursor_ - 1));
					break;
				case KEYCODE_UP: {
					Vec2 location = spot_location();
					Vec2 coord(location.x(), location.y() - (m_data.text_height * 1.5));
					set_cursor_with_screen_coord(coord);
					break;
				}
				case KEYCODE_RIGHT:
					cursor_ = XX_MIN(m_data.string.length(), cursor_ + 1);
					break;
				case KEYCODE_DOWN: {
					Vec2 location = spot_location();
					Vec2 coord(location.x(), location.y() + (m_data.text_height * 0.5));
					set_cursor_with_screen_coord(coord);
					break;
				}
				case KEYCODE_PAGE_UP: /* TODO */
					break;
				case KEYCODE_PAGE_DOWN: /* TODO */
					break;
				case KEYCODE_MOVE_HOME:
					cursor_ = 0;
					break;
				case KEYCODE_MOVE_END:
					cursor_ = m_data.string.length();
					break;
			}
			
			limit_cursor_in_marked_text();
			reset_cursor_twinkle_task_timeout();
			mark(M_INPUT_STATUS);
		}
	}
	
	void auto_selectd() {
		
		if ( !editing_ || m_data.cells.length() == 0 ) return;
		
		Vec2 pos = position();
		Vec2 point = point_;
		Vec2 offset = input_text_offset();
		
		int direction_x = 0, direction_y = 0; // 0 表示没有方向,不做处理
		
		if ( point.x() < pos.x() + 16 ) {
			direction_x = -1; // left
		} else if ( point.x() > pos.x() + m_final_width - 16 ) {
			direction_x = 1; // right
		}
		
		if ( point.y() < pos.y() ) {
			direction_y = -1; // left
		} else if ( point.y() > pos.y() + m_final_height ) {
			direction_y = 1; // right
		}
		
		if ( flag_ == 4 ) { // 自动光标定位
			
			if ( direction_y ) {
				int linenum = cursor_linenum_;
				linenum += direction_y;
				linenum = XX_MIN(m_rows.last_num(), XX_MAX(linenum, 0));
				point.y(pos.y() + m_rows[linenum].baseline + offset.y());
			}
			
			if ( direction_x ) {
				
				int begin = m_data.cells.length() - 1;
				
				for ( ; begin >=0; begin-- ) {
					if ( m_data.cells[begin].line_num == cursor_linenum_ ) break;
				}
				
				float reverse = m_data.cells[begin].reverse;
				int cursor = cursor_ + (direction_x > 0 ? (reverse?-1:1): (reverse?1:-1));
				cursor = XX_MIN(m_data.string.length(), XX_MAX(cursor, 0));
				
				for ( int j = begin; j >= 0; j-- ) {
					Cell* cell = &m_data.cells[j];
					if ( cell->line_num == cursor_linenum_ ) {
						if ( int(cell->begin) <= cursor ) {
							float x = cell->offset[XX_MIN(cursor - cell->begin, cell->chars.length())];
							x = cell->offset_start + (reverse ? -x : x);
							point.x(pos.x() + offset.x() + x);
							break;
						}
					} else {
						cell = &m_data.cells[j+1];
						float x = cell->offset[0];
						x = cell->offset_start + (reverse ? -x : x);
						point.x(pos.x() + offset.x() + x);
						break;
					}
				}
			}
			
			set_cursor_with_screen_coord(point);
		}
	}
	
	void set_cursor_with_screen_coord(Vec2 screen_coord) {
		
		if ( !editing_ || m_data.string.length() == 0 ) {
			return;
		}
	
		// Vec2 pos = position() - m_origin; // TODO 这个方法效率低
		Vec2 pos = Vec2(m_final_matrix[2], m_final_matrix[5]) - m_origin;
		
		// find row
		
		float x = screen_coord.x() - pos.x();
		float y = screen_coord.y() - pos.y();
		Vec2 offset = input_text_offset();
		
		const TextRows::Row* row = nullptr;
		
		if ( y < offset.y() ) {
			row = &m_rows[0];
		} else if ( y > offset.y() + m_rows.max_height() ) {
			row = m_rows.last();
		} else {
			for ( auto& i : m_rows.rows() ) {
				if (y >= offset.y() + i.value().offset_start.y() &&
						y <= offset.y() + i.value().offset_end.y() ) {
					row = &i.value(); break;
				}
			}
		}
		
		XX_ASSERT(row);
		
		// find cell start and end
		int cell_begin = -1, cell_end = -1;
		
		for ( uint i = 0; i < m_data.cells.length(); i++ ) {
			if ( m_data.cells[i].line_num == row->row_num  ) { // 排除小余目标行cell
				cell_begin = i;
				
				for ( int i = m_data.cells.length() - 1; i >= cell_begin; i-- ) {
					if ( m_data.cells[i].line_num == row->row_num  ) { // 排除大余目标行cell
						cell_end = i; break;
					}
				}
				break;
			}
		}
		
		if ( cell_begin == -1 || cell_end == -1 ) { // 所在行没有cell,选择最后行尾
			cursor_ = m_data.string.length();
		} else {
			//
			Cell& cell = m_data.cells[cell_begin]; // 开始cell
			Cell& cell2 = m_data.cells[cell_end];  // 结束cell
			
			float offset_start = offset.x() + cell.offset_start;
			uint end = cell2.begin + cell2.chars.length();
			bool reverse = cell.reverse;
			
			if ( x <= offset_start + (reverse ? -row->offset_end.x() : 0) ) { // 行开始位置
				cursor_ = reverse ? end : cell.begin;
			} else if ( x >= offset_start + (reverse ? 0 : row->offset_end.x()) ) { // 行结束位置
				cursor_ = reverse ? cell.begin : end;
			} else {
				// 通过在cells中查询光标位置
				for ( int i = cell_begin; i <= cell_end; i++ ) {
					Cell& cell = m_data.cells[i];
					float offset0 = offset_start + (reverse ? -cell.offset[0] : cell.offset[0]);
					
					for ( int j = 1, l = cell.offset.length(); j < l; j++ ) {
						float offset = offset_start + (reverse ? -cell.offset[j] : cell.offset[j]);
						
						if ( (offset0 <= x && x <= offset) || (offset <= x && x <= offset0) ) {
							if ( fabs(x - offset0) < fabs(x - offset) ) {
								cursor_ = cell.begin + j - 1;
							} else {
								cursor_ = cell.begin + j;
							}
							goto end;
						} else {
							offset0 = offset;
						}
					}
				}
			}
			//
		}
	 end:
		
		limit_cursor_in_marked_text();
		reset_cursor_twinkle_task_timeout();
		mark(M_INPUT_STATUS);
	}
	
	void limit_cursor_in_marked_text() {
		if ( marked_text_.length() ) { // 限制在marked中
			if ( cursor_ < marked_text_idx_ ) {
				cursor_ = marked_text_idx_;
			} else if ( cursor_ > marked_text_idx_ + marked_text_.length() ) {
				cursor_ = marked_text_idx_ + marked_text_.length();
			}
		}
	}
	
	void reset_cursor_twinkle_task_timeout() {
		cursor_twinkle_status_ = 1;
		if ( flag_ == 4 || flag_ == 6 ) {
			set_task_timeout(sys::time_monotonic() + 10000);
		} else {
			set_task_timeout(sys::time_monotonic() + 700000);
		}
	}
	
	Ucs2String delete_line_feed_format(cString& text) {
		String s = text;
		if ( !is_multi_line_input() ) {
			if ( text.length() > 1 ) {
				s = text.replace_all('\n', String());
			} else if ( text.length() == 1 ) {
				if ( text[0] == '\n' ) return Ucs2String();
			} else {
				return Ucs2String();
			}
		}
		return Codec::decoding_to_uint16(Encoding::utf8, s);
	}
	
	void input_insert_text(cUcs2String& text) {
		
		if ( !text.is_empty() ) {
			
			if ( cursor_ < m_data.string.length() ) { // insert
				Ucs2String old = m_data.string;
				m_data.string = Ucs2String(*old, cursor_, *text, text.length());
				m_data.string.push(*old + cursor_, old.length() - cursor_);
			} else { // append
				m_data.string.push( text );
			}
			
			cursor_ += text.length();
			mark_pre( M_CONTENT_OFFSET ); // 标记内容变化
		}
	}
	
	void input_marked_text(cUcs2String& text) {
		if ( marked_text_.length() == 0 ) {
			marked_text_idx_ = cursor_;
		}
		Ucs2String old = m_data.string;
		m_data.string = Ucs2String(*old, marked_text_idx_, *text, text.length());
		m_data.string.push(*old + marked_text_idx_ + marked_text_.length(),
											 old.length() - marked_text_idx_ - marked_text_.length());
		
		cursor_ += text.length() - marked_text_.length();
		cursor_ = XX_MAX(marked_text_idx_, cursor_);
		marked_text_ = text;
		mark_pre( M_CONTENT_OFFSET ); // 标记内容变化
	}
	
	void input_unmark_text(cUcs2String& text) {
		input_marked_text(text);
		cursor_ = marked_text_idx_ + marked_text_.length();
		marked_text_ = Ucs2String();
	}
	
	void trigger_change() {
		main_loop()->post(Cb([this](Se& e){
			Handle<GUIEvent> evt = New<GUIEvent>(this);
			trigger(GUI_EVENT_CHANGE, **evt); // trigger event
		}, this));
	}
};

Input::Input()
: placeholder_color_(150, 150, 150), marked_color_(0, 160, 255, 100)
, marked_text_idx_(0), cursor_(0), cursor_linenum_(0)
, marked_cell_begin_(0), marked_cell_end_(0)
, text_margin_(6), cursor_x_(0)
, editing_(false), cursor_twinkle_status_(true), security_(false)
, flag_(0), type_(KeyboardType::NORMAL)
, return_type_(KeyboardReturnType::NORMAL)
{
	m_receive = true;

	add_event_listener(GUI_EVENT_CLICK, &Input::Inl::click_handle, Inl_Input(this));
	add_event_listener(GUI_EVENT_TOUCH_START, &Input::Inl::touchstart_handle, Inl_Input(this));
	add_event_listener(GUI_EVENT_TOUCH_MOVE, &Input::Inl::touchmove_handle, Inl_Input(this));
	add_event_listener(GUI_EVENT_TOUCH_END, &Input::Inl::touchend_handle, Inl_Input(this));
	add_event_listener(GUI_EVENT_TOUCH_CANCEL, &Input::Inl::touchend_handle, Inl_Input(this));
	add_event_listener(GUI_EVENT_MOUSE_DOWN, &Input::Inl::mousedown_handle, Inl_Input(this));
	add_event_listener(GUI_EVENT_MOUSE_MOVE, &Input::Inl::mousemove_handle, Inl_Input(this));
	add_event_listener(GUI_EVENT_MOUSE_UP, &Input::Inl::mouseup_handle, Inl_Input(this));
	add_event_listener(GUI_EVENT_FOCUS, &Input::Inl::focus_handle, Inl_Input(this));
	add_event_listener(GUI_EVENT_BLUR, &Input::Inl::blur_handle, Inl_Input(this));
	add_event_listener(GUI_EVENT_KEY_DOWN, &Input::Inl::keydown_handle, Inl_Input(this));
}

void Input::set_value(cUcs2String& str) {
	Text::set_value(str);
	marked_text_ = Ucs2String();
	cursor_ = str.length();
	Inl_Input(this)->trigger_change();
}

View* Input::append_text(cUcs2String& str) throw(Error) {
	View* r = Text::append_text(str);
	marked_text_ = Ucs2String();
	cursor_ = m_data.string.length();
	Inl_Input(this)->trigger_change();
	return r;
}

void Input::remove_all_child() {
	Text::remove_all_child();
	marked_text_ = Ucs2String();
	cursor_ = m_data.string.length();
	Inl_Input(this)->trigger_change();
}

bool Input::run_task(int64 sys_time) {
	
	if ( flag_ > 2 ) {
		// direction
		cursor_twinkle_status_ = 1;
		if ( flag_ == 4 || flag_ == 6 ) {
			Inl_Input(this)->auto_selectd();
		}
		set_task_timeout(sys_time + 100000); /* 100ms */
	} else {
		cursor_twinkle_status_ = !cursor_twinkle_status_;
		set_task_timeout(sys_time + 700000); /* 700ms */
		return true;
	}
	return false;
}

void Input::input_delete(int count) {
	
	if ( editing_ ) {
		int cursor = cursor_;
		if ( !marked_text_.length() ) {
			if ( count < 0 ) {
				count = XX_MIN(cursor, -count);
				if ( count ) {
					Ucs2String old = m_data.string;
					m_data.string = Ucs2String(*old, cursor - count,
																		 *old + cursor, int(old.length()) - cursor);
					cursor_ -= count;
					mark_pre( M_CONTENT_OFFSET ); // 标记内容变化
				}
			} else if ( count > 0 ) {
				count = XX_MIN(int(length()) - cursor, count);
				if ( count ) {
					Ucs2String old = m_data.string;
					m_data.string = Ucs2String(*old, cursor,
																		 *old + cursor + count,
																		 int(old.length()) - cursor - count);
					mark_pre( M_CONTENT_OFFSET ); // 标记内容变化
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
	if ( editing_ ) {
		Inl_Input(this)->input_insert_text(Inl_Input(this)->delete_line_feed_format(text));
		Inl_Input(this)->trigger_change();
		Inl_Input(this)->reset_cursor_twinkle_task_timeout();
	}
}

void Input::input_marked(cString& text) {
	if ( editing_ ) {
		Inl_Input(this)->input_marked_text(Inl_Input(this)->delete_line_feed_format(text));
		Inl_Input(this)->trigger_change();
		Inl_Input(this)->reset_cursor_twinkle_task_timeout();
	}
}

void Input::input_unmark(cString& text) {
	if ( editing_ ) {
		Inl_Input(this)->input_unmark_text(Inl_Input(this)->delete_line_feed_format(text));
		Inl_Input(this)->trigger_change();
		Inl_Input(this)->reset_cursor_twinkle_task_timeout();
	}
}

void Input::input_control(KeyboardKeyName name) {
	if ( editing_ && flag_ == 0 ) {
		// LOG("input_control,%d", name);
	}
}

bool Input::can_become_focus() {
	return true;
}

bool Input::input_can_delete() {
	return editing_ && cursor_ < length();
}

bool Input::input_can_backspace() {
	return editing_ && cursor_;
}

Vec2 Input::input_spot_location() {
	if (editing_) {
		return Inl_Input(this)->spot_location();
	} else {
		return Vec2();
	}
}

KeyboardType Input::input_keyboard_type() {
	return type_;
}

KeyboardReturnType Input::input_keyboard_return_type() {
	return return_type_;
}

/**
 * @func set_type
 */
void Input::set_type(KeyboardType value) {
	type_ = value;
	if ( editing_ ) {
		_inl_app(app())->ime_keyboard_open({ false, type_, return_type_, input_spot_location() });
	}
}

/**
 * @func set_return_type
 */
void Input::set_return_type(KeyboardReturnType value) {
	return_type_ = value;
	if ( editing_ ) {
		_inl_app(app())->ime_keyboard_open({ false, type_, return_type_, input_spot_location() });
	}
}

/**
 * @func set_placeholder
 */
void Input::set_placeholder(cUcs2String& value) {
	placeholder_ = value;
	mark_pre(M_CONTENT_OFFSET);
}

/**
 * @func set_placeholder_color
 */
void Input::set_placeholder_color(Color value) {
	placeholder_color_ = value;
	mark(M_NONE);
}

/**
 * @func set_security
 */
void Input::set_security(bool value) {
	if (security_ != value) {
		security_ = value;
		mark_pre(M_CONTENT_OFFSET);
	}
}

/**
 * @func set_text_margin
 */
void Input::set_text_margin(float value) {
	text_margin_ = XX_MAX(0, value);
	mark_pre(M_CONTENT_OFFSET);
}

/**
 * @overwrite
 */
void Input::draw(Draw* draw) {
	if ( m_visible ) {
		
		if ( mark_value ) {
			
			if ( mark_value & (M_CONTENT_OFFSET | M_LAYOUT_THREE_TIMES) ) {
				set_text_align_offset(text_margin_);
			}

			bool change = mark_value & (M_CONTENT_OFFSET | M_INPUT_STATUS);
			if ( change ) {
				refresh_cursor_screen_position(); // text layout
			}

			solve();

			if (change && editing_) {
				_inl_app(app())->ime_keyboard_spot_location(input_spot_location());
			}
			
			if ( mark_value & (M_TRANSFORM | M_TEXT_SIZE) ) {
				set_glyph_texture_level(m_data);
			}
		}
		
		draw->draw(this);
		
		mark_value = M_NONE;
	}
}

void Input::set_layout_content_offset() {
	
	if ( m_final_visible ) {
		
		float margin_x = text_margin_ + text_margin_;
		
		Vec2 limit = Vec2(m_limit.width() - margin_x, m_limit.height());
		
		m_rows.reset();
		m_data.cells.clear(); // 清空旧布局
		m_data.cell_draw_begin = m_data.cell_draw_end = 0;
		
		TextLineHeightValue line_height = m_text_line_height.value;
		
		bool multi_line = is_multi_line_input();
		
		if ( !multi_line && m_explicit_height && m_text_line_height.value.is_auto() ) {
			line_height.height = m_final_height;
		}
		
		cUcs2String& string = m_data.string.length() ? m_data.string: placeholder_;
		
		if ( string.length() ) {
			
			mark( M_SHAPE );
			
			Options opts = get_options();
			
			opts.space_wrap.auto_wrap = opts.space_wrap.auto_wrap && multi_line;
			opts.space_wrap.merge_space = false;
			opts.space_wrap.merge_line_feed = !multi_line;
			opts.overflow = length() ? TextOverflowEnum::NORMAL: TextOverflowEnum::ELLIPSIS;
			opts.text_line_height = line_height;
			
			// set layout ...
			
			if ( length() && marked_text_.length() && !security_ ) {
				
				uint mark = marked_text_idx_;
				uint mark_end = mark + marked_text_.length();
				
				if ( mark ) {
					set_text_layout_offset(&m_rows, limit, m_data, string, 0, mark, &opts, !multi_line);
				}
				
				marked_cell_begin_ = m_data.cells.length();
				set_text_layout_offset(&m_rows, limit, m_data, string, mark, mark_end, &opts, !multi_line);
				marked_cell_end_ = m_data.cells.length();
				
				uint end = string.length();
				if ( mark_end < end ) {
					set_text_layout_offset(&m_rows, limit, m_data, string, mark_end, end, &opts, !multi_line);
				}
				
			} else {
				marked_cell_begin_ = marked_cell_end_ = 0;
				if ( length() && security_ ) {
					set_text_layout_offset(&m_rows, limit, m_data, 9679/*●*/, length(), &opts);
				} else {
					set_text_layout_offset(&m_rows, limit, m_data,
																 string, 0, string.length(), &opts, !multi_line);
				}
			}
		} else {
			get_font_glyph_table_and_height(m_data, line_height);
			m_rows.update_row(m_data.text_ascender, m_data.text_descender);
		}
		
		m_rows.set_width(m_rows.last()->offset_end.x());
		m_rows.set_width(m_rows.max_width() + margin_x);
		
		set_layout_content_offset_after();
	}
}

/**
 * @func is_multi_line_input
 */
bool Input::is_multi_line_input() {
	return false;
}

/**
 * @func input_text_offset
 */
Vec2 Input::input_text_offset() {
	return Vec2(input_text_offset_x_, 0);
}

/**
 * @func set_input_text_offset
 */
void Input::set_input_text_offset(Vec2 value) {
	input_text_offset_x_ = value.x();
}

/**
 * @func refresh_cursor_screen_position
 */
void Input::refresh_cursor_screen_position() {
	
	if ( editing_ ) {
	
		Vec2  text_offset = input_text_offset();
		Cell* cell = nullptr;
		
		if ( m_data.string.length() ) {
		
			for ( auto& i : m_data.cells ) {
				uint begin = i.value().begin;
				
				if ( cursor_ == begin ) {
					cell = &i.value(); break;
				} else if ( cursor_ > begin ) {
					uint end = begin + i.value().chars.length();
					
					if ( cursor_ < end ) {
						cell = &i.value(); break;
					} else {
						if ( cursor_ == end ) {
							if ( uint(i.index() + 1) == m_data.cells.length() ) { // last cell
								cell = &i.value(); break;
							}
						}
					}
				}
			}
		}
		
		// 计算光标的具体偏移位置与文本编辑状态下最适合的显示的文本偏移量
		
		TextRows::Row* row = nullptr;
		
		if ( cell ) { // set cursor pos
			float offset = cell->offset[cursor_ - cell->begin];
			cursor_linenum_ = cell->line_num;
			cursor_x_ = cell->offset_start + (cell->reverse ? -offset : offset);
			row = &m_rows[cursor_linenum_];
		} else { // 找不到cell定位到最后行
			switch ( m_text_align ) {
				default:
				case TextAlign::LEFT_REVERSE:
					cursor_x_ = text_margin_; break;
				case TextAlign::CENTER_REVERSE:
				case TextAlign::CENTER:
					cursor_x_ = m_final_width / 2.0; break;
				case TextAlign::RIGHT_REVERSE:
				case TextAlign::RIGHT:
					cursor_x_ = m_final_width - text_margin_; break;
			}
			cursor_linenum_ = m_rows.last_num();
			row = &m_rows[cursor_linenum_];
		}
		
		float max_width = m_rows.max_width();
		
		// y
		if ( is_multi_line_input() ) {
			if ( m_rows.max_height() < m_final_height) {
				text_offset.y(0);
			} else {
				float offset = row->offset_start.y() + text_offset.y();
				
				if ( offset < 0 ) { // top cursor
					text_offset.y(-row->offset_start.y());
				} else { // bottom cursor
					offset = row->offset_end.y() + text_offset.y();
					if ( offset > m_final_height ) {
						text_offset.y(m_final_height - row->offset_end.y());
					}
				}
				
				if ( text_offset.y() > 0 ) { // top
					text_offset.y(0); goto x;
				}
				offset = text_offset.y() + m_rows.max_height();
				if ( offset < m_final_height ) { // bottom
					text_offset.y(m_final_height - m_rows.max_height());
				}
			}
		}
		
	 x:
		
		// x
		if ( max_width <= m_final_width - text_margin_ - text_margin_ ) {
			text_offset.x(0);
		} else {
			
			// 让光标x轴始终在可见范围
			
			float offset = cursor_x_ + text_offset.x();
			
			if ( offset < text_margin_ ) { // left cursor
				text_offset.x(text_margin_ - cursor_x_);
			} else if ( offset > m_final_width - text_margin_ )  { // right cursor
				text_offset.x(m_final_width - text_margin_ - cursor_x_);
			}
			
			// 检测文本x轴两端是在非法显示区域
			
			switch ( m_text_align ) {
				default:
				case TextAlign::LEFT_REVERSE:
					offset = text_offset.x(); break;
				case TextAlign::CENTER_REVERSE:
				case TextAlign::CENTER:
					offset = text_offset.x() + (m_final_width - max_width) / 2.0; break;
				case TextAlign::RIGHT_REVERSE:
				case TextAlign::RIGHT:
					offset = text_offset.x() + m_final_width - max_width; break;
			}
			
			if ( offset > text_margin_ ) { // left
				text_offset.x(text_offset.x() - offset + text_margin_); goto end;
			}
			offset += max_width;
			if ( offset < m_final_width - text_margin_ ) { // right
				text_offset.x(text_offset.x() - offset - text_margin_ + m_final_width);
			}
		}
	 end:
		
		set_input_text_offset(text_offset);
	} else {
		if ( !is_multi_line_input() ) { // 
			set_input_text_offset(Vec2());
		}
	}
}

XX_END
