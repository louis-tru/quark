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
#include "../window.h"
#include "./textarea.h"
#include "../../util/codec.h"

namespace qk {

	enum {
		kFlag_Normal = 0,         // 正常状态未激活光标查找
		kFlag_Find_Cursor_Wait,   // 等待超时激活光标查找
		kFlag_Find_Cursor_Disable,// 禁用激活光标查找
		kFlag_Find_Cursor,        // 激活光标查找
		kFlag_Auto_Find_Cursor,   // 激活自动光标查找
		kFlag_Range_Select,       // 范围选择
		kFlag_Auto_Range_Select,  // 自动范围选择
		kFlag_Click_Focus_Check,  // 点击聚焦检查,检查成功后设置成`kFlag_Disable_Click_Focus`状态
		kFlag_Disable_Click_Focus,// 禁用点击聚焦
	};

	Qk_DEFINE_INLINE_MEMBERS(Input, Inl) {
		#define _this static_cast<Input::Inl*>(this)
		#define _async_call window()->preRender().async_call
	public:

		#pragma mark Utils
		void prevent_default(UIEvent &evt) {
			if (_editing) {
				if (_flag == kFlag_Auto_Find_Cursor || _flag == kFlag_Find_Cursor) {
					evt.return_value = 0;
				}
			}
		}

		Vec2 get_position() {
			Vec2 point(padding_left(), padding_top());
			if (_border) {
				point[0] += _border->width[3]; // left
				point[1] += _border->width[0]; // top
			}
			return _mat * point;
		}

		#pragma mark Event handles
		// ============================================================================
		// Event handles
		// ============================================================================
		void handle_Touchstart(UIEvent& evt) {
			auto e = static_cast<TouchEvent*>(&evt);
			Vec2 args(e->changed_touches()[0].x, e->changed_touches()[0].y);
			_async_call([](auto ctx, auto arg) { ctx->start_action(arg.arg, true); }, this, args);
		}

		void handle_Touchmove(UIEvent& evt) {
			prevent_default(evt);
			auto e = static_cast<TouchEvent*>(&evt);
			Vec2 arg(e->changed_touches()[0].x, e->changed_touches()[0].y);
			_async_call([](auto ctx, auto arg) { ctx->move_action(arg.arg); }, this, arg);
		}

		void handle_Touchend(UIEvent& evt) {
			auto e = static_cast<TouchEvent*>(&evt);
			Vec2 arg(e->changed_touches()[0].x, e->changed_touches()[0].y);
			_async_call([](auto ctx, auto arg) { ctx->end_action(arg.arg); }, this, arg);
		}

		void handle_Mousedown(UIEvent& evt) {
			auto e = static_cast<MouseEvent*>(&evt);
			Vec2 arg(e->x(), e->y());
			_async_call([](auto ctx, auto arg) { ctx->start_action(arg.arg, false); }, this, arg);
		}

		void handle_Mousemove(UIEvent& evt) {
			prevent_default(evt);
			auto e = static_cast<MouseEvent*>(&evt);
			Vec2 arg(e->x(), e->y());
			_async_call([](auto ctx, auto arg) { ctx->move_action(arg.arg); }, this, arg);
		}

		void handle_Mouseup(UIEvent& evt) {
			auto e = static_cast<MouseEvent*>(&evt);
			Vec2 arg(e->x(), e->y());
			_async_call([](auto ctx, auto arg) { ctx->end_action(arg.arg); }, this, arg);
		}

		void handle_Click(UIEvent& evt) {
			auto e = static_cast<ClickEvent*>(&evt);

			if ( !_editing && _flag != kFlag_Disable_Click_Focus ) {
				focus();
			}

			_async_call([](auto self, auto arg) {
				if ( self->_editing ) {
					self->window()->dispatch()->set_ime_keyboard_open({
						false, self->_type, self->_return_type, self->spot_rect()
					});
				} else {
					if ( self->_flag == kFlag_Disable_Click_Focus ) { // 禁用点击聚焦
						self->_flag = kFlag_Normal;
					} else {
						if (!self->is_focus())
							self->preRender().post(Cb([self](auto &e) { self->focus(); }), self);
						self->handle_Focus_for_render_t();
						self->find_cursor(arg.arg);
					}
				}
			}, this, Vec2(e->x(), e->y()));
		}

		void handle_Keydown(UIEvent& evt) {
			_async_call([](auto ctx, auto arg) {
				if ( ctx->_editing && ctx->_flag == kFlag_Normal ) {
					auto line_height = ctx->_lines->line(0).end_y;
					switch ( arg.arg ) {
						case KEYCODE_LEFT:
							ctx->_cursor = Qk_MAX(0, int(ctx->_cursor - 1));
							break;
						case KEYCODE_UP: {
							Vec2 pos = ctx->_mat * ctx->spot_offset();
							Vec2 coord(pos.x(), pos.y() - (line_height * 1.5));
							ctx->find_cursor(coord);
							break;
						}
						case KEYCODE_RIGHT:
							ctx->_cursor = Qk_MIN(ctx->text_length(), ctx->_cursor + 1);
							break;
						case KEYCODE_DOWN: {
							Vec2 pos = ctx->_mat * ctx->spot_offset();
							Vec2 coord(pos.x(), pos.y() + (line_height * 0.5));
							ctx->find_cursor(coord);
							break;
						}
						case KEYCODE_PAGE_UP: /* TODO page up */
							break;
						case KEYCODE_PAGE_DOWN: /* TODO page down */
							break;
						case KEYCODE_MOVE_HOME:
							ctx->_cursor = 0;
							break;
						case KEYCODE_MOVE_END:
							ctx->_cursor = ctx->text_length();
							break;
						default: break;
					}
					ctx->limit_cursor_in_marked_text();
					ctx->reset_cursor_twinkle_task_timeout();
					ctx->mark(kInput_Status, true);
				}
			}, this, static_cast<KeyEvent*>(&evt)->keycode());
		}

		void handle_Focus_for_render_t() {
			_editing = _readonly ? false: true;
			_cursor_twinkle_status = 0;
			_flag = kFlag_Normal;
			mark(kInput_Status, true);
			window()->preRender().addtask(this);
		}

		void handle_Focus(UIEvent& evt) {
			_async_call([](auto ctx, auto args) { ctx->handle_Focus_for_render_t(); }, this, 0);
		}

		void handle_Blur(UIEvent& evt) {
			_async_call([](auto ctx, auto args) {
				ctx->_editing = false;
				ctx->_flag = kFlag_Normal;
				if ( ctx->_marked_text.length() ) {
					ctx->input_unmark_text(ctx->_marked_text);
				} else {
					ctx->mark(kInput_Status, true);
				}
				ctx->window()->preRender().untask(ctx);
			}, this, 0);
		}

		// ============================================================================

		void start_action(Vec2 point, bool isTouch) {
			if ( _editing ) {
				_point = point;
				if ( _flag == kFlag_Normal ) { // 开始超时激活光标定位
					if ( is_multiline() && isTouch ) {
						_flag = kFlag_Find_Cursor_Wait;
						// 多行文本输入并且为在touch事件时为了判断是否为滚动与定位查找操作,
						// 只有长按输入框超过1秒没有移动才表示激活光标查找
						preRender().post(Cb([this](auto &e) { // delay call
							_async_call([](auto ctx, auto arg) {
								if ( ctx->_flag == kFlag_Find_Cursor_Wait ) { // 如果状态没有改变继续
									ctx->_flag = kFlag_Find_Cursor; // 激活光标定位
									ctx->find_cursor(ctx->_point);
								}
							}, this, 0);
						}), this, 1e6);
					} else { // 立即激活
						_flag = kFlag_Find_Cursor;
						find_cursor(_point);
					}
				}
			} else {
				if ( _flag == kFlag_Normal ) {
					if ( is_multiline() && isTouch ) { // 多行移动后禁用焦点
						_point = point;
						_flag = kFlag_Click_Focus_Check;  // 开始检测移动聚焦
					}
				}
			}
		}

		/**
		 * 移动鼠标或者触摸时，执行自动光标查找或者范围选择
		*/
		void move_action(Vec2 point) {
			if ( _editing ) {
				_point = point;

				switch (_flag) {
					case kFlag_Find_Cursor_Wait:          // 等待激活光标定位
						_flag = kFlag_Find_Cursor_Disable;  // 立即禁用
						break;
					case kFlag_Find_Cursor: { // 光标定位
						auto has = is_auto_find_is_required(_point);
						if ( has.x() || has.y() ) {
							_flag = kFlag_Auto_Find_Cursor;
						} else {
							find_cursor(_point); // 立即查找位置
						}
						break;
					}
					case kFlag_Auto_Find_Cursor: { // 自动光标定位
						auto has = is_auto_find_is_required(_point);
						if ( has.x() || has.y() ) {
							if ( (has.x() & has.y()) == 0 ) {
								find_cursor(_point);
							}
						} else { // 不需要使用自动选择
							_flag = kFlag_Find_Cursor;
						}
						break;
					}
					case kFlag_Range_Select: // 范围选择
						break;
					case kFlag_Auto_Range_Select: // 自动范围选择
						break;
					default: break;
				}
			} else {
				if ( _flag == kFlag_Click_Focus_Check ) { // 已经开始检测
					if ( is_multiline() ) { // 多行移动后禁用点击聚焦
						auto textarea = static_cast<Textarea*>(static_cast<Input*>(this));
						if ( textarea->scroll_x() != 0 || textarea->scroll_y() != 0 ) {
							// 计算移动距离
							float d = sqrtf(powf(point.x() - _point.x(), 2) + powf(point.y() - _point.y(), 2));
							if ( d > 5 ) { // 移动超过5禁用点击聚焦
								_flag = kFlag_Disable_Click_Focus;
							}
						}
					} // if ( is_multiline() )
				}
			}
		}

		void end_action(Vec2 point) {
			if ( _editing ) {
				if ( _flag == kFlag_Find_Cursor_Wait || _flag == kFlag_Find_Cursor ) {
					find_cursor(point);
				}
			}
			_flag = kFlag_Normal;
		}

		Vec2 spot_offset() {
			auto offset = input_text_offset();
			auto y = _lines->line(_cursor_line).baseline - _cursor_ascent + _cursor_height + offset.y();
			auto x = _cursor_x + offset.x();

			x += padding_left();
			y += padding_top();

			if (_border) {
				x += _border->width[3]; // left
				y += _border->width[0]; // top
			}
			Vec2 left_bottom(x, y);
			// Qk_DEBUG("spot_offset,x:%f,y:%f", left_bottom.x(), left_bottom.y());

			return left_bottom;
		}

		Rect spot_rect() {
			auto left_bottom = spot_offset();
			auto right_top = Vec2(left_bottom.x() + 1, left_bottom.y() - _cursor_height);
			auto a = _mat * left_bottom;
			auto b = _mat * right_top;
			return {
				{Qk_MIN(a.x(), b.x()),Qk_MIN(a.y(), b.y())},
				{fabsf(a.x() - b.x()),fabsf(a.y() - b.y())}
			};
		}

		/**
		 * 当绝对座标超过输入框边界时才返回非零值
		*/
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

		/**
		 * 在task任务中持续调用，自动边界查找光标导引，每次调用成功会光标会移动一个字符或移动一行，
		 * 当 `is_auto_find_is_required(_point)` 返回非零时才能生效
		*/
		void auto_find_cursor() {
			if ( !_editing || _blob.length() == 0 ) return;

			auto pos = get_position();
			auto point = _point;
			auto offset = input_text_offset();
			auto dir = is_auto_find_is_required(point);

			if (_flag == kFlag_Auto_Find_Cursor) { // 自动光标定位

				if ( dir.y() ) {
					int linenum = _cursor_line;
					linenum += dir.y();
					linenum = Qk_MIN(_lines->last()->line, Qk_MAX(linenum, 0));
					point.set_y(pos.y() + _lines->line(linenum).baseline + offset.y());
				}

				if ( dir.x() ) {
					int begin = _blob.length() - 1;

					for ( ; begin >= 0; begin-- ) {
						if ( _blob[begin].line == _cursor_line ) break;
					}

					int cursor = _cursor + (dir.x() > 0 ? 1: -1);
					cursor = Qk_MIN(text_length(), Qk_MAX(cursor, 0));
					
					for ( int j = begin; j >= 0; j-- ) {
						auto cell = &_blob[j];
						if ( cell->line == _cursor_line ) {
							if ( int(cell->index) <= cursor ) {
								float x = cell->origin + cell->blob.offset[
									Qk_MIN(cursor - cell->index, cell->blob.glyphs.length())
								].x();
								x += _lines->line(cell->line).origin;
								point.set_x(pos.x() + offset.x() + x);
								break;
							}
						} else {
							cell = &_blob[j+1];
							float x = cell->origin + cell->blob.offset.front().x();
							x += _lines->line(cell->line).origin;
							point.set_x(pos.x() + offset.x() + x);
							break;
						}
					}
				}

				find_cursor(point);
			} // if (_flag == kFlag_Auto_Find_Cursor)
			else if (_flag == kFlag_Auto_Range_Select) {
				//
			}
		}

		/**
		 * 通过窗口绝对座标查找并设置光标索引
		*/
		void find_cursor(Vec2 coord) {
			if ( !_editing || text_length() == 0 ) {
				return;
			}

			auto pos = get_position();
			// find line

			float x = coord.x() - pos.x();
			float y = coord.y() - pos.y();
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
				auto &blob = _blob[i];
				if ( blob.line == line->line ) { // 排除小余目标行cell
					if (cell_begin == -1)
						cell_begin = i;
					cell_end = i;
				} else {
					if (cell_begin != -1)
						break;
					//else if (blob.line > line->line) {
						// line_feed
					//	auto idx = Int32::max(0, int32_t(blob.index) - 1);
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
					_cursor = _blob[cell_end].index + _blob[cell_end].blob.glyphs.length(); // end_action
				} else {
					// 通过在cells中查询光标位置
					for ( int i = cell_begin; i <= cell_end; i++ ) {
						auto& cell = _blob[i];
						float offset_s = offset_start + cell.origin;
						float offset0 = offset_s + cell.blob.offset.front().x();

						for ( int j = 1, l = cell.blob.offset.length(); j < l; j++ ) {
							float offset = offset_s + cell.blob.offset[j].x();
							
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
			mark(kInput_Status, true);
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
			if ( _flag == kFlag_Auto_Find_Cursor || _flag == kFlag_Auto_Range_Select ) {
				set_task_timeout(time_monotonic() + 10000); // 10ms
			} else {
				set_task_timeout(time_monotonic() + 700000); // 700ms
			}
			_cursor_twinkle_status = 1;
		}

		// ============================================================================
		// input text insert action
		// ============================================================================
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
				if (_max_length && _value_u4.length() + text.length() > _max_length)
					return;

				if ( _cursor < text_length() ) { // insertd text
					String4 old = _value_u4;
					_value_u4 = String4(*old, _cursor, *text, text.length());
					_value_u4.append(*old + _cursor, old.length() - _cursor);
				} else { // append text
					_value_u4.append( text );
				}
				_cursor += text.length();
				mark_layout(kLayout_Typesetting, true); // 标记内容变化
			}
		}

		bool input_marked_text(cString4& text) {
			if (_max_length) {
				if ( _value_u4.length() + text.length() - _marked_text.length() > _max_length)
					return false;
			}
			if ( _marked_text.length() == 0 ) {
				_marked_text_idx = _cursor;
			}
			String4 old = _value_u4;
			_value_u4 = String4(*old, _marked_text_idx, *text, text.length());
			_value_u4.append(*old + _marked_text_idx + _marked_text.length(),
												old.length() - _marked_text_idx - _marked_text.length());

			_cursor += text.length() - _marked_text.length();
			_cursor = Qk_MAX(_marked_text_idx, _cursor);
			_marked_text = text;
			mark_layout(kLayout_Typesetting, true); // 标记内容变化

			return true;
		}

		void input_unmark_text(cString4& text) {
			if (input_marked_text(text)) {
				_cursor = _marked_text_idx + _marked_text.length();
				_marked_text = String4();
			}
		}

		void trigger_Change() {
			preRender().post(Cb([this](Cb::Data& e){
				Sp<UIEvent> evt(qk::New<UIEvent>(this));
				trigger(UIEvent_Change, **evt);
			}),this);
		}

	};

	Input::Input()
		: _security(false), _readonly(false)
		, _type(KeyboardType::Normal)
		, _return_type(KeyboardReturnType::Normal)
		, _placeholder_color(150, 150, 150)
		, _cursor_color(0x43,0x95,0xff) //#4395ff
		, _max_length(0)
		, _marked_color(0, 160, 255, 100)
		, _marked_text_idx(0), _cursor(0), _cursor_line(0)
		, _marked_blob_begin(0), _marked_blob_end(0)
		, _cursor_x(0), _input_text_offset_x(0), _input_text_offset_y(0)
		, _cursor_ascent(0), _cursor_height(0)
		, _editing(false), _cursor_twinkle_status(true), _flag(kFlag_Normal)
	{
		auto _inl = static_cast<Input::Inl*>(this);
		// bind events
		add_event_listener(UIEvent_Click, &Inl::handle_Click, _inl);
		add_event_listener(UIEvent_TouchStart, &Inl::handle_Touchstart, _inl);
		add_event_listener(UIEvent_TouchMove, &Inl::handle_Touchmove, _inl);
		add_event_listener(UIEvent_TouchEnd, &Inl::handle_Touchend, _inl);
		add_event_listener(UIEvent_TouchCancel, &Inl::handle_Touchend, _inl);
		add_event_listener(UIEvent_MouseDown, &Inl::handle_Mousedown, _inl);
		add_event_listener(UIEvent_MouseMove, &Inl::handle_Mousemove, _inl);
		add_event_listener(UIEvent_MouseUp, &Inl::handle_Mouseup, _inl);
		add_event_listener(UIEvent_Focus, &Inl::handle_Focus, _inl);
		add_event_listener(UIEvent_Blur, &Inl::handle_Blur, _inl);
		add_event_listener(UIEvent_KeyDown, &Inl::handle_Keydown, _inl);
	}

	View* Input::init(Window *win) {
		View::init(win);
		set_clip(true);
		set_receive(true);
		set_cursor(CursorStyle::Ibeam);
		set_text_word_break(TextWordBreak::BreakWord);
		return this;
	}

	bool Input::is_multiline() {
		return false;
	}

	void Input::set_value_u4(String4 val, bool isRt) {
		if (_value_u4 != val) {
			_value_u4 = val;
			mark_layout(kLayout_Typesetting, isRt);
			set_max_length(_max_length, isRt);
		}
	}

	String Input::value() const {
		return String(codec_encode(kUTF8_Encoding, _value_u4.array().buffer()));
	}

	String Input::placeholder() const {
		return String(codec_encode(kUTF8_Encoding, _placeholder_u4.array().buffer()));
	}

	void Input::set_value(String val, bool isRt) {
		set_value_u4(String4(codec_decode_to_uint32(kUTF8_Encoding, val)));
	}

	void Input::set_placeholder(String val, bool isRt) {
		set_placeholder_u4(String4(codec_decode_to_uint32(kUTF8_Encoding, val)));
	}

	bool Input::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			if (!is_ready_layout_typesetting())
				return false; // continue iteration
			layout_typesetting_input_text();
		}
		return true; // complete
	}

	Vec2 Input::layout_typesetting_input_text() {
		Vec2 c_size = content_size();
		_lines = new TextLines(this, text_align_value(), c_size, _layout_wrap_x_Rt);
		TextConfig cfg(this, shared_app()->defaultTextOptions());

		FontMetricsBase metrics;

		_lines->set_stable_line_height(text_size().value, text_line_height().value);
		_cursor_height = text_family().value->match(font_style())->getMetrics(&metrics, text_size().value);

		_cursor_ascent = -metrics.fAscent;
		_marked_blob_begin = _marked_blob_end = 0;

		_blob_visible.clear();
		_blob.clear();

		String4 &str = _value_u4.length() ? _value_u4: _placeholder_u4;

		if (str.length()) { // text layout
			TextBlobBuilder tbb(*_lines, this, &_blob);

			if (!is_multiline()) {
				tbb.set_disable_auto_wrap(true);
			}
			tbb.set_disable_overflow(true);

			if (_value_u4.length() && !_security && _marked_text.length()) { // marked text layout
				auto src = *_value_u4;
				auto marked = _marked_text_idx;
				auto marked_end = marked + _marked_text.length();

				Array<TextBlob> blobTmp;

				auto make = [&](const Unichar *src, uint32_t len) {
					tbb.make(string4_to_unichar(src, len, false, false, !is_multiline()));
					_lines->finish_text_blob_pre();
					if (blobTmp.length())
						blobTmp.concat(std::move(_blob));
					else
						blobTmp = std::move(_blob);
				};

				if ( marked ) { // start
					make(src, marked);
				}
				_marked_blob_begin = blobTmp.length();
				make(src+marked, _marked_text.length());
				_marked_blob_end = blobTmp.length();

				if ( marked_end < _value_u4.length() ) {
					make(src+marked_end, _value_u4.length()-marked_end);
				}
				_blob = std::move(blobTmp);
			}
			else if ( _value_u4.length() && _security ) { // password
				Unichar pwd = 9679; /*●*/
				Array<Array<Unichar>> lines(1);
				lines.front().extend(_value_u4.length());
				memset_pattern4(*lines.front(), &pwd, _value_u4.length());
				tbb.make(lines);
			}
			else {
				tbb.make(string4_to_unichar(str, false, false, !is_multiline()));
			}

			if (str[str.length() - 1] == '\n')
				// Add a empty blob placeholder
				_lines->add_text_empty_blob(&tbb, tbb.index_of_unichar());
		}

		_lines->finish();

		Vec2 new_size(
			_layout_wrap_x_Rt ? solve_layout_content_wrap_limit_width(_lines->max_width()): c_size.x(),
			_layout_wrap_y_Rt ? solve_layout_content_wrap_limit_height(_lines->max_height()): c_size.y()
		);

		if (new_size != c_size) {
			set_content_size(new_size);
			parent_Rt()->onChildLayoutChange(this, kChild_Layout_Size);
		}

		unmark(kLayout_Typesetting);

		// mark input status change
		mark(kInput_Status | kRecursive_Visible_Region, true);
		
		// Qk_DEBUG("_lines->max_width(), _lines->max_height(), %f %f", _lines->max_width(), _lines->max_height());

		return Vec2(_lines->max_width(), _lines->max_height());
	}

	void Input::solve_marks(const Mat &mat, uint32_t mark) {
		if (mark & kInput_Status) {
			unmark(kInput_Status);
			solve_cursor_offset(); // text cursor status

			Box::solve_marks(mat, mark);

			if (_editing) {
				// update system ime input position
				window()->dispatch()->set_ime_keyboard_spot_rect(input_spot_rect());
			}
		} else {
			Box::solve_marks(mat, mark);
		}
	}

	bool Input::solve_visible_region(const Mat &mat) {
		if (!Box::solve_visible_region(mat))
			return false;
		_mat = mat;
		window()->clipRegion(screen_region_from_convex_quadrilateral(_vertex));
		_lines->solve_visible_region(mat);
		_lines->solve_visible_region_blob(&_blob, &_blob_visible);
		window()->clipRestore();
		return true;
	}

	/**
	 * 
	 * 1.通过文本光标 `_cursor` 计算光标在当前布局中的相对偏移位置与光标所在的行 `_cursor_line`
	 * 
	 * 2.通过计算好的 `_cursor_x` 计算文本编辑状态下布局文本显示最适合的偏移量,
	 * 如果输入的一行文本超过布局长度始终保持光标在文本布局的可视范围内的边界
	 * 
	 * @method solve_cursor_offset
	*/
	void Input::solve_cursor_offset() {
		if ( !_editing ) {
			if ( !is_multiline() ) // Restore normal rolling offset value
				set_input_text_offset({0, (content_size().y() - _lines->max_height()) / 2});
			return;
		}

		// ===========================
		// 查找光标位置附近的blob
		// ===========================
		TextBlob *cursor_blob = nullptr;
		for ( int i = _blob.length() - 1; i >= 0; i-- ) {
			if (_blob[i].index <= _cursor) { // blob index 小于等于_cursor 做为光标位置
				cursor_blob = &_blob[i];
				break;
			}
		}

		// ===========================
		// 计算光标的具体偏移位置x
		// ===========================
		auto c_size = content_size();
		TextLines::Line* line = nullptr;

		if ( cursor_blob ) { // set cursor pos
			Qk_ASSERT(_value_u4.length());
			auto len = cursor_blob->blob.offset.length();
			auto index = _cursor - cursor_blob->index;
			Qk_ASSERT(index >= 0);
			float offset = 0;
			if (index < len) { // Index is within the offset range
				offset = cursor_blob->blob.offset[index].x();
			} else if (len) { // Select last offset
				offset = cursor_blob->blob.offset.back().x();
			}
			_cursor_line = cursor_blob->line; // y
			line = &_lines->line(_cursor_line);
			_cursor_x = line->origin + cursor_blob->origin + offset; // x
		} else {
			// 找不到blob定位到最后行
			switch ( text_align_value() ) {
				default:
					_cursor_x = 0; break;
				case TextAlign::Center:
					_cursor_x = c_size.width() * 0.5; break;
				case TextAlign::Right:
					_cursor_x = c_size.width(); break;
			}
			_cursor_line = _lines->last()->line; // y
			line = &_lines->line(_cursor_line);
		}

		// ===========================
		// 计算文本编辑状态下最适合的显示的文本偏移量
		// ===========================
		auto text_offset = input_text_offset();
		// y
		if ( is_multiline() ) {
			if ( _lines->max_height() < c_size.height()) {
				text_offset.set_y(0);
			} else {
				if ( line->start_y + text_offset.y() < 0 ) { // top cursor
					text_offset.set_y(-line->start_y);
				} else if (line->end_y + text_offset.y() > c_size.height()) { // bottom cursor
					text_offset.set_y(c_size.height() - line->end_y);
				}
				if ( text_offset.y() > 0 ) { // top
					text_offset.set_y(0);
				} else if ( text_offset.y() + _lines->max_height() < c_size.height() ) { // bottom
					text_offset.set_y(c_size.height() - _lines->max_height());
				}
			}
		} else {
			text_offset.set_y((c_size.height() - _lines->max_height()) / 2);
		}

		// x
		auto max_width = _lines->max_width();
		if ( max_width <= c_size.width() ) {
			text_offset.set_x(0);
		} else {
			// 让光标x轴始终在可见范围
			auto offset = _cursor_x + text_offset.x();

			if ( offset < 0 ) { // left cursor
				text_offset.set_x(-_cursor_x);
			} else if ( offset > c_size.width() ) { // right cursor
				text_offset.set_x(c_size.width() - _cursor_x);
			}

			// 检测文本x轴两端是在非法显示区域
			switch ( text_align_value() ) {
				default:
					offset = text_offset.x(); break;
				case TextAlign::Center:
					offset = text_offset.x() + (c_size.width() - max_width) * 0.5; break;
				case TextAlign::Right:
					offset = text_offset.x() + c_size.width() - max_width; break;
			}

			if ( offset > 0 ) { // left
				text_offset.set_x(text_offset.x() - offset);
			} else {
				offset += max_width;
				if ( offset < c_size.width() ) { // right
					text_offset.set_x(text_offset.x() - offset + c_size.width());
				}
			}
		}
		
		// Qk_DEBUG("set_input_text_offset, %f %f %f %d", text_offset.x(), text_offset.y(), _lines->max_height(), _lines->length());

		set_input_text_offset(text_offset);
	}

	void Input::onActivate() {
		_textFlags = 0xffffffff;
	}

	TextInput* Input::asTextInput() {
		return this;
	}

	TextOptions* Input::asTextOptions() {
		return this;
	}

	void Input::input_delete(int count) {
		if ( _editing ) {
			int cursor = _cursor;
			if ( !_marked_text.length() ) {
				if ( count < 0 ) {
					count = Qk_MIN(cursor, -count);
					if ( count ) {
						String4 old = _value_u4;
						_value_u4 = String4(*old, cursor - count, *old + cursor, int(old.length()) - cursor);
						_cursor -= count;
						mark_layout(kLayout_Typesetting, true); // 标记内容变化
					}
				} else if ( count > 0 ) {
					count = Qk_MIN(int(text_length()) - cursor, count);
					if ( count ) {
						String4 old = _value_u4;
						_value_u4 = String4(*old, cursor,
																*old + cursor + count,
																old.length() - cursor - count);
						mark_layout(kLayout_Typesetting, true); // 标记内容变化
					}
				}
			}

			_this->trigger_Change();
			_this->reset_cursor_twinkle_task_timeout();
		}
	}

	void Input::input_insert(cString& text) {
		if ( _editing ) {
			_this->input_insert_text(_this->delete_line_feed_format(text));
			_this->trigger_Change();
			_this->reset_cursor_twinkle_task_timeout();
		}
	}

	void Input::input_marked(cString& text) {
		if ( _editing ) {
			_this->input_marked_text(_this->delete_line_feed_format(text));
			_this->trigger_Change();
			_this->reset_cursor_twinkle_task_timeout();
		}
	}

	void Input::input_unmark(cString& text) {
		if ( _editing ) {
			_this->input_unmark_text(_this->delete_line_feed_format(text));
			_this->trigger_Change();
			_this->reset_cursor_twinkle_task_timeout();
		}
	}

	void Input::input_control(KeyboardKeyCode name) {
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

	Rect Input::input_spot_rect() {
		if (_editing) {
			return _this->spot_rect();
		} else {
			return {};
		}
	}

	KeyboardType Input::input_keyboard_type() {
		return _type;
	}

	KeyboardReturnType Input::input_keyboard_return_type() {
		return _return_type;
	}

	Object* Input::toObject() {
		return nullptr;
	}

	View* Input::getViewForTextOptions() {
		return this;
	}

	void Input::set_type(KeyboardType value, bool isRt) {
		if (value != _type) {
			_type = value;
			if ( _editing ) {
				window()->dispatch()->
					set_ime_keyboard_open({ false, _type, _return_type, input_spot_rect() });
			}
		}
	}

	void Input::set_return_type(KeyboardReturnType value, bool isRt) {
		if (value != _return_type) {
			_return_type = value;
			if ( _editing ) {
				window()->dispatch()->
					set_ime_keyboard_open({ false, _type, _return_type, input_spot_rect() });
			}
		}
	}

	void Input::set_placeholder_u4(String4 value, bool isRt) {
		_placeholder_u4 = value;
		mark_layout(kLayout_Typesetting, isRt);
	}

	void Input::set_placeholder_color(Color value, bool isRt) {
		if (value != _placeholder_color) {
			_placeholder_color = value;
			mark(kLayout_None, isRt);
		}
	}

	void Input::set_cursor_color(Color value, bool isRt) {
		if (value != _cursor_color) {
			_cursor_color = value;
			mark(kLayout_None, isRt);
		}
	}

	void Input::set_security(bool value, bool isRt) {
		if (_security != value) {
			_security = value;
			mark_layout(kLayout_Typesetting, isRt);
		}
	}

	void Input::set_readonly(bool value, bool isRt) {
		if (_readonly != value) {
			_readonly = value;
			preRender().async_call([](auto self, auto arg) {
				if (arg.arg) {
					self->_editing = false;
				} else{
					if (self->is_focus()) {
						self->_editing = true;
					}
				}
				self->mark_layout(kInput_Status, true);
			}, this, value);
		}
	}

	void Input::set_max_length(uint32_t value, bool isRt) {
		_max_length = value;
		if (_max_length) { // check mx length
			window()->preRender().async_call([](auto self, auto arg) {
				if (self->_value_u4.length() > self->_max_length) {
					self->_value_u4 = self->_value_u4.substr(0, self->_max_length);
					self->mark_layout(kLayout_Typesetting, true);
				}
			}, this, 0);
		}
	}

	uint32_t Input::text_length() const {
		return _value_u4.length();
	}

	Vec2 Input::layout_offset_inside() {
		auto text_offset = input_text_offset();
		Vec2 offset(
			padding_left() + text_offset.x(),
			padding_top() + text_offset.y()
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

	bool Input::run_task(int64_t time) {
		if ( _flag > kFlag_Find_Cursor_Disable ) {
			if ( _flag == kFlag_Auto_Find_Cursor || _flag == kFlag_Auto_Range_Select ) {
				_this->auto_find_cursor();
			}
			_cursor_twinkle_status = 1;
			set_task_timeout(time + 100000); /* 100ms */
		} else {
			_cursor_twinkle_status = !_cursor_twinkle_status;
			set_task_timeout(time + 700000); /* 700ms */
			return true;
		}
		return false;
	}

	bool Input::can_become_focus() {
		return true;
	}

	ViewType Input::viewType() const {
		return kInput_ViewType;
	}

}
