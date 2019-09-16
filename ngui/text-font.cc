/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
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

#include "text-font.h"
#include "app.h"
#include "display-port.h"
#include "layout.h"
#include "text-rows.h"
#include "font/font-1.h"

XX_NS(ngui)

#define equal(attr) if (value.type != attr.type || value.value != attr.value)

TextFont::Data::Data()
: texture_level(TexureLevel::LEVEL_NONE)
, texture_scale(1)
, text_ascender(0)
, text_descender(0)
, text_hori_bearing(0)
, text_height(0)
, cell_draw_begin(0), cell_draw_end(0) {
	
}

/**
 * @constructor
 */
TextFont::TextFont()
: m_text_background_color({ TextAttrType::INHERIT })
, m_text_color({ TextAttrType::INHERIT })
, m_text_size({ TextAttrType::INHERIT })
, m_text_style({ TextAttrType::INHERIT })
, m_text_family(TextAttrType::INHERIT)
, m_text_line_height({ TextAttrType::INHERIT })
, m_text_shadow({ TextAttrType::INHERIT })
, m_text_decoration({ TextAttrType::INHERIT })
{ }

/**
 * @set text_background_color {TextColor}
 */
void TextFont::set_text_background_color(TextColor value) {
	equal(m_text_background_color) {
		m_text_background_color = value;
		mark_text(View::M_LAYOUT | View::M_TEXT_FONT);
	}
}

/**
 * @set text_color {TextColor}
 */
void TextFont::set_text_color(TextColor value) {
	equal(m_text_color) {
		m_text_color = value;
		mark_text(View::M_LAYOUT | View::M_TEXT_FONT);
	}
}

/**
 * @set font_size {TextSize}
 */
void TextFont::set_text_size(TextSize value) {
	value.value = XX_MAX(value.value, 0);
	equal(m_text_size) {
		m_text_size = value;
		mark_text(View::M_LAYOUT |
							View::M_CONTENT_OFFSET |  /* 变化字体尺寸会导致总体尺寸与偏移发发生改变
																					 * 文本视图无法直接处理尺寸变化,最终会传递给顶层Text处理
																					 * 如果没有顶层Text做默认排版处理
																					 *
																					 * 非Layout文本视图(Label)通过这个标记重新设置排版参数
																					 */
							View::M_TEXT_FONT | View::M_TEXT_SIZE);
	}
}

/**
 * @set text_style {TextStyle}
 */
void TextFont::set_text_style(TextStyle value) {
	equal(m_text_style) {
		m_text_style = value;
		mark_text(View::M_LAYOUT | View::M_CONTENT_OFFSET | View::M_TEXT_FONT);
	}
}

/**
 * @set text_family {TextFamily}
 */
void TextFont::set_text_family(TextFamily value) {
	equal(m_text_family) {
		m_text_family = value;
		mark_text(View::M_LAYOUT | View::M_CONTENT_OFFSET | View::M_TEXT_FONT);
	}
}

/**
 * @set text_shadow {TextShadow}
 */
void TextFont::set_text_shadow(TextShadow value) {
	value.value.size = XX_MAX(value.value.size, 0);
	equal(m_text_shadow) {
		m_text_shadow = value;
		mark_text(View::M_LAYOUT | View::M_TEXT_FONT);
	}
}

/**
 * @set text_line_height {TextLineHeight}
 */
void TextFont::set_text_line_height(TextLineHeight value) {
	value.value.height = XX_MAX(value.value.height, 0);
	equal(m_text_line_height) {
		m_text_line_height = value;
		mark_text(View::M_LAYOUT | View::M_CONTENT_OFFSET | View::M_TEXT_FONT);
	}
}

/**
 * @set text_decoration {TextDecoration}
 */
void TextFont::set_text_decoration(TextDecoration value) {
	equal(m_text_decoration) {
		m_text_decoration = value;
		mark_text(View::M_LAYOUT | View::M_TEXT_FONT);
	}
}

/**
 * @func get_font_glyph_table_and_fill_data
 */
FontGlyphTable* TextFont::get_font_glyph_table_and_height(Data& data,
																													TextLineHeightValue text_line_height) {
	
	//if ( m_text_family.name() == "icon" && m_text_line_height.value.height == 48 ) {
	//  LOG("icon");
	//}
	
	FontGlyphTable* table = font_pool()->get_table(m_text_family.value, m_text_style.value);
	XX_ASSERT(table);
	
	/* FontGlyph中获得的数据是 26.6 frac. 64pt 值, 所以这里需要除以这个比例 */
	float ratio = 4096.0 / m_text_size.value; /* 64.0 * 64.0 = 4096.0 */
	
	float line_height = text_line_height.is_auto() ?
											table->text_height() / ratio: /* 自动行高,使用字体文件中建议值 */
											text_line_height.height;
	float max_ascender = table->max_ascender() / ratio;
	float max_descender = table->max_descender() / ratio;
	float max_height = max_ascender + max_descender;
	
	float descender, ascender;
	
	descender = max_descender + (line_height - max_height) / 2.0;
	descender = XX_MAX(descender, 0);
	ascender = line_height - descender;
	
	data.text_ascender = ascender;
	data.text_descender = descender;
	data.text_height = table->text_height() / ratio;
	
	if ( data.text_height > max_height ) { // height > max-ascender + max-descender
		data.text_hori_bearing = max_ascender;
		data.text_height = max_height;
	} else { // height <= ascender + descender
		data.text_hori_bearing = max_ascender + (data.text_height - max_height) / 2.0;
	}
	
	return table;
}

/**
 * @func simple_layout_width
 */
float TextFont::simple_layout_width(cString& text) {
	Ucs2String str = Codec::decoding_to_uint16(Encoding::utf8, text).collapse_string();
	return simple_layout_width(str);
}

/**
 * @func simple_layout_width
 */
float TextFont::simple_layout_width(cUcs2String& text) {
	View* v = view();
	
	TextFamily family = { TextAttrType::INHERIT, app()->default_text_family().value };
	TextStyle style = { TextAttrType::INHERIT, app()->default_text_style().value };
	TextSize size = { TextAttrType::INHERIT, app()->default_text_size().value };
	int ok = 0;
	
	do {
		TextFont* text = v->as_text_font();
		if ( text ) {
			if (family.type == TextAttrType::INHERIT &&
					text->m_text_family.type != TextAttrType::INHERIT) {
				family = text->m_text_family; ok++;
			}
			if (style.type == TextAttrType::INHERIT &&
					text->m_text_style.type != TextAttrType::INHERIT) {
				style = text->m_text_style; ok++;
			}
			if (size.type == TextAttrType::INHERIT &&
					text->m_text_size.type != TextAttrType::INHERIT) {
				size = text->m_text_size; ok++;
			}
		}
		v = v->parent();
	} while(v && ok < 3);
	
	FontGlyphTable* table = font_pool()->get_table(family.value, style.value); XX_ASSERT(table);
	
	float rv = 0;
	float ratio = 4096.0 / size.value;
	
	for ( uint i = 0; i < text.length(); i++ ) {
		rv += table->glyph(text[i])->hori_advance() / ratio;
	}
	
	return rv;
}

/**
 * @func compute_text_visible_draw
 */
bool TextFont::compute_text_visible_draw(Vec2 vertex[4],
																				 Data& data,
																				 float x1, float x2, float in_offset_y) {
	bool has_visible_draw_range = false;
	data.cell_draw_begin = data.cell_draw_end = 0;
	
	/*
	 * 这里考虑到性能不做精确的多边形重叠测试，只测试图形在横纵轴是否与当前绘图区域是否为重叠。
	 * 这种模糊测试在大多数时候都是正确有效的。
	 */
	Region dre = display_port()->draw_region();
	Region re = View::screen_region_from_convex_quadrilateral(vertex);
	
	if (XX_MAX( dre.y2, re.y2 ) - XX_MIN( dre.y, re.y ) <= re.h + dre.h &&
			XX_MAX( dre.x2, re.x2 ) - XX_MIN( dre.x, re.x ) <= re.w + dre.w
	) {
		has_visible_draw_range = true;
	} else {
		return has_visible_draw_range;
	}
	
	if ( !data.cells.length() ) { return has_visible_draw_range; }
	
	// set cell visible draw range
	
	if ( data.cells.length() == 1 ) {
		data.cell_draw_begin = 0; data.cell_draw_end = 1;
		return has_visible_draw_range;
	}
	
	// 限制多行文本框绘图区域
	display_port()->push_draw_region(re);
	
	// 限制在这个区域
	dre = display_port()->draw_region();
	
	View* v = view();
	
	float start_x = -v->origin_x() - x1;
	float end_x   = -v->origin_x() + x2;
	bool  is_cell_draw_begin = false;
	Vec2  vertex2[4];
	float y, y2;
	
	y2 = v->origin_y() - XX_MAX(0, data.text_height - data.text_hori_bearing) + in_offset_y;
	
	y = data.cells[0].baseline - y2 - data.text_height;
	
	#define A vertex2[0]
	#define B vertex2[1]
	#define C vertex2[2]
	#define D vertex2[3]

	D = v->m_final_matrix * Vec2(start_x, y);
	C = v->m_final_matrix * Vec2(end_x, y);
	
	int line_num = -1;
	
	for ( auto& i : data.cells ) {
		Cell& cell = i.value();
		if ( line_num != int(cell.line_num) && cell.chars.length() ) {
			line_num = cell.line_num;
			y = cell.baseline - y2;
			
			A = D; B = C;
			D = v->m_final_matrix * Vec2(start_x, y);
			C = v->m_final_matrix * Vec2(end_x, y);
			re = View::screen_region_from_convex_quadrilateral(vertex2);
			
			if (XX_MAX( dre.y2, re.y2 ) - XX_MIN( dre.y, re.y ) < re.h + dre.h &&
					XX_MAX( dre.x2, re.x2 ) - XX_MIN( dre.x, re.x ) < re.w + dre.w
			) {
				if ( !is_cell_draw_begin ) {
					is_cell_draw_begin = 1;
					data.cell_draw_begin = i.index();
					data.cell_draw_end = data.cells.length();
				}
			} else {
				if ( is_cell_draw_begin ) {
					data.cell_draw_end = i.index(); break;
				}
			}
		}
	}
	
	display_port()->pop_draw_region();
	
	return has_visible_draw_range;
}

/**
 * @func set_glyph_texture_level
 */
void TextFont::set_glyph_texture_level(Data& data) {
	
	View* v = view();
	
	Vec2 p1(v->m_final_matrix * Vec2(1, 0));
	Vec2 p2(v->m_final_matrix * Vec2(0, 1));
	
	float x = p1.x() - p2.x();
	float y = p1.y() - p2.y();
	
	float scale = sqrtf(x * x + y * y) / 1.4142135623731 /* sqrtf(2) = 1.4142135623731 */;
	float final_text_size = m_text_size.value * scale;
	
	data.texture_level = font_pool()->get_glyph_texture_level(final_text_size); // 文本等级
	data.texture_scale = m_text_size.value / final_text_size;
}

// ----------------------------------- TextLayout -----------------------------------

XX_INLINE bool has_space_char(uint16 unicode, bool space, bool line_feed) {
	switch(unicode) {
		case 0x0A: // \n
			return line_feed;
		case 0x09: case 0x0B:
		case 0x0C: case 0x0D:
		case 0x20:
			// case 0xA0: // 160
		case 0x0:
			return space;
	}
	return false;
}

XX_INLINE bool has_english_char(uint16 unicode) {
	switch(unicode) {
		case 48: case 49: case 50: case 51: case 52:
		case 53: case 54: case 55: case 56: case 57: // 0-9
		case 65: case 66: case 67: case 68: case 69:
		case 70: case 71: case 72: case 73: case 74:
		case 75: case 76: case 77: case 78: case 79:
		case 80: case 81: case 82: case 83: case 84:
		case 85: case 86: case 87: case 88: case 89: case 90: // A-Z
		case 97:  case 98:  case 99:  case 100: case 101:
		case 102: case 103: case 104: case 105: case 106:
		case 107: case 108: case 109: case 110: case 111:
		case 112: case 113: case 114: case 115: case 116:
		case 117: case 118: case 119: case 120: case 121: case 122: // a-z
			return true;
	}
	return false;
}

/**
 * @struct Word 单词
 */
struct Word {
	Array<float>  offset;
	Array<uint16> chars;
	float         width;
	bool          newline;
	uint          count;
};

/**
 * @class TextLayout::Inl
 */
class TextLayout::Inl: public TextLayout {
public:
#define _inl(self) static_cast<TextLayout::Inl*>(self)
	
	/**
	 * @func solve_text_layout_mark
	 */
	void solve_text_layout_mark(TextLayout* parent) {
		
		if (m_text_background_color.type == TextAttrType::INHERIT) {
			m_text_background_color.value = parent->m_text_background_color.value;
		}
		if (m_text_color.type == TextAttrType::INHERIT) {
			m_text_color.value = parent->m_text_color.value;
		}
		if (m_text_size.type == TextAttrType::INHERIT) {
			m_text_size.value = parent->m_text_size.value;
		}
		if (m_text_style.type == TextAttrType::INHERIT) {
			m_text_style.value = parent->m_text_style.value;
		}
		if (m_text_family.type == TextAttrType::INHERIT) {
			m_text_family.value = parent->m_text_family.value;
		}
		if (m_text_line_height.type == TextAttrType::INHERIT) {
			m_text_line_height.value = parent->m_text_line_height.value;
		}
		if (m_text_shadow.type == TextAttrType::INHERIT) {
			m_text_shadow.value = parent->m_text_shadow.value;
		}
		if (m_text_decoration.type == TextAttrType::INHERIT) {
			m_text_decoration.value = parent->m_text_decoration.value;
		}
		if (m_text_overflow.type == TextAttrType::INHERIT) {
			m_text_overflow.value = parent->m_text_overflow.value;
		}
		if (m_text_white_space.type == TextAttrType::INHERIT) {
			m_text_white_space.value = parent->m_text_white_space.value;
		}
		
		View* v = view()->first();
		TextLayout* text = nullptr;
		
		while (v) {
			text = v->as_text_layout();
			if (text) {
				_inl(text)->solve_text_layout_mark(this);
			}
			v = v->next();
		}
		view()->mark_value &= (~View::M_TEXT_FONT); // 删除这些标记
	}
	
	/**
	 * @func new_row
	 */
	XX_INLINE void new_row(TextRows* rows, Cell& cell, Data& data, uint begin) {
		
		/* 结束上一行 */
		if ( cell.chars.length() ) {
			cell.baseline = rows->last()->baseline;
			data.cells.push(move(cell));
		} else {
			cell.offset.clear();
		}
		
		rows->push_row(data.text_ascender, data.text_descender); /* 添加新行 */
		
		cell.offset.push(0);              /* 添加第一个初始位置 */
		cell.line_num = rows->last_num(); /* 文本单元所在的行 */
		cell.begin = begin;
	}
	
	/**
	 * @func read_word
	 */
	XX_INLINE bool read_word(Word* word, float offset_start,
													 FontGlyphTable* table, float ratio,
													 Options::SpaceWrap opts, cUcs2String& string, uint begin, uint end) {
		if ( begin < end ) {
			
			const uint16* chars = *string;
			
			word->newline = 0;
			word->count = 1;
			uint16 unicode = chars[begin];
			
			if ( has_space_char(unicode, true, true) ) {
				if ( opts.merge_space || opts.merge_line_feed ) {
					
					if ( unicode == 0x0A ) {
						if ( !opts.merge_line_feed ) { // 不合并换行
							word->newline = 1; goto end;
						}
					} else {
						if ( !opts.merge_space ) { // 不合并空格
							 goto common;
						}
					}
					
					uint count = 0;
					do {
						count++; begin++;
						unicode = chars[begin];
					} while ( begin < end && has_space_char(unicode, opts.merge_space, opts.merge_line_feed) );
					
					word->count = count; unicode = ' ';
				} else {
					if ( unicode == 0x0A ) {
						word->newline = 1; goto end;
					}
				}
				
				goto common;
				
			} else if ( has_english_char(unicode) ) {
				uint count = 0;
				float offset = offset_start;
				do {
					offset += table->glyph(unicode)->hori_advance() / ratio; // 字符宽度
					word->offset.set(count, offset);
					word->chars.set(count, unicode);
					count++; begin++;
					unicode = chars[begin];
				} while ( begin < end && has_english_char(unicode) );
				
				word->count = count;
				word->width = offset - offset_start;
			} else {
				
			common:
				float hori_advance = table->glyph(unicode)->hori_advance() / ratio; // 字符宽度
				word->offset.set(0, offset_start + hori_advance);
				word->chars.set(0, unicode);
				word->width = hori_advance;
			}
			
		end:
			
			return true;
		}
		
		return false;
	}
	
	/**
	 * @func clip_string
	 */
	template<int ellipsis>
	void clip_string(Word& word, Cell& cell, Vec2 limit, FontGlyphTable* table, float ratio) {
		
		float hori_advance = table->glyph('.')->hori_advance() / ratio; // 字符宽度
		cell.offset.write( word.offset, -1, word.count );
		cell.chars.write( word.chars, -1, word.count );
		
		uint ellipsis_count = ellipsis ? 3 : 0;
		
		while ( cell.chars.length() || ellipsis_count ) {
			if ( cell.offset[cell.chars.length()] + ellipsis_count * hori_advance <= limit.width() ) {
				break;
			} else {
				if ( cell.chars.length() ) {
					cell.offset.pop();
					cell.chars.pop();
				} else {
					ellipsis_count--;
				}
			}
		}
		
		if ( ellipsis_count ) {
			float offset = cell.offset[cell.chars.length()] + hori_advance;
			for ( uint i = 0; i < ellipsis_count; i++ ) {
				cell.offset.push(offset + i * hori_advance);
				cell.chars.push('.');
			}
		}
		// cell.line_num
	}
	
	/**
	 * @func set_text_layout_offset
	 */
	template<bool clip, int ellipsis, bool auto_wrap>
	void set_text_layout_offset(TextRows* rows, Vec2 limit, Options opts,
															Data& data, cUcs2String& string,
															uint begin, uint end, bool ignore_empty_cell) {
		// 在这里进行文本排版布局
		FontGlyphTable* table = get_font_glyph_table_and_height(data, opts.text_line_height);
		float ratio = 4096.0 / m_text_size.value; /* 64.0 * 64.0 = 4096.0 */
		float line_height = data.text_ascender + data.text_descender;
		
		Vec2* offset_end = &rows->last()->offset_end;
		Word word;
		
		// 查找第一个单词
		if ( !read_word(&word, offset_end->x(), table, ratio, opts.space_wrap, string, begin, end) ) {
			return;
		}
		
		Cell cell = {
			rows->last_num(), 0, 0, begin,
		};
		cell.offset.push(offset_end->x());
		
		// 先检查第一个单词,如果第一个单词与上一个视图在同一行,需要更新行高与基线位置
		if ( !word.newline ) {  // not newline
			/* 行开始 || !自动换行 || 第一个单词宽度 <= 行剩余宽度 */
			if (offset_end->x() == 0 || !auto_wrap || word.width <= limit.width() - offset_end->x() ) {
				// 当前行可以放下该单词，所以更新这一行的行高
				rows->update_row(data.text_ascender, data.text_descender);
			}
		}
		
		do {
			
			if ( word.newline ) { // \n 换行
				if (clip) { // 多行溢出修剪...
					if ( offset_end->y() + line_height > limit.height() ) { // clip
						break; // exit loop
					}
				}
				
				if ( ignore_empty_cell ) {
					new_row(rows, cell, data, begin + 1); // new row
				}
				else {
					Cell line_feed = {
						rows->last_num(), rows->last()->baseline, 0, begin,
					};
					line_feed.offset.push(offset_end->x());
					
					new_row(rows, cell, data, begin + 1); // new row
					
					// add line feed
					data.cells.push(line_feed);
				}
				
				offset_end = &rows->last()->offset_end;
			} else {
				
				if ( auto_wrap ) { // auto wrap
					
					if ( word.width + offset_end->x() > limit.width() ) { // 当前行无法排列单词
						// 如果当前行的x偏移为0,表示这一行还没有任何内容,所以无需换行
						if ( offset_end->x() > 0 ) {
							
							// 多行溢出修剪...
							if (clip) {
								if ( offset_end->y() + line_height > limit.height() ) { // clip
									rows->mark_clip();
									clip_string<ellipsis>(word, cell, limit, table, ratio); break; // exit loop
								}
							}
							
							float offset = offset_end->x();
							for ( uint i = 0; i < word.count; i++ ) {
								word.offset[i] -= offset; // 新行重置单词偏移量
							}
							new_row(rows, cell, data, begin);
							
							offset_end = &rows->last()->offset_end;
						}
					}
					
				} else {
					if (clip) { // 单行溢出修剪...
						if ( word.width + offset_end->x() > limit.width() ) { // 当前行无法排列单词
							rows->mark_clip();
							clip_string<ellipsis>(word, cell, limit, table, ratio); break; // exit loop
						}
					}
				}
				
				cell.offset.write( word.offset, -1, word.count );
				cell.chars.write( word.chars, -1, word.count );
				offset_end->x(offset_end->x() + word.width);  // 更新Text行偏移
			}
			
			begin += word.count;
			
		} while ( read_word(&word, offset_end->x(), table, ratio, opts.space_wrap, string, begin, end) );
		
		if ( cell.chars.length() ) {
			cell.baseline = rows->last()->baseline;
			offset_end->x(cell.offset[cell.chars.length()]); // 更新Text行偏移
			data.cells.push(move(cell));
		}
	}
	
};

TextLayout::TextLayout(): TextFont()
, m_text_overflow({ TextAttrType::INHERIT })
, m_text_white_space({ TextAttrType::INHERIT }) {
	
}

TextLayout::Options TextLayout::get_options(TextLayout* hybrid) {
	Options opts;
	
	switch (hybrid ? hybrid->m_text_white_space.value : m_text_white_space.value) {
		default:                            opts.space_wrap = { true, false, false }; break;
		case TextWhiteSpaceEnum::NO_WRAP:   opts.space_wrap = { false, true, true }; break;
		case TextWhiteSpaceEnum::NO_SPACE:  opts.space_wrap = { true, true, true }; break;
		case TextWhiteSpaceEnum::PRE:       opts.space_wrap = { false, false, false }; break;
		case TextWhiteSpaceEnum::PRE_LINE:  opts.space_wrap = { true, true, false }; break;
	}
	
	opts.overflow = m_text_overflow.value;
	opts.text_line_height = m_text_line_height.value;
	
	return opts;
}

/**
 * @func is_auto_wrap
 */
bool TextLayout::is_auto_wrap(TextLayout* text) {
	if ( text ) {
		switch (text->m_text_white_space.value) {
			case TextWhiteSpaceEnum::NO_WRAP:
			case TextWhiteSpaceEnum::PRE: return false;
			default:return true;
		}
	}
	return false;
}

/**
 * @func set_text_layout_offset
 */
void TextLayout::set_text_layout_offset(TextRows* rows, Vec2 limit,
																				Data& data, cUcs2String& string,
																				uint begin, uint end,
																				Options* options, bool ignore_empty_cell) {
	
	Options opts = options ? *options : get_options();
	
	// ...
	// 2.单行省略 auto_wrap = false, merge_line_feed = true NO_WRAP
	// 1.多行省略 auto_wrap = true NORMAL|NO_SPACE|PRE_LINE
	
	if ( opts.space_wrap.auto_wrap || opts.space_wrap.merge_line_feed ) { // 允许多行修剪 or 允许单行修剪
		if ( opts.space_wrap.auto_wrap ) {
			switch ( m_text_overflow.value ) {
				case TextOverflowEnum::NORMAL:
					_inl(this)->set_text_layout_offset<0, 0, 1>(rows, limit, opts, data,
																											string, begin, end, ignore_empty_cell);
					break;
				case TextOverflowEnum::CLIP:
					_inl(this)->set_text_layout_offset<1, 0, 1>(rows, limit, opts, data,
																											string, begin, end, ignore_empty_cell);
					break;
				case TextOverflowEnum::ELLIPSIS:
					_inl(this)->set_text_layout_offset<1, 1, 1>(rows, limit, opts, data,
																											string, begin, end, ignore_empty_cell);
					break;
				case TextOverflowEnum::CENTER_ELLIPSIS:
					_inl(this)->set_text_layout_offset<1, 2, 1>(rows, limit, opts, data,
																											string, begin, end, ignore_empty_cell);
					break;
			}
		} else {
			switch ( m_text_overflow.value ) {
				case TextOverflowEnum::NORMAL:
					_inl(this)->set_text_layout_offset<0, 0, 0>(rows, limit, opts, data,
																											string, begin, end, ignore_empty_cell);
					break;
				case TextOverflowEnum::CLIP:
					_inl(this)->set_text_layout_offset<1, 0, 0>(rows, limit, opts, data,
																											string, begin, end, ignore_empty_cell);
					break;
				case TextOverflowEnum::ELLIPSIS:
					_inl(this)->set_text_layout_offset<1, 1, 0>(rows, limit, opts, data,
																											string, begin, end, ignore_empty_cell);
					break;
				case TextOverflowEnum::CENTER_ELLIPSIS:
					_inl(this)->set_text_layout_offset<1, 2, 0>(rows, limit, opts, data,
																											string, begin, end, ignore_empty_cell);
					break;
			}
		}
	} else {
		_inl(this)->set_text_layout_offset<0, 0, 0>(rows, limit, opts, data,
																								string, begin, end, ignore_empty_cell);
	}
}

void TextLayout::set_text_layout_offset(TextRows* rows, Vec2 limit,
																				Data& data, uint16 security, uint count, Options* opts) {
	Ucs2String string;
	for ( uint i = 0; i < count; i++ ) {
		string.push(&security, 1);
	}
	set_text_layout_offset(rows, limit, data, string, 0, count, opts, true);
}

void TextLayout::set_text_overflow(TextOverflow value) {
	equal(m_text_overflow) {
		m_text_overflow = value;
		mark_text(View::M_LAYOUT | View::M_CONTENT_OFFSET | View::M_TEXT_FONT);
	}
}

void TextLayout::set_text_white_space(TextWhiteSpace value) {
	equal(m_text_white_space) {
		m_text_white_space = value;
		mark_text(View::M_LAYOUT | View::M_CONTENT_OFFSET | View::M_TEXT_FONT);
	}
}

/**
 * @func mark_text
 */
void TextLayout::mark_text(uint value) {
	view()->mark_pre(value);
}

/**
 * @func solve_text_layout_mark
 */
void TextLayout::solve_text_layout_mark() {
		
	TextLayout* parent = view()->parent()->as_text_layout();
	
	if (parent) {
		if (m_text_background_color.type == TextAttrType::INHERIT) {
			m_text_background_color.value = parent->m_text_background_color.value;
		}
		if (m_text_color.type == TextAttrType::INHERIT) {
			m_text_color.value = parent->m_text_color.value;
		}
		if (m_text_size.type == TextAttrType::INHERIT) {
			m_text_size.value = parent->m_text_size.value;
		}
		if (m_text_style.type == TextAttrType::INHERIT) {
			m_text_style.value = parent->m_text_style.value;
		}
		if (m_text_family.type == TextAttrType::INHERIT) {
			m_text_family.value = parent->m_text_family.value;
		}
		if (m_text_line_height.type == TextAttrType::INHERIT) {
			m_text_line_height.value = parent->m_text_line_height.value;
		}
		if (m_text_shadow.type == TextAttrType::INHERIT) {
			m_text_shadow.value = parent->m_text_shadow.value;
		}
		if (m_text_decoration.type == TextAttrType::INHERIT) {
			m_text_decoration.value = parent->m_text_decoration.value;
		}
		if (m_text_overflow.type == TextAttrType::INHERIT) {
			m_text_overflow.value = parent->m_text_overflow.value;
		}
		if (m_text_white_space.type == TextAttrType::INHERIT) {
			m_text_white_space.value = parent->m_text_white_space.value;
		}
	} else { // 没有父视图使用全局默认值
		if (m_text_background_color.type == TextAttrType::INHERIT) {
			m_text_background_color.value = app()->default_text_background_color().value;
		}
		if (m_text_color.type == TextAttrType::INHERIT) {
			m_text_color.value = app()->default_text_color().value;
		}
		if (m_text_size.type == TextAttrType::INHERIT) {
			m_text_size.value = app()->default_text_size().value;
		}
		if (m_text_style.type == TextAttrType::INHERIT) {
			m_text_style.value = app()->default_text_style().value;
		}
		if (m_text_family.type == TextAttrType::INHERIT) {
			m_text_family.value = app()->default_text_family().value;
		}
		if (m_text_line_height.type == TextAttrType::INHERIT) {
			m_text_line_height.value = app()->default_text_line_height().value;
		}
		if (m_text_shadow.type == TextAttrType::INHERIT) {
			m_text_shadow.value = app()->default_text_shadow().value;
		}
		if (m_text_decoration.type == TextAttrType::INHERIT) {
			m_text_decoration.value = app()->default_text_decoration().value;
		}
		if (m_text_overflow.type == TextAttrType::INHERIT) {
			m_text_overflow.value = app()->default_text_overflow().value;
		}
		if (m_text_white_space.type == TextAttrType::INHERIT) {
			m_text_white_space.value = app()->default_text_white_space().value;
		}
	}
	
	View* v = view()->first();
	
	while (v) {
		TextLayout* text = v->as_text_layout();
		if (text) {
			_inl(text)->solve_text_layout_mark(this);
		}
		v = v->next();
	}
	view()->mark_value &= (~View::M_TEXT_FONT); // 删除这些标记
}

XX_END
