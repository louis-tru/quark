/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, blue.chu
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

#include "./input.h"
#include "../app.h"
#include "../pre_render.h"
#include "../util/codec.h"
#include "./textarea.h"
#include <math.h>

namespace qk {

	enum {
		kFlag_Normal = 0,      // 正常状态未激活光标查询
		kFlag_Wait_Find,       // 等待超时激活光标定位
		kFlag_Disable_Find,    // 禁用激活光标定位
		kFlag_Find,            // 激活光标定位
		kFlag_Auto_Find,       // 激活自动光标定位
		kFlag_Range_Select,    // 范围选择
		kFlag_Auto_Range_Select,  // 自动范围选择
		kFlag_Check_Move_Focus,   // 检测文本移动聚焦
		kFlag_Disable_Click_Find, // 禁用点击聚焦
	};

	Qk_DEFINE_INLINE_MEMBERS(Input, Inl) {
	public:

		void touchstart_handle(UIEvent& evt) {
			TouchEvent* e = static_cast<TouchEvent*>(&evt);
			start_action(Vec2(e->changed_touches()[0].x, e->changed_touches()[0].y));
		}

		void touchmove_handle(UIEvent& evt) {
			TouchEvent* e = static_cast<TouchEvent*>(&evt);
			move_action(evt, Vec2(e->changed_touches()[0].x, e->changed_touches()[0].y));
		}

		void touchend_handle(UIEvent& evt) {
			TouchEvent* e = static_cast<TouchEvent*>(&evt);
			end_action(Vec2(e->changed_touches()[0].x, e->changed_touches()[0].y));
		}

		void mousedown_handle(UIEvent& evt) {
			MouseEvent* e = static_cast<MouseEvent*>(&evt);
			start_action(Vec2(e->x(), e->y()));
		}

		void mousemove_handle(UIEvent& evt) {
			MouseEvent* e = static_cast<MouseEvent*>(&evt);
			move_action(evt, Vec2(e->x(), e->y()));
		}

		void mouseup_handle(UIEvent& evt) {
			MouseEvent* e = static_cast<MouseEvent*>(&evt);
			end_action(Vec2(e->x(), e->y()));
		}

		void click_handle(UIEvent& evt) {
			ClickEvent* e = static_cast<ClickEvent*>(&evt);
			if ( _editing ) {
				pre_render()->host()->dispatch()->
					set_ime_keyboard_open({ false, _type, _return_type, input_spot_location() });
			} else {
				if ( _flag == kFlag_Disable_Click_Find ) { // 禁用点击聚焦
					_flag = kFlag_Normal;
				} else {
					focus();
					find_cursor(Vec2(e->x(), e->y()));
				}
			}
		}

		void keydown_handle(UIEvent& evt) { // keyboard event
			if ( _editing && _flag == kFlag_Normal ) {
				
				switch ( static_cast<KeyEvent*>(&evt)->keycode() ) {
					default: break;
					case KEYCODE_LEFT:
						_cursor = Qk_MAX(0, int(_cursor - 1));
						break;
					case KEYCODE_UP: {
						Vec2 location = spot_location();
						Vec2 coord(location.x(), location.y() - (_text_height * 1.5));
						find_cursor(coord);
						break;
					}
					case KEYCODE_RIGHT:
						_cursor = Qk_MIN(text_length(), _cursor + 1);
						break;
					case KEYCODE_DOWN: {
						Vec2 location = spot_location();
						Vec2 coord(location.x(), location.y() + (_text_height * 0.5));
						find_cursor(coord);
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
				mark_none(kInput_Status);
			}
		}

		void focus_handle(UIEvent& evt) {
			if (_readonly)
				_editing = false;
			else
				_editing = true;
			_cursor_twinkle_status = 0;
			_flag = kFlag_Normal;
			mark_none(kInput_Status);
			pre_render()->addtask(this);
		}

		void blur_handle(UIEvent& evt) {
			_editing = false;
			_flag = kFlag_Normal;
			if ( _marked_text.length() ) {
				input_unmark_text(_marked_text);
			} else {
				mark_none(kInput_Status);
			}
			pre_render()->untask(this);
		}

		Vec2 get_position() {
			Vec2 point(
				padding_left() - origin_value()[0],
				padding_top() - origin_value()[1]
			);
			if (_border) {
				point[0] += _border->width[3]; // left
				point[1] += _border->width[0]; // top
			}
			return matrix() * point;
		}

		void start_action(Vec2 point) {
			if ( _editing ) {
				_point = point;
				if ( _flag == kFlag_Normal ) { // 开始超时激活光标定位
					_flag = kFlag_Wait_Find;
					int64_t timeout = is_multiline() ? 1e6/*1s*/: 0;
					if ( timeout ) {
						pre_render()->host()->loop()->post(Cb([this](Cb::Data& evt) { // delay
							UILock lock;
							if ( _flag == kFlag_Wait_Find ) {
								_flag = kFlag_Find; // 激活光标定位
								find_cursor(_point);
							}
						}, this), Qk_MIN(timeout, 1e6/*1s*/));
					} else { // 立即激活
						_flag = kFlag_Find;
					}
				}
			} else {
				if ( _flag == kFlag_Normal ) {
					if ( is_multiline() ) { // 多行移动后禁用焦点
						_point = point;
						_flag = kFlag_Check_Move_Focus;  // 开始检测点击聚焦
					}
				}
			}
		}

		void move_action(UIEvent& evt, Vec2 point) {
			if ( _editing ) {
				_point = point;
				
				switch (_flag) {
					case kFlag_Wait_Find:          // 等待激活光标定位
						_flag = kFlag_Disable_Find;  // 禁用
						break;
					case kFlag_Find: { // 光标定位
						auto has = is_auto_find_is_required(_point);
						if ( has.x() || has.y() ) {
							_flag = kFlag_Auto_Find;
						} else {
							find_cursor(_point); // 立即查找位置
						}
						evt.return_value = 0;
						break;
					}
					case kFlag_Auto_Find: { // 自动光标定位
						auto has = is_auto_find_is_required(_point);
						if ( has.x() || has.y() ) {
							if ( (has.x() & has.y()) == 0 ) {
								find_cursor(_point);
							}
						} else { // 不需要使用自动选择
							_flag = kFlag_Find;
						}
						evt.return_value = 0;
						break;
					}
					case kFlag_Range_Select: // 范围选择
						break;
					case kFlag_Auto_Range_Select: // 自动范围选择
						break;
					default: break;
				}
			} else {
				if ( _flag == kFlag_Check_Move_Focus ) { // 已经开始检测
					if ( is_multiline() ) { // 多行移动后禁用点击聚焦
						auto textarea = static_cast<Textarea*>(static_cast<Input*>(this));
						if ( textarea->scroll_x() != 0 || textarea->scroll_y() != 0 ) {
							// 计算移动距离
							float d = sqrtf(powf(point.x() - _point.x(), 2) + powf(point.y() - _point.y(), 2));
							if ( d > 5 ) { // 移动超过5禁用点击聚焦
								_flag = kFlag_Disable_Click_Find;
							}
						}
					} // if ( is_multiline() )
				}
			}
		}

		void end_action(Vec2 point) {
			if ( _editing ) {
				if ( _flag == kFlag_Wait_Find || _flag == kFlag_Find ) {
					find_cursor(point);
				}
				_flag = kFlag_Normal;
			}
		}

		Vec2 spot_location() {
			auto offset = input_text_offset();
			auto y = _lines->line(_cursor_linenum).baseline - _text_ascent + _text_height + offset.y();
			auto x = _cursor_x + offset.x();
			auto origin = origin_value();

			x += padding_left();
			y += padding_top();

			if (_border) {
				x += _border->width[3]; // left
				y += _border->width[0]; // top
			}

			Vec2 cursor_offset(x - origin.x(), y - origin.y());
			Vec2 location = matrix() * cursor_offset;

			// Qk_DEBUG("input_spot_location,x:%f,y:%f", location.x(), location.y());
			
			return location;
		}

		iVec2 is_auto_find_is_required(Vec2 point) {
			auto pos = get_position();
			auto size = content_size();

			int x = 0, y = 0; // 0 表示没有方向,不需要自动查找
			int edge_x1 = 10 - padding_left();
			int edge_x2 = 10 - padding_right();
			int edge_y1 = size.y() > 20 ? 10 - padding_top(): 0;
			int edge_y2 = size.y() > 20 ? 10 - padding_bottom(): 0;

			if ( point.x() < pos.x() + edge_x1 ) {
				x = -1; // left
			} else if ( point.x() > pos.x() + size.x() - edge_x2 ) {
				x = 1; // right
			}

			if ( point.y() < pos.y() + edge_y1 ) {
				y = -1; // top
			} else if ( point.y() > pos.y() + size.y() - edge_y2 ) {
				y = 1; // bottom
			}

			return iVec2(x, y);
		}

		void auto_selectd() {
			if ( !_editing || _blob.length() == 0 ) return;

			auto pos = get_position();
			auto point = _point;
			auto offset = input_text_offset();
			auto dir = is_auto_find_is_required(point);

			if (_flag == kFlag_Auto_Find) { // 自动光标定位

				if ( dir.y() ) {
					int linenum = _cursor_linenum;
					linenum += dir.y();
					linenum = Qk_MIN(_lines->last()->line, Qk_MAX(linenum, 0));
					point.set_y(pos.y() + _lines->line(linenum).baseline + offset.y());
				}

				if ( dir.x() ) {
					int begin = _blob.length() - 1;

					for ( ; begin >= 0; begin-- ) {
						if ( _blob[begin].line == _cursor_linenum ) break;
					}

					int cursor = _cursor + (dir.x() > 0 ? 1: -1);
					cursor = Qk_MIN(text_length(), Qk_MAX(cursor, 0));
					
					for ( int j = begin; j >= 0; j-- ) {
						auto cell = &_blob[j];
						if ( cell->line == _cursor_linenum ) {
							if ( int(cell->index) <= cursor ) {
								float x = cell->origin + cell->core.offset[Qk_MIN(cursor - cell->index, cell->core.glyphs.length())].x();
								x += _lines->line(cell->line).origin;
								point.set_x(pos.x() + offset.x() + x);
								break;
							}
						} else {
							cell = &_blob[j+1];
							float x = cell->origin + cell->core.offset.front().x();
							x += _lines->line(cell->line).origin;
							point.set_x(pos.x() + offset.x() + x);
							break;
						}
					}
				}
				
				find_cursor(point);
			} // if (_flag == kFlag_Auto_Find)
		}

		void find_cursor(Vec2 screen_coord) {

			if ( !_editing || text_length() == 0 ) {
				return;
			}

			auto pos = get_position();

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
						line = &item;
						break;
					}
				}
			}
			
			Qk_ASSERT(line);
			
			// find cell start_action and end_action
			int cell_begin = -1, cell_end = -1;
			
			for ( uint32_t i = 0; i < _blob.length(); i++ ) {
				auto &it = _blob[i];
				if ( it.line == line->line ) { // 排除小余目标行cell
					if (cell_begin == -1)
						cell_begin = i;
					cell_end = i;
				} else {
					if (cell_begin != -1)
						break;
					//else if (it.line > line->line) {
						// line_feed
					//	auto idx = Int32::max(0, int32_t(it.index) - 1);
					//	_cursor = idx;
					//	goto end_action;
					//}
				}
			}
			
			if ( cell_begin == -1 || cell_end == -1 ) { // 所在行没有cell,选择最后行尾
				_cursor = text_length();
			} else {
				float offset_start = offset.x() + line->origin;

				if ( x <= offset_start ) { // 行开始位置
					_cursor = _blob[cell_begin].index;
				} else if ( x >= offset_start + line->width ) { // 行结束位置
					_cursor = _blob[cell_end].index + _blob[cell_end].core.glyphs.length(); // end_action
				} else {
					// 通过在cells中查询光标位置
					for ( int i = cell_begin; i <= cell_end; i++ ) {
						auto& cell = _blob[i];
						float offset_s = offset_start + cell.origin;
						float offset0 = offset_s + cell.core.offset.front().x();

						for ( int j = 1, l = cell.core.offset.length(); j < l; j++ ) {
							float offset = offset_s + cell.core.offset[j].x();
							
							if ( (offset0 <= x && x <= offset) || (offset <= x && x <= offset0) ) {
								if ( fabs(x - offset0) < fabs(x - offset) ) {
									_cursor = cell.index + j - 1;
								} else {
									_cursor = cell.index + j;
								}
								goto end_action;
							} else {
								offset0 = offset;
							}
						}
					}
				}
				// if ( x <= offset_start )
			}
		end_action:
			
			limit_cursor_in_marked_text();
			reset_cursor_twinkle_task_timeout();
			mark_none(kInput_Status);
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
			if ( _flag == kFlag_Auto_Find || _flag == kFlag_Auto_Range_Select ) {
				set_task_timeout(time_monotonic() + 10000); // 10ms
			} else {
				set_task_timeout(time_monotonic() + 700000); // 700ms
			}
		}

		// input text insert action

		String4 delete_line_feed_format(cString& text) {
			String s = text;
			if ( !is_multiline() ) {
				if ( s.length() > 1 ) {
					s = s.replaceAll('\n', String());
				} else if ( s.length() == 1 ) {
					if ( s[0] == '\n' )
						return String4();
				} else {
					return String4();
				}
			}
			return codec_decode_to_uint32(kUTF8_Encoding, s);
		}

		void input_insert_text(cString4& text) {
			if ( text.length() ) {
				if (_max_length && _text_value_u4.length() + text.length() > _max_length)
					return;

				if ( _cursor < text_length() ) { // insertd
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

		bool input_marked_text(cString4& text) {
			if (_max_length) {
				if ( _text_value_u4.length() + text.length() - _marked_text.length() > _max_length)
					return false;
			}
			if ( _marked_text.length() == 0 ) {
				_marked_text_idx = _cursor;
			}
			String4 old = _text_value_u4;
			_text_value_u4 = String4(*old, _marked_text_idx, *text, text.length());
			_text_value_u4.append(*old + _marked_text_idx + _marked_text.length(),
												old.length() - _marked_text_idx - _marked_text.length());
			
			_cursor += text.length() - _marked_text.length();
			_cursor = Qk_MAX(_marked_text_idx, _cursor);
			_marked_text = text;
			mark(kLayout_Typesetting); // 标记内容变化

			return true;
		}

		void input_unmark_text(cString4& text) {
			if (input_marked_text(text)) {
				_cursor = _marked_text_idx + _marked_text.length();
				_marked_text = String4();
			}
		}

		void trigger_change() {
			pre_render()->host()->loop()->post(Cb([this](Cb::Data& e){
				Handle<UIEvent> evt = qk::New<UIEvent>(this);
				trigger(UIEvent_Change, **evt); // trigger event
			}, this));
		}

	};

	Input::Input(App *host)
		: Box(host)
		, _security(false), _readonly(false)
		, _text_align(TextAlign::kLeft)
		, _type(KeyboardType::kNormal)
		, _return_type(KeyboardReturnType::kNormal)
		, _placeholder_color(150, 150, 150)
		, _cursor_color(0x43,0x95,0xff) //#4395ff
		, _max_length(0)
		, _marked_color(0, 160, 255, 100)
		, _marked_text_idx(0), _cursor(0), _cursor_linenum(0)
		, _marked_blob_begin(0), _marked_blob_end(0)
		, _cursor_x(0), _input_text_offset_x(0), _input_text_offset_y(0)
		, _text_ascent(0), _text_height(0)
		, _editing(false), _cursor_twinkle_status(true), _flag(kFlag_Normal)
	{
		set_is_clip(true);
		set_receive(true);
		set_text_word_break(TextWordBreak::kBreakWord);
		// bind events
		add_event_listener(UIEvent_Click, &Inl::click_handle, Inl_Input(this));
		add_event_listener(UIEvent_TouchStart, &Inl::touchstart_handle, Inl_Input(this));
		add_event_listener(UIEvent_TouchMove, &Inl::touchmove_handle, Inl_Input(this));
		add_event_listener(UIEvent_TouchEnd, &Inl::touchend_handle, Inl_Input(this));
		add_event_listener(UIEvent_TouchCancel, &Inl::touchend_handle, Inl_Input(this));
		add_event_listener(UIEvent_MouseDown, &Inl::mousedown_handle, Inl_Input(this));
		add_event_listener(UIEvent_MouseMove, &Inl::mousemove_handle, Inl_Input(this));
		add_event_listener(UIEvent_MouseUp, &Inl::mouseup_handle, Inl_Input(this));
		add_event_listener(UIEvent_Focus, &Inl::focus_handle, Inl_Input(this));
		add_event_listener(UIEvent_Blur, &Inl::blur_handle, Inl_Input(this));
		add_event_listener(UIEvent_KeyDown, &Inl::keydown_handle, Inl_Input(this));
	}

	bool Input::is_multiline() {
		return false;
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
			set_max_length(_max_length);
		}
	}

	String Input::text_value() const {
		return String(codec_encode(kUTF8_Encoding, _text_value_u4));
	}

	String Input::placeholder() const {
		return String(codec_encode(kUTF8_Encoding, _placeholder_u4));
	}

	void Input::set_text_value(String val) {
		set_text_value_u4(String4(codec_decode_to_uint32(kUTF8_Encoding, val)));
	}

	void Input::set_placeholder(String val) {
		set_placeholder_u4(String4(codec_decode_to_uint32(kUTF8_Encoding, val)));
	}

	bool Input::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			if (!is_ready_layout_typesetting())
				return true; // continue iteration
			layout_typesetting_input_text();
		}
		return false;
	}

	Vec2 Input::layout_typesetting_input_text() {

		Vec2 size = content_size();
		_lines = new TextLines(this, _text_align, size, layout_wrap_x());
		TextConfig cfg(this, pre_render()->host()->default_text_options());

		FontMetricsBase metrics;
		text_family().value->match(font_style())->getMetrics(&metrics, text_size().value);

		_lines->set_metrics(&metrics, text_line_height().value);
		_text_ascent = -metrics.fAscent;
		_text_height =  _text_ascent + metrics.fDescent + metrics.fLeading;
		_marked_blob_begin = _marked_blob_end = 0;

		_blob_visible.clear();
		_blob.clear();

		auto str = _text_value_u4.length() ? &_text_value_u4: &_placeholder_u4;

		if (str->length()) { // text layout
			TextBlobBuilder tbb(*_lines, this, &_blob);

			if (!is_multiline()) {
				tbb.set_disable_auto_wrap(true);
			}
			tbb.set_disable_overflow(true);

			if (_text_value_u4.length() && !_security && _marked_text.length()) { // marked text layout
				auto src = *_text_value_u4;
				auto mark = _marked_text_idx;
				auto mark_end = mark + _marked_text.length();

				Array<TextBlob> blobTmp;

				auto make = [&](const Unichar *src, uint32_t len) {
					tbb.make(string4_to_unichar(src, len, false, false, !is_multiline()));
					if (blobTmp.length()) blobTmp.concat(std::move(_blob));
					else blobTmp = std::move(_blob);
				};

				if ( mark ) {
					make(src, mark);
				}
				_marked_blob_begin = blobTmp.length();
				make(src+mark, _marked_text.length());
				_marked_blob_end = blobTmp.length();

				if ( mark_end < _text_value_u4.length() ) {
					make(src+mark_end, _text_value_u4.length()-mark_end);
				}
				_blob = std::move(blobTmp);
			}
			else if ( _text_value_u4.length() && _security ) { // password
				Unichar pwd = 9679; /*●*/
				Array<Array<Unichar>> lines(1);
				lines.front().extend(_text_value_u4.length());
				memset_pattern4(*lines.front(), &pwd, _text_value_u4.length());
				tbb.make(lines);
			}
			else {
				tbb.make(string4_to_unichar(*str, false, false, !is_multiline()));
			}
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

		// mark input status change
		mark_none(kInput_Status | kRecursive_Visible_Region);

		return Vec2(_lines->max_width(), _lines->max_height());
	}

	void Input::solve_marks(uint32_t mark) {
		if (mark & kInput_Status) {
			unmark(kInput_Status);
			// text cursor status
			refresh_cursor_screen_position(); // text layout

			View::solve_marks(mark);

			if (_editing) {
				pre_render()->host()->dispatch()->
					set_ime_keyboard_spot_location(input_spot_location());
			}
		} else {
			View::solve_marks(mark);
		}
	}

	bool Input::solve_visible_region() {
		if (!Box::solve_visible_region())
			return false;
		_lines->solve_visible_region();
		_lines->solve_visible_region_blob(&_blob, &_blob_visible);
		return true;
	}

	void Input::refresh_cursor_screen_position() {

		if ( _editing ) {
			auto size = content_size();
			auto final_width = size.x();
			auto final_height = size.y();
			auto text_offset = input_text_offset();
			TextBlob* blob = nullptr;
			
			for ( int i = 0; i < _blob.length(); i++ ) {
				auto &it  = _blob[i];
				auto index = it.index;
				auto end = index + it.core.glyphs.length();
				if (_cursor >= index && _cursor <= end) {
					blob = &it; break;
				} else if (i + 1 < _blob.length()) {
					if (_cursor < _blob[i + 1].index) {
						blob = &it; break;
					}
				} else if (it.line == _lines->last()->line) { // last blob
					blob = &it;
				}
			}

			// 计算光标的具体偏移位置
			TextLines::Line* line = nullptr;

			if ( blob && _text_value_u4.length() ) { // set cursor pos
				auto idx = _cursor - blob->index;
				float offset = 0;
				if (idx < blob->core.offset.length()) {
					offset = blob->core.offset[idx].x();
				} else if (blob->core.offset.length()) {
					offset = blob->core.offset.back().x();
				}
				_cursor_linenum = blob->line;
				line = &_lines->line(_cursor_linenum);
				_cursor_x = line->origin + blob->origin + offset;
			} else { // 找不到cell定位到最后行
				switch ( _text_align ) {
					default:
						_cursor_x = 0; break;
					case TextAlign::kCenter:
						_cursor_x = final_width / 2; break;
					case TextAlign::kRight:
						_cursor_x = final_width; break;
				}
				_cursor_linenum = _lines->last()->line;
				line = &_lines->line(_cursor_linenum);
			}
			
			// 计算文本编辑状态下最适合的显示的文本偏移量
			// y
			if ( is_multiline() ) {
				if ( _lines->max_height() < final_height) {
					text_offset.set_y(0);
				} else {
					if ( line->start_y + text_offset.y() < 0 ) { // top cursor
						text_offset.set_y(-line->start_y);
					} else if (line->end_y + text_offset.y() > final_height) { // bottom cursor
						text_offset.set_y(final_height - line->end_y);
					}
					if ( text_offset.y() > 0 ) { // top
						text_offset.set_y(0);
					} else if ( text_offset.y() + _lines->max_height() < final_height ) { // bottom
						text_offset.set_y(final_height - _lines->max_height());
					}
				}
			} else {
				text_offset.set_y((final_height - _lines->max_height()) / 2);
			}

			// x
			auto max_width = _lines->max_width();
			if ( max_width <= final_width ) {
				text_offset.set_x(0);
			} else {
				// 让光标x轴始终在可见范围
				auto offset = _cursor_x + text_offset.x();
				
				if ( offset < 0 ) { // left cursor
					text_offset.set_x(-_cursor_x);
				} else if ( offset > final_width )  { // right cursor
					text_offset.set_x(final_width - _cursor_x);
				}
				
				// 检测文本x轴两端是在非法显示区域
				switch ( _text_align ) {
					default:
						offset = text_offset.x(); break;
					case TextAlign::kCenter:
						offset = text_offset.x() + (final_width - max_width) / 2.0; break;
					case TextAlign::kRight:
						offset = text_offset.x() + final_width - max_width; break;
				}

				if ( offset > 0 ) { // left
					text_offset.set_x(text_offset.x() - offset);
				} else {
					offset += max_width;
					if ( offset < final_width ) { // right
						text_offset.set_x(text_offset.x() - offset + final_width);
					}
				}
			}

			set_input_text_offset(text_offset);
		} else {
			if ( !is_multiline() ) {
				set_input_text_offset(Vec2(0, (content_size().y() - _lines->max_height()) / 2));
			}
		}
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
					count = Qk_MIN(cursor, -count);
					if ( count ) {
						String4 old = _text_value_u4;
						_text_value_u4 = String4(*old, cursor - count, *old + cursor, int(old.length()) - cursor);
						_cursor -= count;
						mark(kLayout_Typesetting); // 标记内容变化
					}
				} else if ( count > 0 ) {
					count = Qk_MIN(int(text_length()) - cursor, count);
					if ( count ) {
						String4 old = _text_value_u4;
						_text_value_u4 = String4(*old, cursor,
																	*old + cursor + count,
																	int(old.length()) - cursor - count);
						mark(kLayout_Typesetting); // 标记内容变化
					}
				}
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
		if ( _editing && _flag == kFlag_Normal ) {
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

	Object* Input::toObject() {
		return this;
	}

	void Input::onTextChange(uint32_t value) {
		value ? mark(value): mark_none();
	}

	void Input::set_type(KeyboardType value) {
		if (value != _type) {
			_type = value;
			if ( _editing ) {
				pre_render()->host()->dispatch()->
					set_ime_keyboard_open({ false, _type, _return_type, input_spot_location() });
			}
		}
	}

	void Input::set_return_type(KeyboardReturnType value) {
		if (value != _return_type) {
			_return_type = value;
			if ( _editing ) {
				pre_render()->host()->dispatch()->
					set_ime_keyboard_open({ false, _type, _return_type, input_spot_location() });
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

	void Input::set_readonly(bool value) {
		if (_readonly != value) {
			_readonly = value;
			if (_readonly) {
				_editing = false;
			} else if (is_focus()) {
				_editing = true;
			}
			mark(kInput_Status);
		}
	}

	void Input::set_max_length(uint32_t value) {
		_max_length = value;
		if (_max_length) { // check mx length
			if (_text_value_u4.length() > _max_length) {
				_text_value_u4 = _text_value_u4.substr(0, _max_length);
				mark(kLayout_Typesetting);
			}
		}
	}

	uint32_t Input::text_length() const {
		return _text_value_u4.length();
	}

	Vec2 Input::layout_offset_inside() {
		auto origin = origin_value();
		auto text_offset = input_text_offset();
		Vec2 offset(
			padding_left() - origin.x() + text_offset.x(),
			padding_top()  - origin.y() + text_offset.y()
		);
		if (_border) {
			offset.val[0] += _border->width[3]; // left
			offset.val[1] += _border->width[0]; // top
		}
		return offset;
	}

	Vec2 Input::input_text_offset() {
		return Vec2(_input_text_offset_x, _input_text_offset_y);
	}

	void Input::set_input_text_offset(Vec2 val) {
		_input_text_offset_x = val.x();
		_input_text_offset_y = val.y();
	}

	bool Input::run_task(int64_t sys_time) {
		if ( _flag > kFlag_Disable_Find ) {
			_cursor_twinkle_status = 1;
			if ( _flag == kFlag_Auto_Find || _flag == kFlag_Auto_Range_Select ) {
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

}
