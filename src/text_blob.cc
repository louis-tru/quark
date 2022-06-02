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
			default:
				return kInvalid_Symbol;
		}
	}

	Array<Array<Unichar>> string_to_unichar(cString& str, TextWhiteSpace space) {
		Unichar data;
		Array<Array<Unichar>> rows;
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
			rows.push(std::move(row));
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

		return rows;
	}

	TextBlobBuilder::TextBlobBuilder(TextRows *rows, TextOptions *opts, Array<TextBlob>* blob)
		: _rows(rows), _opts(opts), _blob(blob)
	{}

	void TextBlobBuilder::make(cString& text) {
		FontMetrics metrics;

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

		if (_rows->wrap_x() || // 容器没有固定宽度
				text_white_space == TextWhiteSpace::NO_WRAP ||
				text_white_space == TextWhiteSpace::PRE
		) { // 不使用自动wrap
			is_auto_wrap = false;
		}

		auto unis = string_to_unichar(text, text_white_space);

		for ( int i = 0; i < unis.length(); i++ ) {
			if (i) { // force line feed
				_rows->push(_opts);
			}
			auto fg_arr = _opts->text_family().value->makeFontGlyphs(unis[i], _opts->font_style(), _opts->text_size().value);
			auto unichar = *unis[i];

			for (auto& fg: fg_arr) {
				if (is_auto_wrap) {
					switch (text_word_break) {
						default:
						case TextWordBreak::NORMAL: as_normal(fg, unichar); break;
						case TextWordBreak::BREAK_WORD: as_normal(fg, unichar); break;
						case TextWordBreak::BREAK_ALL: as_break_all(fg, unichar); break;
						case TextWordBreak::KEEP_ALL: as_keep_all(fg, unichar); break;
					}
					unichar += fg.glyphs().length();
				} else {
					as_no_auto_wrap(fg); // no auto wrap
				}
				fg    .get_metrics(&metrics);
				_rows->set_metrics(&metrics);
			}
		}

	}

	void TextBlobBuilder::as_no_auto_wrap(FontGlyphs &fg) {
		auto row = _rows->last();
		auto origin = row->width;
		auto offset = fg.get_offset();
		row->width += offset.back();
		_blob->push({
			fg.typeface(),
			fg.glyphs(), std::move(offset), origin, row->row_num,
		});
	}

	// NORMAL 保持单词在同一行
	// BREAK_WORD 保持单词在同一行,除非单词长度超过一行才截断
	void TextBlobBuilder::as_normal(FontGlyphs &fg, Unichar *unichar) {
		auto& glyphs = fg.glyphs();
		auto  offset = fg.get_offset();

		float limitX = _rows->size().x();
		float origin = _rows->cur_width;
		int   len = fg.glyphs().length();
		int   blob_start_idx = 0;

		for (int j = 0; j < len; j++) {
			float x = origin + offset[j + 1];
			if (x > limitX) {
				_rows->push(); // new row
				origin = -offset[j];
			} else {
				_rows->cur_width = x;
			}
			// prev word end or next word start, record position and offset
			if (unicode_to_symbol(unichar[j]) == kSpace_Symbol) { // space
				_rows->add_text_blob(fg, _blob, blob_start_idx, j + 1, true);
				blob_start_idx = j + 1;
			}
		}

		if (blob_start_idx < len) {
			_rows->add_text_blob(fg, _blob, blob_start_idx, len, false);
		}
	}

	// BREAK_ALL 以字符为单位行空间不足换行
	void TextBlobBuilder::as_break_all(FontGlyphs &fg, Unichar *unichar) {
		auto& glyphs = fg.glyphs();
		auto  offset = fg.get_offset();
		auto  row = _rows->last();

		float limitX = _rows->size().x();
		float origin = row->width;
		int   len = glyphs.length();
		int   blob_start_idx = 0;

		for (int j = 0; j < len; j++) {
			float x = origin + offset[j + 1];
			if (x > limitX) {
				if (j) {
					_blob->push({ fg.typeface(),
						glyphs.copy(blob_start_idx, j),
						offset.copy(blob_start_idx, j + 1), origin, row->row_num
					});
					blob_start_idx = j;
				}
				_rows->push(_opts); // new row
				row = _rows->last();
				origin = -offset[j];
			} else {
				row->width = x;
			}
		}

		_blob->push({ fg.typeface(),
			glyphs.copy(blob_start_idx, len),
			offset.copy(blob_start_idx, len + 1), origin, row->row_num
		});

		row->width = origin + offset[len + 1];
	}

	// KEEP_ALL 所有连续的字符都当成一个单词,除非出现空白符、换行符、标点符
	void TextBlobBuilder::as_keep_all(FontGlyphs &fg, Unichar *unichar) {
		// TODO ...
	}

}