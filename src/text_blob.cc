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

#include "./text_blob.h"
#include "./util/codec.h"

namespace noug {

	enum Symbol {
		kInvalid_Symbol,
		kLineFeed_Symbol,
		kSpace_Symbol,
		kPunctuation_Symbol,
		kNumber_Symbol,
		kLetter_Symbol,
		kLetterCap_Symbol,
	};

	Symbol unicode_to_symbol(Unichar unicode) {
		switch(unicode) {
			case 0x0A: // \n
				return kLineFeed_Symbol;
			case 0x09: // \t
			case 0x0B: // VT (vertical tab)
			case 0x0C: // \f
			case 0x0D: // \r
			case 0x20: // \s space
			// case 0xA0: // 160
			case 0x0:
				return kSpace_Symbol;
			case 48: case 49: case 50: case 51: case 52:
			case 53: case 54: case 55: case 56: case 57: return kNumber_Symbol; // 0-9
			case 65: case 66: case 67: case 68: case 69:
			case 70: case 71: case 72: case 73: case 74:
			case 75: case 76: case 77: case 78: case 79:
			case 80: case 81: case 82: case 83: case 84:
			case 85: case 86: case 87: case 88: case 89: case 90: return kLetterCap_Symbol; // A-Z
			case 97:  case 98:  case 99:  case 100: case 101:
			case 102: case 103: case 104: case 105: case 106:
			case 107: case 108: case 109: case 110: case 111:
			case 112: case 113: case 114: case 115: case 116:
			case 117: case 118: case 119: case 120: case 121: case 122: return kLetter_Symbol; // a-z
			case 44: // ,
			case 46: // .
			case 58: // :
			case 59: // ;
			case 63: // ?
			case 126: // ~
			// case 12290: // 。
			// case 65292: // ，
				return kPunctuation_Symbol;
			default:
				return kInvalid_Symbol;
		}
	}

	Array<Array<Unichar>> string_to_unichar(cString& str, TextWhiteSpace space) {
		Unichar data;
		Array<Array<Unichar>> lines;
		Array<Unichar> row;
		cChar* source = *str;
		cChar* end = source + str.length();

		bool is_merge_runing = false;
		bool is_merge_space = true;
		bool is_merge_line_feed = true;

		// 	NORMAL,        /* 合并空白序列,使用自动wrap */
		// 	NO_WRAP,       /* 合并空白序列,不使用自动wrap */
		// 	PRE,           /* 保留所有空白,不使用自动wrap */
		// 	PRE_WRAP,      /* 保留所有空白,使用自动wrap */
		// 	PRE_LINE,      /* 合并空白符序列,但保留换行符,使用自动wrap */

		if (space == TextWhiteSpace::PRE || space == TextWhiteSpace::PRE_WRAP) { // 保留所有空白
			is_merge_space = false;
			is_merge_line_feed = false;
		} else if (space == TextWhiteSpace::PRE_LINE) { // 保留换行符
			is_merge_line_feed = false;
		}

		auto push_row = [&]() {
			row.realloc(row.length() + 1);
			(*row)[row.length()] = 0;
			lines.push(std::move(row));
		};

		while (source < end) {
			source += Codec::decode_utf8_to_unichar(reinterpret_cast<const uint8_t*>(source), &data);

			switch (unicode_to_symbol(data)) {
				case kLineFeed_Symbol:
					if (is_merge_line_feed)
						goto merge;
					else // new row
						push_row();
					break;
				case kSpace_Symbol:
					if (is_merge_space) {
						merge:
						if (!is_merge_runing) {
							is_merge_runing = true;
							row.push(0x20);
						}
					}
					break;
				default:
					is_merge_runing = false;
					row.push(data);
					break;
			}
		}

		if (row.length()) push_row();

		return lines;
	}

	TextBlobBuilder::TextBlobBuilder(TextLines *lines, TextOptions *opts, Array<TextBlob>* blob)
		: _lines(lines), _opts(opts), _blob(blob)
	{}

	void TextBlobBuilder::make(cString& text) {

		auto text_white_space = _opts->text_white_space_value();
		auto text_word_break = _opts->text_word_break_value();
		bool is_auto_wrap = true;

		// enum class TextWhiteSpace: uint8_t {
		// 	NORMAL,        /* 合并空白序列,使用自动wrap */
		// 	NO_WRAP,       /* 合并空白序列,不使用自动wrap */
		// 	PRE,           /* 保留所有空白,不使用自动wrap */
		// 	PRE_WRAP,      /* 保留所有空白,使用自动wrap */
		// 	PRE_LINE,      /* 合并空白符序列,但保留换行符,使用自动wrap */
		// };

		// enum class TextWordBreak: uint8_t {
		// 	NORMAL,    /* 保持单词在同一行 */
		// 	BREAK_WORD,/* 保持单词在同一行,除非单词长度超过一行才截断 */
		// 	BREAK_ALL, /* 以字为单位行空间不足换行 */
		// 	KEEP_ALL,  /* 所有连续的字符都当成一个单词,除非出现空白符、换行符、标点符 */
		// };

		if (_lines->wrap_x() || // 容器没有固定宽度
				text_white_space == TextWhiteSpace::NO_WRAP ||
				text_white_space == TextWhiteSpace::PRE
		) { // 不使用自动wrap
			is_auto_wrap = false;
		}

		auto unis = string_to_unichar(text, text_white_space);

		for ( int i = 0; i < unis.length(); i++ ) {
			if (i) { // force line feed
				_lines->push(_opts);
			}

			auto fg_arr = _opts->text_family().value->makeFontGlyphs(unis[i], _opts->font_style(), _opts->text_size().value);
			auto unichar = *unis[i];

			for (auto& fg: fg_arr) {
				if (is_auto_wrap) {
					switch (text_word_break) {
						default:
						case TextWordBreak::NORMAL: as_normal(fg, unichar, false, false); break;
						case TextWordBreak::BREAK_WORD: as_normal(fg, unichar, true, false); break;
						case TextWordBreak::BREAK_ALL: as_break_all(fg, unichar); break;
						case TextWordBreak::KEEP_ALL: as_normal(fg, unichar, false, true); break;
					}
					unichar += fg.glyphs().length();
				} else {  // no auto wrap
					as_no_auto_wrap(fg);
				}
			}
		}

	}

	void TextBlobBuilder::as_no_auto_wrap(FontGlyphs &fg) {
		auto origin = _lines->pre_width();
		auto offset = fg.get_offset();
		auto overflow = _opts->text_overflow_value();
		auto limitX = _lines->size().x();
		auto text_size = _opts->text_size().value;
		
		if (overflow != TextOverflow::NORMAL) {
			if (origin >= limitX) return; // skip

			// CLIP,            /* 剪切 */
			// ELLIPSIS,        /* 剪切并显示省略号 */
			// ELLIPSIS_CENTER, /* 剪切并居中显示省略号 */

			int overflow_val = origin + offset.back() - limitX;
			if (overflow_val > 0) {
				int len = fg.glyphs().length();

				if (overflow == TextOverflow::CLIP) {
					for (int j = 0; j < len; j++) {
						float x = origin + offset[j + 1];
						if (x > limitX) {
							// discard overflow part
							_lines->add_text_blob({fg.typeface(), text_size, _blob}, fg.glyphs().slice(0, j), offset.slice(0, j + 1), false);
							_lines->set_pre_width(limitX);
							break;
						}
					}
				} else { // ELLIPSIS or ELLIPSIS_CENTER

					Array<Unichar> uinchar({46,46,46});
					auto ellipsis = _opts->text_family().value->makeFontGlyphs(uinchar, _opts->font_style(), text_size)[0];
					auto ellipsis_offset = ellipsis.get_offset();
					auto ellipsis_width = ellipsis_offset.back();
					auto limit2 = limitX - ellipsis_width;

					if (limit2 >= 0) {
						for (int j = 0; j < len; j++) {
							float x = origin + offset[j + 1];
							if (x > limit2) {
								if (j) {
									_lines->add_text_blob({fg.typeface(), text_size, _blob}, fg.glyphs().slice(0, j), offset.slice(0, j + 1), false);
								}
								break;
							}
						}

						// add ellipsis
						_lines->add_text_blob({fg.typeface(), text_size, _blob}, ellipsis.glyphs(), ellipsis_offset, false);
						_blob->back().origin = limitX - ellipsis_width; // align right
						_lines->set_pre_width(limitX);

					} else { // limit2 < 0.0, only add ellipsis
						for (int j = 0; j < 3; j++) {
							float x = origin + offset[j + 1];
							if (x > limitX) {
								_lines->add_text_blob({fg.typeface(), text_size, _blob}, ellipsis.glyphs().slice(0, j), ellipsis_offset.slice(0, j + 1), false);
								_lines->set_pre_width(limitX);
								break;
							}
						}
						N_ASSERT(_lines->pre_width() == limitX);
					}
				}

				return;
			}
		}

		_lines->add_text_blob({fg.typeface(), text_size, _blob}, fg.glyphs(), offset, false);
		_lines->set_pre_width(origin + offset.back());
	}

	// NORMAL 保持单词在同一行
	// BREAK_WORD 保持单词在同一行,除非单词长度超过一行才截断
	// KEEP_ALL 所有连续的字符都当成一个单词,除非出现空白符、换行符、标点符
	void TextBlobBuilder::as_normal(FontGlyphs &fg, Unichar *unichar, bool is_BREAK_WORD, bool is_KEEP_ALL) {
		auto& glyphs = fg.glyphs();
		auto  offset = fg.get_offset();
		auto  row = _lines->last();
		bool  line_head = row->width == 0.0;
		auto  text_size = _opts->text_size().value;

		float limitX = _lines->size().x();
		float origin = _lines->pre_width();
		int   len = fg.glyphs().length();
		int   start = 0;

		for (int j = 0; j < len; j++) {
			auto sym = unicode_to_symbol(unichar[j]);
			if (sym == kSpace_Symbol && _lines->last()->line && _lines->pre_width() == 0.0) {
				// skip line leading spaces
			skipLine:
				origin = -offset[j];
				if (++j < len) {
					sym = unicode_to_symbol(unichar[j]);
					if (sym == kSpace_Symbol)
						goto skipLine;
				} else {
					break;
				}
			}

			float x = origin + offset[j + 1];
			if (x > limitX) {
				if (is_BREAK_WORD) { // force new line
					_lines->add_text_blob({fg.typeface(), text_size, _blob}, glyphs.slice(start, j), offset.slice(start, j + 1), false);
					goto newLine;
				}
				if (line_head) { // line start then not new line
					_lines->set_pre_width(x);
				} else {
				newLine:
					_lines->push(); // new row
					line_head = true;
					origin = -offset[j];
				}
			} else {
				_lines->set_pre_width(x);
			}

			// prev word end or next word start, record position and offset
			if (is_KEEP_ALL ? sym == kSpace_Symbol || sym == kPunctuation_Symbol :
												sym < kNumber_Symbol
			) {
				_lines->add_text_blob({fg.typeface(), text_size, _blob}, glyphs.slice(start, j), offset.slice(start, j + 1), false);
				if (start < j)
					line_head = false;
				start = j;
			}
		}

		if (start < len) {
			_lines->add_text_blob({fg.typeface(), text_size, _blob}, glyphs.slice(start, len), offset.slice(start, len + 1), true);
		}
	}

	// BREAK_ALL 以字符为单位行空间不足换行
	void TextBlobBuilder::as_break_all(FontGlyphs &fg, Unichar *unichar) {
		auto& glyphs = fg.glyphs();
		auto  offset = fg.get_offset();
		auto  text_size = _opts->text_size().value;

		float limitX = _lines->size().x();
		float origin = _lines->pre_width();
		int   len = glyphs.length();
		int   start = 0;

		for (int j = 0; j < len; j++) {
			float x = origin + offset[j + 1];
			if (x > limitX) {
				_lines->add_text_blob({fg.typeface(), text_size, _blob}, glyphs.slice(start, j), offset.slice(start, j + 1), false);
				_lines->push(); // new row
				origin = -offset[j];
				start = j;
			} else {
				_lines->set_pre_width(x);
			}
		}

		_lines->add_text_blob({fg.typeface(), text_size, _blob}, glyphs.slice(start, len), offset.slice(start, len + 1), false);
	}

}