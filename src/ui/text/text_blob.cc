/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, Louis.chu
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

#include "./text_blob.h"
#include "../../util/codec.h"

namespace qk {

	enum Symbol {
		kInvalid_Symbol,
		kLineFeed_Symbol,   // \n
		kSpace_Symbol,      // \t\s
		kPunctuation_Symbol,//,.:;?~
		kNumber_Symbol,    // 0-9
		kLetter_Symbol,    // a-z
		kLetterCap_Symbol, // A-Z
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

	Array<Array<Unichar>> to_unichar_lines(
		bool is_merge_space, bool is_merge_line_feed,
		bool disable_line_feed, bool ignore_single_space_line,
		bool (*each)(Unichar& out, void* ctx), void* ctx
	) {
		Array<Array<Unichar>> lines;
		Array<Unichar> row;
		Unichar data;
		bool is_merge_runing = false;
		bool is_push_row = false;
		
		auto push_row = [&]() {
			if (ignore_single_space_line) { // ignore single space line
				if (row.length() == 1 && row[0] == 0x20)
					return;
			}
			lines.push(std::move(row));
		};

		while (each(data, ctx)) {
			is_push_row = false;

			switch (unicode_to_symbol(data)) {
				case kLineFeed_Symbol:
					if (is_merge_line_feed) {
						if (!disable_line_feed)
							goto merge;
					} else { // new row
						is_push_row = true;
						push_row();
					}
					break;
				case kSpace_Symbol:
					if (is_merge_space) {
						merge:
						if (!is_merge_runing) {
							is_merge_runing = true;
							row.push(0x20);
						}
					} else {
						row.push(0x20);
					}
					break;
				default:
					is_merge_runing = false;
					row.push(data);
					break;
			}
		}

		if (row.length() || is_push_row) {
			push_row();
		}

		return lines;
	}

	Array<Array<Unichar>> string4_to_unichar(const Unichar *src, uint32_t length,
		bool is_merge_space, bool is_merge_line_feed, bool disable_line_feed) {
		
		struct Ctx { const Unichar* src, *end; } ctx = { src, src+length };

		auto each = [](Unichar &unicode, void* ctx) {
			auto _ = (Ctx*)ctx;
			if (_->src < _->end) {
				unicode = *_->src;
				_->src++;
				return true;
			}
			return false;
		};

		return to_unichar_lines(is_merge_space, is_merge_line_feed, disable_line_feed, false, each, &ctx);
	}

	Array<Array<Unichar>> string4_to_unichar(cString4& str,
		bool is_merge_space, bool is_merge_line_feed, bool disable_line_feed
	) {
		return string4_to_unichar(*str, str.length(), is_merge_space, is_merge_line_feed, disable_line_feed);
	}

	Array<Array<Unichar>> string_to_unichar(cString& str, WhiteSpace space, bool ignore_single_space_line) {
		Unichar data;
		Array<Array<Unichar>> lines;
		Array<Unichar> row;
		cChar* src = *str;
		cChar* end = src + str.length();

		bool is_merge_space = true;
		bool is_merge_line_feed = true;

		// 	NORMAL,        /* 合并空白序列,使用自动wrap */
		// 	NO_WRAP,       /* 合并空白序列,不使用自动wrap */
		// 	PRE,           /* 保留所有空白,不使用自动wrap */
		// 	PRE_WRAP,      /* 保留所有空白,使用自动wrap */
		// 	PRE_LINE,      /* 合并空白符序列,但保留换行符,使用自动wrap */

		if (space == WhiteSpace::Pre || space == WhiteSpace::PreWrap) { // 保留所有空白
			is_merge_space = false;
			is_merge_line_feed = false;
		} else if (space == WhiteSpace::PreLine) { // 保留换行符
			is_merge_line_feed = false;
		}

		struct Ctx { cChar *src, *end; } ctx = { src, end };

		auto each = [](Unichar &unicode, void* ctx) {
			auto _ = (Ctx*)ctx;
			if (_->src < _->end) {
				_->src += codec_decode_utf8_to_unichar(reinterpret_cast<const uint8_t*>(_->src), &unicode);
				return true;
			}
			return false;
		};

		return to_unichar_lines(is_merge_space, is_merge_line_feed, false, ignore_single_space_line, each, &ctx);
	}

	TextBlobBuilder::TextBlobBuilder(TextLines *lines, TextOptions *opts, Array<TextBlob>* blob)
		: _disable_overflow(false), _disable_auto_wrap(false), _lines(lines), _opts(opts), _blobOut(blob)
		, _index_of_unichar(0)
		, _font_size(opts->font_size().value)
	{
	}

	void TextBlobBuilder::set_font_size(float val) {
		_font_size = val;
	}

	void TextBlobBuilder::set_disable_overflow(bool value) {
		_disable_overflow = value;
	}

	void TextBlobBuilder::set_disable_auto_wrap(bool value) {
		_disable_auto_wrap = value;
	}

	void TextBlobBuilder::make(cString& text) {
		auto lines = string_to_unichar(
			text, _opts->white_space_value(), _lines->ignore_single_space_line()
		);
		make(lines);
	}

	void TextBlobBuilder::make(Array<Array<Unichar>>&& lines) {
		make(lines);
	}

	void TextBlobBuilder::make(Array<Array<Unichar>>& lines) {
		if (lines.length() == 0)
			return;
		auto white_space = _opts->white_space_value();
		auto word_break = _opts->word_break_value();
		bool is_auto_wrap = true;

		// enum class WhiteSpace: uint8_t {
		// 	NORMAL,        /* 合并空白序列,使用自动wrap */
		// 	NO_WRAP,       /* 合并空白序列,不使用自动wrap */
		// 	PRE,           /* 保留所有空白,不使用自动wrap */
		// 	PRE_WRAP,      /* 保留所有空白,使用自动wrap */
		// 	PRE_LINE,      /* 合并空白符序列,但保留换行符,使用自动wrap */
		// };

		// enum class WordBreak: uint8_t {
		// 	NORMAL,    /* 保持单词在同一行并清除行尾溢出的空白符 */
		// 	BREAK_WORD,/* 保持单词在同一行,除非单词长度超过一行才截断 */
		// 	BREAK_ALL, /* 以字为单位行空间不足换行 */
		// 	KEEP_ALL,  /* 所有连续的字符都当成一个单词,除非出现空白符、换行符、标点符 */
		// };

		auto no_limit = _lines->limit_range().end.x() == 0;

		if (_disable_auto_wrap || no_limit || // 不使用自动wrap
				white_space == WhiteSpace::NoWrap ||
				white_space == WhiteSpace::Pre
		) { // 不使用自动wrap,溢出不换行
			is_auto_wrap = false;
		}

		for ( int i = 0, len = lines.length(); i < len; i++ ) {
#if DEBUG
				if (lines[i].length() == 0) {
					//Qk_Log("====== TextBlobBuilder::make 0 %s", "\\n");
				} else {
					auto &weak = ArrayWeak<Unichar>(*lines[i], lines[i].length()).buffer();
					auto buf = codec_encode(Encoding::kUTF8_Encoding, weak);
					//Qk_Log("====== TextBlobBuilder::make 1 %s", *buf);
				}
#endif

			if (i) { // force line feed
				_lines->lineFeed(this, _index_of_unichar);
				_index_of_unichar++; // add index \n
			}

			auto fg_arr = _opts->font_family().value->
				makeFontGlyphs(lines[i], _opts->font_style(), _font_size);
			auto unichar = *lines[i];

			for (auto& fg: fg_arr) {
				if (is_auto_wrap) {
					switch (word_break) {
						default:
						case WordBreak::Normal:
							as_normal(fg, unichar, false, false); break;
						case WordBreak::BreakWord:
							as_normal(fg, unichar, true, false); break;
						case WordBreak::BreakAll:
							as_break_all(fg, unichar); break;
						case WordBreak::KeepAll:
							as_normal(fg, unichar, false, true); break;
					}
					unichar += fg.length();
				} else {  // no auto wrap
					as_no_auto_wrap(fg);
				}
				_index_of_unichar += fg.length();
			}
		}
	}

	// skip line start space symbol
	static bool skip_space(Unichar *unichar, TextLines *lines, int &j, int len) {
		auto i = j;
		do {
			if (unicode_to_symbol(unichar[i]) != kSpace_Symbol) break;
		} while (++i < len);
		return j != i ? ((j = i), true): false;
	}

	// NORMAL 保持单词在同一行并清除行尾溢出的空白符
	// BREAK_WORD 保持单词在同一行,除非单词长度超过一行才截断
	// KEEP_ALL 所有连续的字符都当成一个单词,除非出现空白符、换行符、标点符号
	void TextBlobBuilder::as_normal(FontGlyphs &fg, Unichar *unichar, bool is_BREAK_WORD, bool is_KEEP_ALL) {
		auto& glyphs = fg.glyphs();
		auto  offset = fg.getHorizontalOffset();
		auto  line = _lines->last();
		bool  isEmpty = line->width == 0.0; // empty line
		auto  font_size = _font_size;
		auto  line_height = _opts->line_height().value;

		float limitX = _lines->limit_range().end.width();
		float origin = _lines->pre_width();
		int   len = fg.glyphs().length();
		int   start = 0, j = 0;

		auto add_text_blob = [&](int j, bool isPre) {
			if (!isPre) {
				_lines->finish_text_blob_pre(); // finish pre
			}
			if (start < j) {
				_lines->add_text_blob(
					{fg.typeface(), font_size, line_height, _index_of_unichar + start, _blobOut},
					glyphs.slice(start, j).buffer(), offset.slice(start, j + 1).buffer(), isPre
				);
			}
		};

		while (j < len) {
			auto sym = unicode_to_symbol(unichar[j]);
			auto x = origin + offset[j + 1].x();
			auto overflow = x > limitX;

			// check word end
			// prev word end or next word start, record position and offset
			if (sym == kSpace_Symbol || (is_KEEP_ALL ? sym == kPunctuation_Symbol/*,.*/ : sym < kNumber_Symbol/*,\s\n你*/)) {
				auto i = overflow ? j: j+1; // not overflow then add word char to line
				add_text_blob(i, false); // Add blob immediately
				start = i;
				isEmpty = line->width == 0.0;
			}

			// check wrap overflow new line
			if (overflow) {
				if (sym == kSpace_Symbol) { // skip end of line space
					skip_space(unichar, _lines, j, len);
					start = j;
					continue;
				}
				// Add new line when not empty or Force truncation and add new line
				if (!isEmpty || is_BREAK_WORD) {
					add_text_blob(j, !isEmpty); // Add immediately when empty lines appear
					_lines->push(_opts); // new row
					line = _lines->last();
					isEmpty = true;
					start = j;
					origin = _lines->pre_width() - offset[j].x();
					x = origin + offset[j+1].x(); // new x
				}
			}
			_lines->set_pre_width(x);
			j++;
		}

		add_text_blob(len, true);
	}

	// BREAK_ALL 以字符为单位行空间不足换行并清除行尾溢出的空白符
	void TextBlobBuilder::as_break_all(FontGlyphs &fg, Unichar *unichar) {
		auto& glyphs = fg.glyphs();
		auto  offset = fg.getHorizontalOffset();
		auto  font_size = _opts->font_size().value;
		auto  line_height = _opts->line_height().value;

		float limitX = _lines->limit_range().end.width();
		float origin = _lines->pre_width();
		int   len = glyphs.length();
		int   start = 0, j = 0;

		auto add_text_blob = [&]() {
			_lines->finish_text_blob_pre(); // finish pre
			if (start < j) {
				_lines->add_text_blob(
					{fg.typeface(), font_size, line_height, _index_of_unichar + start, _blobOut},
					glyphs.slice(start, j).buffer(), offset.slice(start, j+1).buffer(), false
				);
			}
		};

		while (j < len) {
			auto x = origin + offset[j+1].x();
			if (x > limitX) { // overflow
				add_text_blob();
				if (skip_space(unichar, _lines, j, len)) { // skip end of line space symbol
					start = j;
					continue;
				}
				_lines->push(_opts); // new row
				origin = _lines->pre_width() - offset[j].x();
				x = origin + offset[j+1].x();
				start = j;
			}
			_lines->set_pre_width(x);
			j++;
		}

		add_text_blob();
	}

	void TextBlobBuilder::as_no_auto_wrap(FontGlyphs &fg) {
		auto origin = _lines->pre_width();
		auto offset = fg.getHorizontalOffset();
		auto overflow = _opts->text_overflow_value();
		auto limitX = _lines->limit_range().end.width();
		auto font_size = _font_size;
		auto line_height = _opts->line_height().value;

		if (!_disable_overflow && overflow != TextOverflow::Normal && limitX > 0) {
			if (origin >= limitX) return; // skip

			// CLIP,            /* 剪切 */
			// ELLIPSIS,        /* 剪切并显示省略号 */
			// ELLIPSIS_CENTER, /* 剪切并居中显示省略号 */

			int overflow_val = origin + offset.back().x() - limitX;
			if (overflow_val > 0) {
				uint32_t len = fg.glyphs().length();

				if (overflow == TextOverflow::Clip) {
					for (uint32_t j = 0; j < len; j++) {
						float x = origin + offset[j + 1].x();
						if (x > limitX) {
							// discard overflow part
							_lines->add_text_blob(
								{fg.typeface(), font_size, line_height, _index_of_unichar, _blobOut},
								fg.glyphs().slice(0, j).buffer(), offset.slice(0, j+1).buffer(), false
							);
							_lines->set_pre_width(limitX);
							break;
						}
					}
				} else { // ELLIPSIS or ELLIPSIS_CENTER

					Array<Unichar> uinchar({46,46,46});
					auto ellipsis = _opts->font_family().value->makeFontGlyphs(uinchar, _opts->font_style(), font_size)[0];
					auto ellipsis_offset = ellipsis.getHorizontalOffset();
					auto ellipsis_width = ellipsis_offset.back();
					auto limit2 = limitX - ellipsis_width.x();

					if (limit2 >= 0) {
						uint32_t j = 0;
						for (; j < len; j++) {
							float x = origin + offset[j + 1].x();
							if (x > limit2) {
								if (j) {
									_lines->add_text_blob(
										{fg.typeface(), font_size, line_height, _index_of_unichar, _blobOut},
										fg.glyphs().slice(0, j).buffer(), offset.slice(0, j+1).buffer(), false
									);
								}
								break;
							}
						}

						// add ellipsis
						_lines->add_text_blob(
							{ellipsis.typeface(), font_size, line_height, j, _blobOut},
							ellipsis.glyphs(), ellipsis_offset, false
						);
						_lines->set_pre_width(limitX);

					} else { // limit2 < 0.0, only add ellipsis
						for (int j = 0; j < 3; j++) {
							float x = origin + offset[j + 1].x();
							if (x > limitX) {
								_lines->add_text_blob(
									{ellipsis.typeface(), font_size, line_height, _index_of_unichar, _blobOut},
									ellipsis.glyphs().slice(0, j).buffer(), ellipsis_offset.slice(0, j + 1).buffer(), false
								);
								_lines->set_pre_width(limitX);
								break;
							}
						}
						Qk_ASSERT(_lines->pre_width() == limitX);
					}
				}

				return;
			}
		}

		_lines->add_text_blob({fg.typeface(), font_size, line_height, _index_of_unichar, _blobOut}, fg.glyphs(), offset, false);
		_lines->set_pre_width(origin + offset.back().x());
	}

}
