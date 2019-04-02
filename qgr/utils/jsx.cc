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
 * THIS.SOFTWARE IS.PROVIDED BY THE COPYRIGHT HOLDERS.AND CONTRIBUTORS."AS.IS" AND
 * ANY EXPRESS.OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES.OF MERCHANTABILITY AND FITNESS.FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS.OR SERVICES;
 * LOSS.OF USE, DATA, OR PROFITS; OR BUSINESS.INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "qgr/utils/jsx.h"
#include "qgr/utils/string.h"
#include "qgr/utils/string-builder.h"
#include "qgr/utils/map.h"
#include "qgr/utils/fs.h"

XX_NS(qgr)

#define UNEXPECTED_TOKEN_ERROR() error()

#define ASSERT_NEXT(tok, ...) \
if(next() != tok) error(__VA_ARGS__)

#define ASSERT_TOKEN(tok, ...)         \
if(_scanner->token() != tok) error(__VA_ARGS__)

#define ASSERT_PEEK(tok, ...)          \
if(_scanner->peek() != tok) error(__VA_ARGS__)

#define ASSERT(con, ...) if(!con) error(__VA_ARGS__)

#define DEF_STATIC_STR_LIST(F) \
	F(SPACE, ' ') \
	F(INDENT, "  ") \
	F(LT, '<') \
	F(GT, '>') \
	F(ADD, '+') \
	F(SUB, '-') \
	F(DIV, '/') \
	F(ASSIGN, '=') \
	F(INC, "++") \
	F(DEC, "--") \
	F(ASSIGN_BIT_OR, "|=") \
	F(ASSIGN_BIT_XOR, "^=") \
	F(ASSIGN_BIT_AND, "&=") \
	F(ASSIGN_SHL, "<<=") \
	F(ASSIGN_SAR, ">>=") \
	F(ASSIGN_SHR, ">>>=") \
	F(ASSIGN_ADD, "+=") \
	F(ASSIGN_SUB, "-=") \
	F(ASSIGN_MUL, "*=") \
	F(ASSIGN_POWER, "**=") \
	F(ASSIGN_DIV, "/=") \
	F(ASSIGN_MOD, "%=") \
	F(OR, "||") \
	F(AND, "&&") \
	F(SHL, "<<") \
	F(SAR, ">>") \
	F(SHR, ">>>") \
	F(EQ, "==") \
	F(NE, "!=") \
	F(EQ_STRICT, "===") \
	F(NE_STRICT, "!==") \
	F(LTE, "<=") \
	F(GTE, ">=") \
	F(PERIOD, '.') \
	F(COMMAND, "${") \
	F(LBRACE, '{') \
	F(RBRACE, '}') \
	F(LBRACK, '[') \
	F(RBRACK, ']') \
	F(LPAREN, '(') \
	F(RPAREN, ')') \
	F(CONDITIONAL, '?') \
	F(NOT, '!') \
	F(BIT_OR, '|') \
	F(BIT_NOT, '~') \
	F(BIT_XOR, '^') \
	F(MUL, '*') \
	F(POWER, "**") \
	F(BIT_AND, '&') \
	F(MOD, '%') \
	F(AT, '@') \
	F(QUOTES, '"') \
	F(NEWLINE, '\n') \
	F(CONST, "const"); /*const*/ \
	F(VAR, "var");     /*var*/ \
	F(REQUIRE, "require");   /*require*/ \
	F(COMMA, ',') \
	F(COLON, ':') \
	F(SEMICOLON, ';') \
	F(VX, "vx")       /*v*/ \
	F(__VX, "__vx") \
	F(VALUE, "value") \
	F(DATA_BIND_FUNC, "($,ctr)=>{return") /* ($,ctr)=>{return*/ \
	F(XML_COMMENT, "/***") \
	F(XML_COMMENT_END, "**/") \
	F(COMMENT, "/*") \
	F(COMMENT_END, "*/") \
	F(EXPORT_COMMENT, "/*export*/") \
	F(EXPORTS, "exports") \
	F(EXPORT_DEFAULT, "exports.default") \
	F(MODULE_EXPORT, "module._export") \
	F(DEFAULT, "default") \
	F(__EXTEND, "__extend") \
	F(PROTOTYPE, "prototype") \
	F(NUMBER_0, "0") \
	F(NUMBER_1, "1") \
	F(NUMBER_2, "2") \
	F(NUMBER_3, "3") \
	F(VDATA, "vdata") \
	F(ATTRS_COMMENT, "/*attrs*/") \
	F(CHILDS_COMMENT, "/*childs*/") \
	F(TYPE, "vx") \
	F(VALUE2, "v") \
	F(MULTIPLE, "m") \
	F(STATIC, "static") \

struct static_str_list_t {
#define F(N,V) Ucs2String N = V;
DEF_STATIC_STR_LIST(F)
#undef F
} static *static_str_list = nullptr;

#define S (*static_str_list)

//LF
static inline bool is_carriage_return(int c) {
	return c == 0x000D;
}

//CR
static inline bool is_line_feed(int c) {
	return c == 0x000A;
}

static inline bool is_line_terminator(int c) {
	return c == 0x000A || c == 0x000D;
}

// If c is in 'A'-'Z' or 'a'-'z', return its lower-case.
// Else, return something outside of 'A'-'Z' and 'a'-'z'.
// Note: it ignores LOCALE.
static inline int ascii_alpha_to_lower(int c) {
	return c | 0x20;
}

static inline bool is_in_range(int value, int lower_limit, int higher_limit) {
	XX_ASSERT(lower_limit <= higher_limit);
	return (uint)(value - lower_limit) <= (uint)(higher_limit - lower_limit);
}

static inline bool is_decimal_digit(int c) {
	// ECMA-262, 3rd, 7.8.3 (p 16)
	return is_in_range(c, '0', '9');
}

static inline bool is_hex_digit(int c) {
	// ECMA-262, 3rd, 7.6 (p 15)
	return is_decimal_digit(c) || is_in_range(ascii_alpha_to_lower(c), 'a', 'f');
}

static inline bool is_int(int64 i) {
	return !(1 << 31 & i);
}

static inline int char_to_number(int c) {
	return c - '0';
}

// Returns the value (0 .. 15) of a hexadecimal character c.
// If c is not a legal hexadecimal character, returns a value < 0.
static inline int hex_char_to_number(int c) {
	if(is_decimal_digit(c)){
		return char_to_number(c);
	}
	return 10 + ascii_alpha_to_lower(c) - 'a';
}

static inline bool is_octal_digit(int c) {
	// ECMA-262, 6th, 7.8.3
	return is_in_range(c, '0', '7');
}

static inline bool is_binary_digit(int c) {
	// ECMA-262, 6th, 7.8.3
	return c == '0' || c == '1';
}

static inline bool is_identifier_start(int c) {
	return is_in_range(ascii_alpha_to_lower(c), 'a', 'z') || c == '_' || c == '$';
}

static inline bool is_xml_element_start(int c) {
	return is_identifier_start(c);
}

static inline bool is_identifier_part(int c) {
	return is_identifier_start(c) || is_decimal_digit(c);
}

static inline int64 int64_multiplication(int64 i, int multiple, int add) {
	
	double f = 1.0 * i / Int64::max;
	
	if (f * multiple > 1) { // 溢出
		return -1;
	}
	else {
		return i * multiple + add;
	}
}

enum Token {
	/* End of source indicator. */
	EOS,                    // eos
	SHELL_HEADER,           // !#/bin/sh
	AT,                     // @
	/* Xml */
	XML_ELEMENT_TAG,        // <xml
	XML_ELEMENT_TAG_END,    // </xml>
	XML_NO_IGNORE_SPACE,    // @@
	XML_COMMENT,            // <!-- comment -->
	COMMAND,                // `str${
	COMMAND_DATA_BIND,      // `str%%{
	COMMAND_DATA_BIND_ONCE, // `str%{
	COMMAND_END,            // str`
	/* Punctuators. */
	LPAREN,                 // (
	RPAREN,                 // )
	LBRACK,                 // [
	RBRACK,                 // ]
	LBRACE,                 // {
	RBRACE,                 // }
	COLON,                  // :
	SEMICOLON,              // ;
	COMMA,                  // ,
	CONDITIONAL,            // ?
	PERIOD,                 // .
	INC,                    // ++
	DEC,                    // --
	/* Assignment operators. */
	ASSIGN,                 // =
	ASSIGN_BIT_OR,          // |=
	ASSIGN_BIT_XOR,         // ^=
	ASSIGN_BIT_AND,         // &=
	ASSIGN_SHL,             // <<=
	ASSIGN_SAR,             // >>=
	ASSIGN_SHR,             // >>>=
	ASSIGN_ADD,             // +=
	ASSIGN_SUB,             // -=
	ASSIGN_MUL,             // *=
	ASSIGN_POWER,           // **=
	ASSIGN_DIV,             // /=
	ASSIGN_MOD,             // %=
	/* Binary operators sorted by precedence. */
	OR,                     // ||
	AND,                    // &&
	BIT_OR,                 // |
	BIT_XOR,                // ^
	BIT_AND,                // &
	SHL,                    // <<
	SAR,                    // >>
	SHR,                    // >>>
	ADD,                    // +
	SUB,                    // -
	MUL,                    // *
	POWER,                  // **
	DIV,                    // /
	MOD,                    // %
	/* Compare operators sorted by precedence. */
	EQ,                     // ==
	NE,                     // !=
	EQ_STRICT,              // ===
	NE_STRICT,              // !==
	LT,                     // <
	GT,                     // >
	LTE,                    // <=
	GTE,                    // >=
	INSTANCEOF,             // instanceof
	IN,                     // in
	/* Unary operators. */
	NOT,                    // !
	BIT_NOT,                // ~
	TYPEOF,                 // typeof
	/*  Literal */
	NUMBER_LITERAL,         // number
	STRIXX_LITERAL,         // string
	REGEXP_LITERAL,         // regexp
	IDENTIFIER,             // identifier
	/* KEYWORD */
	AS,                     // as
	ASYNC,                  // async
	EXPORT,                 // export
	FROM,                   // from
	IMPORT,                 // import
	IF,                     // if
	OF,                     // of
	RETURN,                 // return
	VAR,                    // var
	CLASS,                  // class
	FUNCTION,               // function
	LET,                    // let
	DEFAULT,                // default
	CONST,                  // const
	EXTENDS,                // extends
	EVENT,                  // event
	ELSE,                   // else
	GET,                    // get
	SET,                    // set
	STATIC,                 // static
	/* Illegal token - not able to scan. */
	ILLEGAL,                // illegal
	/* Scanner-internal use only. */
	WHITESPACE,             // space
};

inline static bool isAssignmentOp(Token tok) {
	return ASSIGN <= tok && tok <= ASSIGN_MOD;
}

inline static bool isBinaryOrCompareOp(Token op){
	return OR <= op && op <= IN;
}

/**
 * @class Scanner jsx 简易词法分析器
 */
class Scanner : public Object {
 public:
	
	Scanner(const uint16* code, uint size, bool clean_comment)
	: code_(code)
	, size_(size)
	, pos_(0)
	, line_(0)
	, current_(new TokenDesc())
	, next_(new TokenDesc()), clean_comment_(clean_comment)
	{ //
		c0_ = size_ == 0 ? -1: *code_;
		strip_bom();
		current_->token = ILLEGAL;
		
		// skip shell header
		// #!/bin/sh
		if ( c0_ == '#' ) {
			advance();
			if ( c0_ == '!' ) {
				next_->token = SHELL_HEADER;
				next_->string_value.push('#');
				do {
					next_->string_value.push(c0_);
					advance();
				} while(c0_ != '\n' && c0_ != EOS);
				next_->location.beg_pos = 0;
				next_->location.end_pos = pos_;
				return;
			}
		}
		
		scan();
	}
	
	virtual ~Scanner() {
		delete current_;
		delete next_;
	}
	
	struct Location {
		uint beg_pos;
		uint end_pos;
		uint line;
	};
	
	void skip_shell_header() {
		
	}
	
	Token next() {
		
		// Table of one-character tokens, by character (0x00..0x7f only).
		static cbyte one_char_tokens[] = {
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::LPAREN,       // (
			Token::RPAREN,       // )
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::COMMA,        // ,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::COLON,         // :
			Token::SEMICOLON,     // ;
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::CONDITIONAL,   // ?
			Token::AT,            // @
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::LBRACK,      // [
			Token::ILLEGAL,
			Token::RBRACK,      // ]
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::ILLEGAL,
			Token::LBRACE,        // {
			Token::ILLEGAL,
			Token::RBRACE,        // }
			Token::BIT_NOT,       // ~
			Token::ILLEGAL
		};
		
		TokenDesc* tok = current_;
		prev_ = tok->token;
		current_ = next_;
		next_ = tok;
		next_->location.beg_pos = pos_;
		next_->location.line = line_;
		next_->string_space = next_->string_value = Ucs2String();
		next_->before_line_feed = false;
		
		if ((uint)c0_ <= 0x7f) {
			Token token = (Token)one_char_tokens[c0_];
			if (token != ILLEGAL) {
				next_->token = token;
				next_->location.end_pos = pos_ + 1;
				advance();
				return current_->token;
			}
		}
		scan();
		return current_->token;
	}
	
	Token scan_xml_content(bool ignore_space, uint pos) {
		if ( !set_pos(pos) ) {
			next_->location = { pos_, pos_, line_ };
			next_->token = ILLEGAL;
			return ILLEGAL;
		}
		
		next_->location.beg_pos = pos_;
		next_->location.line = line_;
		next_->string_value = next_->string_space = Ucs2String();
		
		Token token = ILLEGAL;
		
		if (c0_ == '<') {
			// <!-- xml comment --> or <xml or </xml> or < <= << <<=
			token = scan_lt_and_xml_element();
		}
		else if (c0_ == '$') {  // command block
			//  ${ command }
			advance();
			if (c0_ == '{') {                       // COMMAND
				token = COMMAND;                      //
			} else {
				token = STRIXX_LITERAL;
				next_->string_value.push('$');
			}
		}
		else if (c0_ == '%') { // Data bind command block
			// %{ command } %%{ command }
			advance();
			if ( c0_ == '{' ) { // COMMAND_DATA_BIND_ONCE
				token = COMMAND_DATA_BIND_ONCE;
			} else if ( c0_ == '%' ) {
				int c = c0_;
				advance();
				if ( c0_ == '{' ) { // COMMAND_DATA_BIND
					token = COMMAND_DATA_BIND;
				} else {
					token = STRIXX_LITERAL;
					next_->string_value.push('%');
					next_->string_value.push(c);
				}
			}
			else {
				token = STRIXX_LITERAL;
				next_->string_value.push('%');
			}
		}
		else if (c0_ == '@') { // xml文本转义,不忽略内容文本中的空格以及制表符
			advance();
			if (c0_ == '@') {
				advance();
				token = XML_NO_IGNORE_SPACE;  //
			} else {
				token = STRIXX_LITERAL;
				next_->string_value.push('@');
			}
		}
		else {
			token = scan_xml_content_string(ignore_space);
		}
		next_->location.end_pos = pos_;
		next_->token = token;
		return token;
	}
	
	Token scan_regexp(uint pos) {
		if ( !set_pos(pos) ) {
			next_->location = { pos_, pos_, line_ };
			next_->token = ILLEGAL;
			return ILLEGAL;
		}
		
		Token token = REGEXP_LITERAL;
		next_->location.beg_pos = pos_;
		next_->location.line = line_;
		next_->string_value = next_->string_space = Ucs2String();
		
		XX_ASSERT(c0_ == '/');
		
		advance();
		if (c0_ == '/' || c0_ == '*') { // ILLEGAL
			token = ILLEGAL;
		} else {
			next_->string_value.push('/');
			
			bool is_LBRACK = false; // [
			
			do {
				next_->string_value.push(c0_);
				if ( c0_ == '\\' ) { // 正则转义符
					advance();
					if (c0_ < 0) break;
					next_->string_value.push(c0_);
				} else if ( c0_ == '[' ) {
					is_LBRACK = true;
				} else if ( c0_ == ']' ) {
					is_LBRACK = false;
				}
				advance();
			} while( c0_ >= 0  && (is_LBRACK || c0_ != '/') && !is_line_terminator(c0_) );
			
			if (c0_ == '/') {
				next_->string_value.push('/');
				
				int i = 0, m = 0, g = 0, y = 0, u = 0;
				
				while (true) { // regexp flags
					advance();
					if (c0_ == 'i') {
						if (i) break; else { i = 1; next_->string_value.push('i'); }
					} else if (c0_ == 'm') {
						if (m) break; else { m = 1; next_->string_value.push('m'); }
					} else if (c0_ == 'g') {
						if (g) break; else { g = 1; next_->string_value.push('g'); }
					} else if (c0_ == 'y') {
						if (y) break; else { y = 1; next_->string_value.push('y'); }
					} else if (c0_ == 'u') {
						if (u) break; else { u = 1; next_->string_value.push('u'); }
					} else {
						break;
					}
				}
			} else {
				token = ILLEGAL;
			}
		}
		
		next_->location.end_pos = pos_;
		next_->token = token;
		return token;
	}
	
	Token scan_command_string(uint pos) {
		if ( ! set_pos(pos) ) {
			next_->location = { pos_, pos_, line_ };
			next_->token = ILLEGAL;
			return ILLEGAL;
		}
		
		Token token = COMMAND_END;
		next_->location.beg_pos = pos_;
		next_->location.line = line_;
		
		while ( c0_ != '`' && c0_ >= 0 /*&& !is_line_terminator(c0_)*/ ) {
			int c = c0_;
			advance();
			if (c == '\\') { // 转义符
				if (c0_ < 0 || !scan_string_escape()) {
					token = ILLEGAL; break;
				}
				continue;
			} else if (c == '$') { // js字符串内部指令开始必须以}结束
				// ${ command }
				if (c0_ == '{') {         // COMMAND
					token = COMMAND; break;
				}
			}
			else if ( is_line_terminator(c) ) { // \n
				next_->string_space.push(c);
			}
			next_->string_value.push(c);
		}
		if (c0_ == '`') {
			advance();  // consume quote
			next_->string_value.push('`');
		} else if (c0_ != '{') { // err
			token = ILLEGAL;
		}
		
		next_->location.end_pos = pos_;
		next_->token = token;
		return token;
	}
	
	inline Token        token()             { return current_->token; }
	inline Location     location()          { return current_->location; }
	inline Ucs2String&  string_space()      { return current_->string_space; }
	inline Ucs2String&  string_value()      { return current_->string_value; }
	inline bool         before_line_feed()  { return current_->before_line_feed; }
	inline Token        prev()              { return prev_; }
	inline Token        peek()              { return next_->token; }
	inline Location     next_location()     { return next_->location; }
	inline Ucs2String&  next_string_space() { return next_->string_space; }
	inline Ucs2String&  next_string_value() { return next_->string_value; }
	inline bool         next_before_line_feed() { return next_->before_line_feed; }
	inline bool         has_scape_before()  { return !current_->string_space.is_empty(); }
	inline bool         has_scape_before_next() { return !next_->string_space.is_empty(); }
	
 private:

	struct TokenDesc {
		Token token;
		Location location;
		Ucs2String string_space;
		Ucs2String string_value;
		bool  before_line_feed;
	};
	
	void scan() { // scan javascript code
		
		Token token;
		next_->string_value = next_->string_space = Ucs2String();
		next_->before_line_feed = false;
		
		do {
			// Remember the position of the next token
			next_->location.beg_pos = pos_;
			next_->location.line = line_;
			
			switch (c0_) {
				case ' ':
				case '\t':
					next_->string_space.push(c0_);
					advance();
					token = Token::WHITESPACE;
					break;
					
				case '\n':
					next_->string_space.push(c0_);
					advance();
					next_->before_line_feed = true;
					token = WHITESPACE;
					break;
					
				case '"':
				case '\'':
					token = scan_string();
					break;
					
				case '`': // 检查指令字符串内部是否有 ${
					advance();
					next_->string_value.push('`');
					token = scan_command_string(pos_);
					if ( token == COMMAND_END ) {
						token = STRIXX_LITERAL;
					}
					break;
					
				case '$':
					advance();
					if (c0_ == '{') {                       // COMMAND
						// ${ command }
						token = COMMAND;                      //
					} else {
						back(); token = scan_identifier();
					}
					break;
					
				case '%':
					// % %= %{ %%{
					advance();
					if ( c0_ == '{' ) {         // %{ COMMAND_DATA_BIND_ONCE
						// %{ command }
						token = COMMAND_DATA_BIND_ONCE;
					} else if ( c0_ == '%' ) { // %%
						advance();
						if ( c0_ == '{' ) {      // %%{ COMMAND_DATA_BIND
							// %%{ command }
							token = COMMAND_DATA_BIND;
						} else {
							back();
							goto mod;
						}
					} else {
						// % %=
					mod:
						token = select('=', ASSIGN_MOD, MOD);
					}
					break;
					
				case '<':
					// <!-- xml comment --> or <xml or </xml> or < <= << <<=
					token = scan_lt_and_xml_element();
					break;
					
				case '>':
					// > >= >> >>= >>> >>>=
					advance();
					if (c0_ == '=') { // >=
						token = select(GTE);
					} else if (c0_ == '>') {
						// >> >>= >>> >>>=
						advance();
						if (c0_ == '=') { // >>=
							token = select(ASSIGN_SAR);
						} else if (c0_ == '>') { // >>>=
							token = select('=', ASSIGN_SHR, SHR);
						} else { // >>
							token = SAR;
						}
					} else { // >
						token = GT;
					}
					break;
					
				case '=':
					// = == ===
					advance();
					if (c0_ == '=') {
						token = select('=', EQ_STRICT, EQ);
					} else {
						token = ASSIGN;
					}
					break;
					
				case '!':
					// ! != !==
					advance();
					if (c0_ == '=') {
						token = select('=', NE_STRICT, NE);
					} else {
						token = NOT;
					}
					break;
					
				case '+':
					// + ++ +=
					advance();
					if (c0_ == '+') {
						token = select(INC);
					} else if (c0_ == '=') {
						token = select(ASSIGN_ADD);
					} else {
						token = ADD;
					}
					break;
					
				case '-':
					// - -- -=
					advance();
					if (c0_ == '-') {
						advance();
						token = DEC;
					} else if (c0_ == '=') {
						token = select(ASSIGN_SUB);
					} else {
						token = SUB;
					}
					break;
					
				case '*':
					// * *= ** **=
					advance();
					if (c0_ == '*') { // ** **=
						token = select('=', ASSIGN_POWER, POWER);
					} else if (c0_ == '=') { // *=
						token = select(ASSIGN_MUL);
					} else { // *
						token = MUL;
					}
					break;

				case '/':
					// /  // /* /=
					advance();
					if (c0_ == '/') { // js 单行注解
						token = skip_single_line_comment();
					} else if (c0_ == '*') { // js 多行注解
						token = skip_multi_line_comment();
					} else if (c0_ == '=') {
						token = select(ASSIGN_DIV);
					} else {
						token = DIV;
					}
					break;
					
				case '&':
					// & && &=
					advance();
					if (c0_ == '&') {
						token = select(AND);
					} else if (c0_ == '=') {
						token = select(ASSIGN_BIT_AND);
					} else {
						token = BIT_AND;
					}
					break;
					
				case '|':
					// | || |=
					advance();
					if (c0_ == '|') {
						token = select(OR);
					} else if (c0_ == '=') {
						token = select(ASSIGN_BIT_OR);
					} else {
						token = BIT_OR;
					}
					break;
					
				case '^':
					// ^ ^=
					token = select('=', ASSIGN_BIT_XOR, BIT_XOR);
					break;
					
				case '.': // . Number
					advance();
					if (is_decimal_digit(c0_)) {
						token = scan_number(true);
					} else {
						token = PERIOD;
					}
					break;
					
				case ':':
					token = select(COLON);
					break;
					
				case ';':
					token = select(SEMICOLON);
					break;
					
				case ',':
					token = select(COMMA);
					break;
					
				case '(':
					token = select(LPAREN);
					break;
					
				case ')':
					token = select(RPAREN);
					break;
					
				case '[':
					token = select(LBRACK);
					break;
					
				case ']':
					token = select(RBRACK);
					break;
					
				case '{':
					token = select(LBRACE);
					break;
					
				case '}':
					token = select(RBRACE);
					break;
					
				case '?':
					token = select(CONDITIONAL);
					break;
					
				case '~':
					token = select(BIT_NOT);
					break;
					
				case '@':
					token = select(AT);
					break;
					
				default:
					if (is_identifier_start(c0_)) {
						token = scan_identifier();
					} else if (is_decimal_digit(c0_)) {
						token = scan_number(false);
					} else if (skip_white_space()) {
						token = WHITESPACE;
					} else if (c0_ < 0) {
						token = EOS;
					} else {
						token = select(ILLEGAL);
					}
					break;
			}
			// Continue scanning for tokens as long as we're just skipping
			// whitespace.
		} while(token == WHITESPACE);
		
		next_->location.end_pos = pos_;
		next_->token = token;
	}
	
	Token scan_identifier() {
		
		// Scan the rest of the identifier characters.
		do {
			next_->string_value.push(c0_);
			advance();
		} while(is_identifier_part(c0_));
		
		const int input_length = next_->string_value.length();
		const uint16* input = next_->string_value.c();
		const int kMinLength = 2;
		const int kMaxLength = 10;
		
		if (input_length < kMinLength || input_length > kMaxLength) {
			return IDENTIFIER;
		}
		
		// ----------------------------------------------------------------------------
		// Keyword Matcher
#define KEYWORDS(KEYWORD_GROUP, KEYWORD)  \
	KEYWORD_GROUP('a')  \
	KEYWORD("as", AS) \
	KEYWORD("async", ASYNC) \
	KEYWORD_GROUP('c')  \
	KEYWORD("class", CLASS) \
	KEYWORD("const", CONST) \
	KEYWORD_GROUP('d')  \
	KEYWORD("default", DEFAULT) \
	KEYWORD_GROUP('e')  \
	KEYWORD("export", EXPORT) \
	KEYWORD("extends", EXTENDS) \
	KEYWORD("event", EVENT) \
	KEYWORD("else", ELSE) \
	KEYWORD_GROUP('f')  \
	KEYWORD("from", FROM) \
	KEYWORD("function", FUNCTION) \
	KEYWORD_GROUP('g')  \
	KEYWORD("get", GET) \
	KEYWORD_GROUP('i')  \
	KEYWORD("instanceof", INSTANCEOF) \
	KEYWORD("import", IMPORT) \
	KEYWORD("in", IN) \
	KEYWORD("if", IF) \
	KEYWORD_GROUP('l')  \
	KEYWORD("let", LET) \
	KEYWORD_GROUP('o')  \
	KEYWORD("of", OF) \
	KEYWORD_GROUP('r')  \
	KEYWORD("return", RETURN) \
	KEYWORD_GROUP('s')  \
	KEYWORD("set", SET) \
	KEYWORD("static", STATIC) \
	KEYWORD_GROUP('t')  \
	KEYWORD("typeof", TYPEOF) \
	KEYWORD_GROUP('v')  \
	KEYWORD("var", VAR) \
		
		switch (input[0]) {
			default:
#define KEYWORD_GROUP_CASE(ch) break; case ch:
#define KEYWORD(keyword, token)                         \
{                                                       \
/* 'keyword' is a char array, so sizeof(keyword) is */  \
/* strlen(keyword) plus 1 for the NUL char. */          \
	const int keyword_length = sizeof(keyword) - 1;       \
	XX_ASSERT(keyword_length >= kMinLength);               \
	XX_ASSERT(keyword_length <= kMaxLength);               \
	if (input_length == keyword_length &&                 \
		input[1] == keyword[1] &&                           \
		(keyword_length <= 2 || input[2] == keyword[2]) &&  \
		(keyword_length <= 3 || input[3] == keyword[3]) &&  \
		(keyword_length <= 4 || input[4] == keyword[4]) &&  \
		(keyword_length <= 5 || input[5] == keyword[5]) &&  \
		(keyword_length <= 6 || input[6] == keyword[6]) &&  \
		(keyword_length <= 7 || input[7] == keyword[7]) &&  \
		(keyword_length <= 8 || input[8] == keyword[8]) &&  \
		(keyword_length <= 9 || input[9] == keyword[9])) {  \
		return token;                                       \
	}                                                     \
}
KEYWORDS(KEYWORD_GROUP_CASE, KEYWORD)
		}
		return Token::IDENTIFIER;
	}
	
	// <!-- xml comment --> or <xml or </xml> or < <= << <<=
	Token scan_lt_and_xml_element() {
		advance();
		if (c0_ == '!') { // <!-- xml comment -->
			
			advance();
			if (c0_ != '-') {
				back(); return LT;
			}
			
			advance();
			if (c0_ != '-') {
				back(); back(); return LT;
			}
			
			advance();
			while (c0_ >= 0) {
				if (c0_ == '-') {
					advance();
					if (c0_ >= 0) {
						if (c0_ == '-') {
							advance();
							if (c0_ >= 0) {
								if (c0_ == '>') {
									advance();
									return XML_COMMENT;
								} else {
									next_->string_value.push('-');
									next_->string_value.push('-');
									if (clean_comment_) {
										if (c0_ == '\n')
											next_->string_value.push(c0_);
									}
									else
										next_->string_value.push(c0_ == '*' ? 'x' : c0_);
								}
							} else break;
						} else {
							next_->string_value.push('-');
							if (clean_comment_) {
								if (c0_ == '\n')
									next_->string_value.push(c0_);
							}
							else
								next_->string_value.push(c0_ == '*' ? 'x' : c0_);
						}
					} else break;
				} else {
					if (clean_comment_) {
						if (c0_ == '\n')
							next_->string_value.push(c0_);
					}
					else
						next_->string_value.push(c0_ == '*' ? 'x' : c0_);
				}
				advance();
			}
			
			// Unterminated multi-line comment.
			return ILLEGAL;
		}
		else if ( is_xml_element_start(c0_) ) { // <xml
			
			scan_xml_tag_identifier();
			
			int c = c0_; advance();
			if (c == ':' && is_xml_element_start(c0_)) {
				next_->string_value.push(':');
				scan_xml_tag_identifier();
			} else {
				back();
			}
			return XML_ELEMENT_TAG;
			
		}
		else if (c0_ == '/') { // </xml>
			advance();
			
			if (is_xml_element_start(c0_)) {
				
				scan_xml_tag_identifier();
				
				int c = c0_; advance();
				
				if (c == '>') {
					return XML_ELEMENT_TAG_END;
				}
				if (c == ':' && is_xml_element_start(c0_)) {
					next_->string_value.push(':');
					
					scan_xml_tag_identifier();
					
					if (c0_ == '>') {
						advance();
						return XML_ELEMENT_TAG_END;
					}
				}
			}
		}
		else { // < <= << <<=
			if (c0_ == '=') { // <=
				return select(LTE);
			} else if (c0_ == '<') { // <<= or <<
				return select('=', ASSIGN_SHL, SHL);
			} else { // <
				return LT;
			}
		}
		return ILLEGAL;
	}
	
	void scan_xml_tag_identifier() {
		scan_identifier();
		while ( c0_ == '.') {
			advance();
			if ( is_xml_element_start(c0_) ) {
				next_->string_value.push('.');
				scan_identifier();
			} else {
				back(); break;
			}
		}
	}
	
	Token scan_xml_content_string(bool ignore_space) {
		do {
			if ( ignore_space && skip_white_space(true) ) {
				next_->string_value.push(' ');
			}
			if (c0_ == '\\') {
				advance();
				if (c0_ < 0 || !scan_string_escape()) return ILLEGAL;
			} else if (is_line_terminator(c0_)) {
				next_->string_space.push( c0_ );
				// Allow CR+LF newlines in multiline string literals.
				if (is_carriage_return(c0_) && is_line_feed(c0_)) advance();
				// Allow LF+CR newlines in multiline string literals.
				if (is_line_feed(c0_) && is_carriage_return(c0_)) advance();
				next_->string_value.push('\\');
				next_->string_value.push('n');
				advance();
			} else if (c0_ == '"') {
				next_->string_value.push('\\'); // 转义
				next_->string_value.push('"');
				advance();
			} else if (c0_ == '<' || c0_ == '$' || c0_ == '%' || c0_ == '@') {
				break;
			} else {
				next_->string_value.push(c0_);
				advance();
			}
		} while(c0_ >= 0);
		
		return STRIXX_LITERAL;
	}
	
	Token skip_multi_line_comment() {
		advance();

		if (!clean_comment_) {
			next_->string_space.push('/');
			next_->string_space.push('*');
		}

		while (c0_ >= 0) {
			int ch = c0_;
			advance();
			// If we have reached the end of the multi-line comment, we
			// consume the '/' and insert a whitespace. This way all
			// multi-line comments are treated as whitespace.
			if (ch == '*' && c0_ == '/') {
				advance();
				if (!clean_comment_) {
					next_->string_space.push('*');
					next_->string_space.push('/');
				}
				return WHITESPACE;
			} else {
				if (clean_comment_) {
					if (ch == '\n')
						next_->string_space.push(ch);
				}
				else
					next_->string_space.push(ch);
			}
		}
		// Unterminated multi-line comment.
		return ILLEGAL;
	}
	
	Token skip_single_line_comment() {
		advance();
		
		// The line terminator at the end of the line is not considered
		// to be part of the single-line comment; it is recognized
		// separately by the lexical grammar and becomes part of the
		// stream of input elements for the syntactic grammar (see
		if (!clean_comment_) {
			next_->string_space.push('/');
			next_->string_space.push('/');
		}
		while (c0_ >= 0 && !is_line_terminator(c0_)) {
			if (!clean_comment_)
				next_->string_space.push(c0_);
			advance();
		}
		return WHITESPACE;
	}
	
	Token scan_string() {
		byte quote = c0_;
		next_->string_value = Ucs2String();
		advance();  // consume quote
		next_->string_value.push(quote);
		
		while (c0_ != quote && c0_ >= 0 && !is_line_terminator(c0_)) {
			int c = c0_;
			advance();
			if (c == '\\') {
				if (c0_ < 0 || !scan_string_escape()) return ILLEGAL;
			} else {
				next_->string_value.push(c);
			}
		}
		if (c0_ != quote) return ILLEGAL;
		
		next_->string_value.push(quote);
		
		advance();  // consume quote
		return STRIXX_LITERAL;
	}
	
	bool scan_string_escape() {
		int c = c0_;
		advance();
		
		next_->string_value.push('\\');
		// Skip escaped newlines.
		if ( is_line_terminator(c) ) {
			// Allow CR+LF newlines in multiline string literals.
			if (is_carriage_return(c) && is_line_feed(c0_)) advance();
			// Allow LF+CR newlines in multiline string literals.
			if (is_line_feed(c) && is_carriage_return(c0_)) advance();
			next_->string_value.push('\n');
			return true;
		}
		if (c == 'u') {
			if (scan_hex_number(4) == -1) return false;
			next_->string_value.push(&code_[pos_ - 5], 5);
			return true;
		}
		next_->string_value.push(c);
		return true;
	}
	
	int scan_hex_number(int expected_length) {
		XX_ASSERT(expected_length <= 4);  // prevent overflow
		int x = 0;
		for (int i = 0; i < expected_length; i++) {
			
			if (!is_hex_digit(c0_)) { // 不是16进制字符
				for (int j = i - 1; j >= 0; j--) {
					back();
				}
				return -1;
			}
			x = x * 16 + hex_char_to_number(c0_);
			advance();
		}
		return x;
	}
	
	Token scan_number(bool seen_period) {
		Token tok = NUMBER_LITERAL;
		next_->string_value = Ucs2String();
		
		if (seen_period) { // 浮点
			tok = scan_decimal_digit(true);
		}
		else if (c0_ == '0') {
			advance();
			next_->string_value = '0';
			
			if (c0_ < 0) { // 结束,10进制 0
				return tok;
			}
			switch (c0_) {
				case 'b': case 'B': // 0b 2进制
					next_->string_value.push(c0_);
					advance();
					if (is_binary_digit(c0_)) {
						do {
							next_->string_value.push(c0_);
							advance();
						} while(is_binary_digit(c0_));
					} else {
						return ILLEGAL;
					}
					break;

				case 'e': case 'E': // 0e+1 / 1e-6
					tok = scan_decimal_digit(false);
					break;
					
				case 'x': case 'X': // 0x 16进制
					next_->string_value.push(c0_);
					advance();
					if (is_hex_digit(c0_)) {
						do {
							next_->string_value.push(c0_);
							advance();
						} while(is_hex_digit(c0_));
					} else {
						return ILLEGAL;
					}
					break;
					
				case '.': // 10进制浮点数
					back();
					next_->string_value = Ucs2String();
					tok = scan_decimal_digit(false);
					break;
					
				default:
					if (is_octal_digit(c0_)) { // 0 8进制
						do {
							next_->string_value.push(c0_);
							advance();
						} while(is_octal_digit(c0_));
					} // else 10进制 0
					break;
			}
		}
		else { // 10进制
			tok = scan_decimal_digit(false);
		}
		
		if (c0_ >= 0) {
			if (is_identifier_part(c0_) || c0_ == '.') {
				return ILLEGAL;
			}
		}
		return tok;
	}
	
	Token scan_decimal_digit(bool seen_period) {
		
		if (seen_period) { // 直接为浮点数
			next_->string_value.push('.');
			do {
				next_->string_value.push(c0_);
				advance();
			} while(is_decimal_digit(c0_));
		}
		else {
			while (is_decimal_digit(c0_)) { // 整数
				next_->string_value.push(c0_);
				advance();
			}
			
			if (c0_ == '.') { // 浮点数
				next_->string_value.push(c0_);
				advance();
				while (is_decimal_digit(c0_)) {
					next_->string_value.push(c0_);
					advance();
				}
			}
		}
		
		// int i = 1.9e-2;  科学记数法
		if (c0_ == 'e' || c0_ == 'E') {
			next_->string_value.push(c0_);
			advance();
			
			if (c0_ == '+' || c0_ == '-') {
				next_->string_value.push(c0_);
				advance();
			}
			
			if (is_decimal_digit(c0_)) {
				do {
					next_->string_value.push(c0_);
					advance();
				}
				while(is_decimal_digit(c0_));
			}
			else {
				return ILLEGAL;
			}
		}
		
		return NUMBER_LITERAL;
	}
	
	bool skip_white_space(bool ignore_record_first_space = false) {
		uint start_position = pos_;
		bool first = true;
		
		while(true) {
			switch(c0_) {
				case 0x09:  // \t
				case 0x20:  // space
				case 0x0A:  // \n
				case 0x0B:  //
				case 0x0C:  //
				case 0x0D:  // \r
					if (ignore_record_first_space) { // 忽略记录第一个空格
						if (!(first && c0_ == 0x20)) {
							next_->string_space.push(c0_);
						}
					} else {
						next_->string_space.push(c0_);
					}
					first = false;
					advance();
					break;
				default:
					return start_position != pos_;
			}
		}
		return false;
	}
	
	/*
	 * Remove byte order marker. This catches EF BB BF (the UTF-8 BOM)
	 * because the buffer-to-string conversion in `fs.readFileSync()`
	 * translates it to FEFF, the UTF-16 BOM.
	 */
	void strip_bom() {
		//0xFEFF
		//0xFFFE
		if (c0_ == 0xFEFF || c0_ == 0xFFFE) {
			advance();
		}
	}
	
	bool set_pos(uint pos) {
		XX_ASSERT(pos >= 0);
		
		if ( pos < size_ ) {
			if (pos > pos_) {
				do {
					if (c0_ == '\n') line_++;
					pos_++;
					c0_ = code_[pos_];
				} while (pos_ != pos);
			} else {
				while (pos_ != pos) {
					pos_--;
					c0_ = code_[pos_];
					if (c0_ == '\n') line_--;
				}
			}
		} else {
			return false;
		}
		return true;
	}
	
	inline void advance() {
		if (pos_ < size_) {
			pos_++;
			if (c0_ == '\n') line_++;
			c0_ = pos_ == size_ ? -1 : code_[pos_];
		}
		else c0_ = -1;
	}
	
	inline void back() { // undo advance()
		if (pos_ != 0) {
			pos_--;
			c0_ = code_[pos_];
			if (c0_ == '\n') line_--;
		}
	}
	
	inline Token select(Token tok) { advance(); return tok; }
	
	inline Token select(int next, Token then, Token else_) {
		advance();
		return c0_ == next ? advance(), then: else_;
	}
	
	const uint16 *code_;
	uint size_, pos_, line_;
	int c0_;
	TokenDesc *current_, *next_;
	Token prev_;
	bool clean_comment_;
};

/**
 * @class Parser
 */
class Parser: public Object {
public:
	
	Parser(cUcs2String& in, cString& path, bool is_jsx, bool clean_comment)
	: _out(nullptr)
	, _path(path)
	, _level(0)
	, _is_jsx(is_jsx)
	, _is_class_member_data_expression(false)
	, _is_xml_attribute_expression(false)
	, _single_if_expression_before(false)
	, _has_export_default(false)
	, _clean_comment(clean_comment)
	{
		if (!static_str_list) {
			static_str_list = new static_str_list_t();
		}
		_scanner = new Scanner(*in, in.length(), _clean_comment);
		_out = &_top_out;
	}
	
	virtual ~Parser() {
		Release(_scanner);
	}
	
	Ucs2String transform() {
		parse_document();
		Ucs2String rv = _out->to_basic_string();
		//if ( _path.index_of("test.js") != -1 ) {
		//  String str = rv.to_string();
		//  LOG(str);
		//}
		return rv;
	}
	
 private:
	
	void parse_document() {
		
		Token token = next();
		
		if ( token == SHELL_HEADER ) {
			fetch_code();
			token = next();
		}
		
		while (token != EOS) {
			if (token == EXPORT) {
				parse_export();
			} else {
				parse_advance();
			}
			token = next();
		}
		
		// class member data
		for ( auto& i : _class_member_data_expression ) {
			if ( i.value().expressions.length() ) {
				out_code(S.NEWLINE);   // \n
				out_code(S.__EXTEND);   // __extend
				out_code(S.LPAREN);    // (
				out_code(i.value().class_name);    // class_name
				out_code(S.PERIOD);    // .
				out_code(S.PROTOTYPE);  // prototype
				out_code(S.COMMA);     // ,
				out_code(S.LBRACE);    // {
				out_code(S.NEWLINE);   // \n
				for ( auto& j : i.value().expressions ) {
					out_code(S.INDENT);    // \t
					out_code(j.key());    // identifier
					out_code(S.COLON);     // :
					out_code(j.value().to_basic_string());  // expression
					out_code(S.COMMA);     // ,
					out_code(S.NEWLINE);   // \n
				}
				out_code(S.RBRACE);    // }
				out_code(S.COMMA);     // ,
				out_code(S.NUMBER_1);  // 1
				out_code(S.RPAREN);    // )
				out_code(S.SEMICOLON); // ;
			}
		}
		
		// export
		for (uint i = 0; i < _exports.length(); i++) {
			out_code(S.NEWLINE);
			out_code(S.EXPORTS);    // exports.xxx=xxx;
			out_code(S.PERIOD);     // .
			out_code(_exports[i]); // xxx
			out_code(S.ASSIGN);     // =
			out_code(_exports[i]); // xxx
			out_code(S.SEMICOLON);  // ;
		}
		
		// export default
		if (_has_export_default && !_export_default.is_empty()) {
			out_code(S.NEWLINE);
			out_code(S.EXPORT_DEFAULT);  // exports.default=xxx;
			out_code(S.ASSIGN);          // =
			out_code(_export_default);  // xxx
			out_code(S.SEMICOLON);       // ;
		}
		
		out_code(S.NEWLINE);
	}

	void parse_advance() {
		
		collapse_scape();
		
		switch(token()) {
			case WHITESPACE:  // space
			case INSTANCEOF:  // instanceof
			case TYPEOF:  // typeof
			case IDENTIFIER:  // identifier
			case IN:  // in
			case AS:  // as
			case ASYNC: // async
			case FROM:  // from
			case OF:  // of
			case RETURN:  // return
			case VAR: // var
			case FUNCTION:  // function
			case LET: // let
			case DEFAULT: // default
			case CONST: // const
			case EVENT: // event
			case ELSE:  // else
			case GET: // get
			case SET: // set
			case NUMBER_LITERAL:  // number
			case STRIXX_LITERAL:  // string
				fetch_code(); break;
			case CLASS: // class
				if ( _scanner->prev() == PERIOD ) { // .class
					fetch_code();
				} else {
					parse_class(); // class xxxx { }
				}
				break;
			case IF:  // if
				fetch_code();
				ASSERT_NEXT(LPAREN);        // (
				out_code(S.LPAREN);
				parse_brace_expression(LPAREN, RPAREN);
				out_code(S.RPAREN);
				if ( peek() != LBRACE ) { // {
					next();
					_single_if_expression_before = true;
					parse_advance();
					_single_if_expression_before = false;
				}
				break;
			case LT:                      // <
				out_code(S.LT); break;
			case GT:                      // >
				out_code(S.GT); break;
			case ASSIGN:                  // =
				out_code(S.ASSIGN); break;
			case PERIOD:                  // .
				out_code(S.PERIOD); break;
			case ADD:                     // +
				out_code(S.ADD); break;
			case SUB:                     // -
				out_code(S.SUB); break;
			case COMMA:                   // ,
				out_code(S.COMMA); break;
			case COLON:                   // :
				out_code(S.COLON); break;
			case SEMICOLON:               // ;
				out_code(S.SEMICOLON); break;
			case CONDITIONAL:             // ?
				out_code(S.CONDITIONAL); break;
			case NOT:                     // !
				out_code(S.NOT); break;
			case BIT_OR:                  // |
				out_code(S.BIT_OR); break;
			case BIT_NOT:                 // ~
				out_code(S.BIT_NOT); break;
			case BIT_XOR:                 // ^
				out_code(S.BIT_XOR); break;
			case MUL:                     // *
				out_code(S.MUL); break;
			case POWER:                     // **
				out_code(S.POWER); break;
			case BIT_AND:                 // &
				out_code(S.BIT_AND); break;
			case MOD:                     // %
				out_code(S.MOD); break;
			case INC:                    // ++
				out_code(S.INC); break;
			case DEC:                    // --
				out_code(S.DEC); break;
			case ASSIGN_BIT_OR:          // |=
				out_code(S.ASSIGN_BIT_OR); break;
			case ASSIGN_BIT_XOR:         // ^=
				out_code(S.ASSIGN_BIT_XOR); break;
			case ASSIGN_BIT_AND:         // &=
				out_code(S.ASSIGN_BIT_AND); break;
			case ASSIGN_SHL:             // <<=
				out_code(S.ASSIGN_SHL); break;
			case ASSIGN_SAR:             // >>=
				out_code(S.ASSIGN_SAR); break;
			case ASSIGN_SHR:             // >>>=
				out_code(S.ASSIGN_SHR); break;
			case ASSIGN_ADD:             // +=
				out_code(S.ASSIGN_ADD); break;
			case ASSIGN_SUB:             // -=
				out_code(S.ASSIGN_SUB); break;
			case ASSIGN_MUL:             // *=
				out_code(S.ASSIGN_MUL); break;
			case ASSIGN_POWER:             // **=
				out_code(S.ASSIGN_POWER); break;
			case ASSIGN_MOD:             // %=
				out_code(S.ASSIGN_MOD); break;
			case OR:                     // ||
				out_code(S.OR); break;
			case AND:                    // &&
				out_code(S.AND); break;
			case SHL:                    // <<
				out_code(S.SHL); break;
			case SAR:                    // >>
				out_code(S.SAR); break;
			case SHR:                    // >>>
				out_code(S.SHR); break;
			case EQ:                     // ==
				out_code(S.EQ); break;
			case NE:                     // !=
				out_code(S.NE); break;
			case EQ_STRICT:              // ===
				out_code(S.EQ_STRICT); break;
			case NE_STRICT:              // !==
				out_code(S.NE_STRICT); break;
			case LTE:                    // <=
				out_code(S.LTE); break;
			case GTE:                    // >=
				out_code(S.GTE); break;
			case XML_ELEMENT_TAG:         // <xml
				if ( _is_xml_attribute_expression ) {
					UNEXPECTED_TOKEN_ERROR();
				} else {
					parse_xml_element(false);
				}
				break;
			case XML_COMMENT:             // <!-- comment -->
				if (_is_jsx && !_is_xml_attribute_expression) {
					out_code(S.XML_COMMENT);
					out_code(_scanner->string_value());
					out_code(S.XML_COMMENT_END);
				} else {
					UNEXPECTED_TOKEN_ERROR();
				}
				break;
			case LPAREN:                  // (
				out_code(S.LPAREN);
				parse_brace_expression(LPAREN, RPAREN);
				out_code(S.RPAREN);
				break;
			case LBRACK:                  // [
				out_code(S.LBRACK);
				parse_brace_expression(LBRACK, RBRACK);
				out_code(S.RBRACK);
				break;
			case LBRACE:                  // {
				out_code(S.LBRACE);
				parse_brace_expression(LBRACE, RBRACE);
				out_code(S.RBRACE);
				break;
			case COMMAND:                 // `str${
				parse_command_string();
				break;
			case IMPORT:                  // import
				 parse_import(); break;
			case DIV:                     // / regexp
			case ASSIGN_DIV:              // /=
				if (is_legal_literal_begin(false)) {
					parse_regexp_expression();
				} else {
					if (token() == DIV) {
						out_code(S.DIV);
					} else {
						out_code(S.ASSIGN_DIV);
					}
				}
				break;
			case ILLEGAL:                 // illegal
			default: UNEXPECTED_TOKEN_ERROR(); break;
		}
	}
	
	void parse_expression() {
		parse_single_expression();
		parse_operation_expression();
	}
	
	void parse_single_expression() {
		
		Token tok = next();
		
		if ( is_declaration_identifier(tok) ) {
			parse_variable_visit_expression();
			parse_identifier_expression();
			return;
		}
		
		switch (tok) {
			case FUNCTION: // 暂不处理函数表达式
				UNEXPECTED_TOKEN_ERROR(); break;
			case NUMBER_LITERAL: // 10
			case STRIXX_LITERAL: // "String"
				fetch_code();
				break;
			case COMMAND:     // `str${
				// command string start
				parse_command_string();
				break;
			case DIV:            // /     /regexp/imgyu
			case ASSIGN_DIV:     // /=    /regexp/imgyu
				parse_regexp_expression();
				break;
			case XML_ELEMENT_TAG:         // <xml
				if ( _is_xml_attribute_expression ) {
					// xml 属性不能为xml
					UNEXPECTED_TOKEN_ERROR();
				} else {
					parse_xml_element(false);
				}
				break;
			case XML_COMMENT:             // <!-- comment -->
				if (_is_jsx && !_is_xml_attribute_expression) {              //
					if ( _is_class_member_data_expression ) { // 结束原先的多行注释
						_out->push(S.COMMENT_END); // */
					}
					out_code(S.XML_COMMENT);
					out_code(_scanner->string_value());
					out_code(S.XML_COMMENT_END);
					if ( _is_class_member_data_expression ) { // 重新开始多行注释
						_out->push(S.COMMENT); // /*
					}
				} else {
					UNEXPECTED_TOKEN_ERROR();
				}
				break;
			case LPAREN:                  // (
				out_code(S.LPAREN);
				parse_brace_expression(LPAREN, RPAREN);
				out_code(S.RPAREN);
				break;
			case LBRACK:                  // [
				out_code(S.LBRACK);
				parse_brace_expression(LBRACK, RBRACK);
				out_code(S.RBRACK);
				break;
			case LBRACE:                  // {
				out_code(S.LBRACE);
				parse_brace_expression(LBRACE, RBRACE);
				out_code(S.RBRACE);
				break;
			case INC:            // ++
			case DEC:            // --
				if ( is_declaration_identifier(peek()) ) {
					out_code(tok == INC ? S.INC : S.DEC);
					parse_single_expression();
				} else {
					UNEXPECTED_TOKEN_ERROR();
				}
				break;
			case TYPEOF:         // typeof
				fetch_code(); parse_single_expression(); break;
			case ADD:            // +
				out_code(S.ADD); parse_single_expression(); break;
			case SUB:            // -
				out_code(S.SUB); parse_single_expression(); break;
			case NOT:            // !
				out_code(S.NOT); parse_single_expression(); break;
			case BIT_NOT:        // ~
				out_code(S.BIT_NOT); parse_single_expression(); break;
			default:
				UNEXPECTED_TOKEN_ERROR(); break;
		}
	}
	
	void parse_operation_expression() {
		
		Token op = peek();
		
		if ( isAssignmentOp(op) ) {
			// 无法确定左边表达式是否为一个变量表达式,所以暂时不处理,
			UNEXPECTED_TOKEN_ERROR();
		}
		
		while(true) {
			
			// cond ? then_exp : else_exp
			if (op == CONDITIONAL) { // 三元运算符,就此终结表达式
				parse_conditional_expression(); return;
			}
			
			if ( _is_xml_attribute_expression ) { // xml 属性中不允许使用运算符,要使用运算符号必须使用小括号
				return;
			}
			
			/* Not two Binary operators or Compare operators. */
			if (isBinaryOrCompareOp(op)) {
				
				next(); fetch_code();
				parse_single_expression();
				op = peek();
			} else {
				
				if ( op == XML_ELEMENT_TAG ) { // <tag
					// parse with compare
					next();
					out_code(S.LT);  // <
					fetch_code();   // identifier
					op = peek();
				} else { // 就此终结
					return;
				}
			}
		}
		
	}
	
	void parse_conditional_expression() {
		ASSERT_NEXT(CONDITIONAL); // ?
		out_code(S.CONDITIONAL); // ?
		parse_expression();
		ASSERT_NEXT(COLON); // :
		out_code(S.COLON); // ?
		parse_expression();
	}
	
	void parse_regexp_expression() {
		XX_ASSERT(_scanner->token() == DIV || _scanner->token() == ASSIGN_DIV);
		
		if (_scanner->scan_regexp(_scanner->location().beg_pos) == REGEXP_LITERAL) {
			_scanner->next();
			fetch_code();
		} else {
			error("RegExp Syntax error");
		}
	}
	
	void parse_brace_expression(Token begin, Token end) {
		XX_ASSERT( _scanner->token() == begin );
		uint level = _level;
		_level++;
		while(true) {
			Token token = next();
			if (token == end) {
				break;
			} else if (token == EOS) {
				UNEXPECTED_TOKEN_ERROR();
			} else {
				parse_advance();
			}
		}
		_level--;
		if ( _level != level ) {
			UNEXPECTED_TOKEN_ERROR();
		}
		collapse_scape();
	}
	
	void parse_variable_visit_expression() {
		XX_ASSERT( is_declaration_identifier(token()) );
		
		fetch_code(); // identifier
		
		while(true) {
			switch(peek()) {
				case PERIOD:     // .
					next(); out_code( S.PERIOD ); // .
					next();
					ASSERT(is_declaration_identifier(token()));
					fetch_code(); // identifier
					break;
				case LBRACK:     // [
					parse_single_expression();
					break;
				default: return;
			}
		}
	}
	
	void parse_identifier_expression() {
		
		switch (peek()) {
			case INC:    // ++
				next(); out_code(S.INC);
				return;
			case DEC:    // --
				next(); out_code(S.DEC);
				return;
			case LPAREN: // ( function call
				break;
			default:
				return;
		}
		
		// parse function call
		
		next();
		out_code(S.LPAREN); // (
		parse_brace_expression(LPAREN, RPAREN);
		out_code(S.RPAREN); // )
	}
	
	void parse_command_string() {
		if ( _scanner->string_value().is_empty()) {
			UNEXPECTED_TOKEN_ERROR();
		}
		
		while(true) {
			XX_ASSERT(peek() == LBRACE);
			out_code(_scanner->string_value());
			out_code(S.COMMAND);  // ${
			_scanner->next();
			parse_command_string_block(); // parse { block }
			XX_ASSERT(peek() == RBRACE);
			out_code(S.RBRACE);   // }
			_scanner->scan_command_string(_scanner->next_location().end_pos);
			Token tok = _scanner->next();
			if (tok == COMMAND_END) {
				fetch_code(); break;
			} else if (tok != COMMAND) {
				UNEXPECTED_TOKEN_ERROR();
			}
		}
	}
	
	void parse_command_string_block() {
		XX_ASSERT( _scanner->token() == LBRACE );
		while(true) {
			if (peek() == RBRACE) {
				break;
			}
			if (next() == EOS) {
				UNEXPECTED_TOKEN_ERROR();
			} else {
				parse_advance();
			}
		}
		collapse_scape();
	}

	Ucs2String to_event_js_code(cUcs2String& name) {
		//  get onchange() { return this.getNoticer('change') }
		//  set onchange(func) { this.addDefaultListener('change', func) }
		//  triggerchange(data, is_event) { return this.$trigger('change', data, is_event) }
		//
		static cUcs2String a1(String("get on"));
		static cUcs2String a2(String("() { return this.getNoticer('"));
		static cUcs2String a3(String("') }"));
		static cUcs2String b1(String("set on"));
		static cUcs2String b2(String("(func) { this.addDefaultListener('"));
		static cUcs2String b3(String("', func) }"));
		static cUcs2String c1(String("trigger"));
		static cUcs2String c2(String("(ev,is_ev) { return this.$trigger('"));
		static cUcs2String c3(String("',ev,is_ev) }"));
		
		Ucs2String rv;
		rv.push(a1); rv.push(name); rv.push(a2); rv.push(name); rv.push(a3);
		rv.push(b1); rv.push(name); rv.push(b2); rv.push(name); rv.push(b3);
		rv.push(c1); rv.push(name); rv.push(c2); rv.push(name); rv.push(c3);
		return rv;
	}

	void parse_class() {
		XX_ASSERT(_scanner->token() == CLASS);
		
		fetch_code();
		
		Ucs2String class_name;
		MemberDataExpression* member_data = nullptr;
		
		Token tok = next();
		if (is_declaration_identifier(tok)) {
			fetch_code();
			
			class_name = _scanner->string_value();
			
			if ( _level == 0 ) {
				uint len = _class_member_data_expression.push({ class_name });
				member_data = &_class_member_data_expression[len - 1];
			}
			
			tok = next();
			if (tok == EXTENDS) {
				fetch_code();
				
				while(true) { // find {
					tok = next();
					if (tok == LBRACE) { // {
						break;
					} else if (tok == EOS) {
						UNEXPECTED_TOKEN_ERROR();
					} else {
						parse_advance();
					}
				}
			} else if (tok != LBRACE) { // {
				UNEXPECTED_TOKEN_ERROR();
			}
		}
		else if (tok == EXTENDS) {  // extends
			fetch_code();
			// parse_member_data = false; // Anonymous class not parse member data
			ASSERT_NEXT(LBRACE); // {
		} else {
			ASSERT_TOKEN(LBRACE); // {
		}
		
		XX_ASSERT(_scanner->token() == LBRACE); // {
		
		out_code(S.LBRACE); // {
		
		while(true) {
			Token tok = next();
			
			switch(tok) {
				case GET:
				case SET:
				 get_set:
					if ( is_class_member_identifier(peek()) ) {
						// Property accessor
						// get identifier() { ... }
						// set identifier(identifier) { ... }
						fetch_code(); // fetch get or set identifier
						next(); // identifier
						fetch_code(); // fetch identifier
						if ( next() == LPAREN ) { // (
							out_code(S.LPAREN); // (
							if ( tok == SET ) {
								if ( is_declaration_identifier(next()) ) { // param identifier
									fetch_code(); // fetch set param identifier
								} else {
									error("syntax error, define set property accessor");
								}
							}
							if ( next() == RPAREN ) { // )
								out_code(S.RPAREN); // )
								if ( next() == LBRACE ) {
									out_code(S.LBRACE); // {
									parse_brace_expression(LBRACE, RBRACE);
									out_code(S.RBRACE); // }
									break; // ok
								}
							}
						}
						error("Syntax error, define property accessor");
					}
				case IDENTIFIER:
				case AS:
				case OF:
				case FROM:
				case STRIXX_LITERAL:
					//class member identifier
					if ( peek() == LPAREN ) { // (
						goto function;
					} else if ( member_data && peek() == ASSIGN ) { // = class member data
						out_code(S.COMMENT); // /*
						Ucs2String identifier = _scanner->string_value();
						fetch_code(); // identifier
						next(); // =
						out_code(S.ASSIGN); // =
						_out_class_member_data_expression.clear();
						_is_class_member_data_expression = true;
						parse_expression();
						_is_class_member_data_expression = false;
						ASSERT_NEXT(SEMICOLON); // ;
						member_data->expressions.set(identifier, move(_out_class_member_data_expression));
						out_code(S.COMMENT_END); // */
					} else {
						UNEXPECTED_TOKEN_ERROR();
					}
					break;
				case STATIC: // static
					fetch_code(); // fetch static
					if (next() == GET || _scanner->token() == SET) {
						tok = _scanner->token();
						goto get_set;
					} else if (_scanner->token() == ASYNC) {
						goto async;
					} else if ( is_class_member_identifier(_scanner->token()) ) {
						goto function;
					} else if (_scanner->token() == MUL) {
						goto mul;
					}
					UNEXPECTED_TOKEN_ERROR();
				case ASYNC: // async function
				 async:
					fetch_code(); // fetch async
					if ( is_class_member_identifier(next()) ) {
						goto function;
					} else if (_scanner->token() != MUL) {
						UNEXPECTED_TOKEN_ERROR();
					}
				case MUL: // * generator function
				 mul:
					out_code(S.MUL);
					if ( !is_class_member_identifier(next()) ) {
						UNEXPECTED_TOKEN_ERROR();
					}
				 function:
					fetch_code(); // fetch function identifier
					if ( next() == LPAREN ) { // arguments
						out_code(S.LPAREN); // (
						parse_brace_expression(LPAREN, RPAREN);
						out_code(S.RPAREN); // )
						if ( next() == LBRACE ) { // function body
							out_code(S.LBRACE); // {
							parse_brace_expression(LBRACE, RBRACE); //
							out_code(S.RBRACE); // }
							break; // ok
						}
					}
					UNEXPECTED_TOKEN_ERROR();
				case EVENT: {
					// Event declaration
					ASSERT_NEXT(IDENTIFIER); // event onevent
					Ucs2String event = _scanner->string_value();
					if (event.length() > 2 &&
							event[0] == 'o' &&
							event[1] == 'n' && is_xml_element_start(event[2])) {
						out_code(to_event_js_code(event.substr(2)));
						ASSERT_NEXT(SEMICOLON); // ;
					} else {
						error("Syntax error, event name incorrect");
					}
					break;
				}
				case SEMICOLON: //;
					out_code(S.SEMICOLON); break;
				case RBRACE: // }
					goto end;  // Class end
				default:
					UNEXPECTED_TOKEN_ERROR(); break;
			};
		}
		
	 end:
		out_code(S.RBRACE); // {
	}
	
	void parse_export() {
		XX_ASSERT(_scanner->token() == EXPORT);
		
		collapse_scape();
		Token tok = _scanner->next();
		bool has_export_default = false;
		
		if (tok == DEFAULT) {
			if (_has_export_default) {
				UNEXPECTED_TOKEN_ERROR();
			} else {
				_has_export_default = true;
				has_export_default = true;
			}
			collapse_scape();
			tok = _scanner->next();
		}
		
		switch (tok) {
			case ASYNC:
				out_code(S.EXPORT_COMMENT);
				fetch_code();
				ASSERT_NEXT(FUNCTION);
				tok = token();
				goto identifier;
			case VAR:       // var
			case CLASS:     // class
			case FUNCTION:  // function
			case LET:       // let
			case CONST:     // const
				out_code(S.EXPORT_COMMENT);
			 identifier:
				if ( is_declaration_identifier(peek()) ) {
					if (has_export_default) {
						_export_default = _scanner->next_string_value();
					} else {
						_exports.push(_scanner->next_string_value());
					}
					parse_advance();
				} else {
					UNEXPECTED_TOKEN_ERROR();
				}
				break;
			case LPAREN:                  // (
			case LBRACK:                  // [
			case LBRACE:                  // {
				goto _export;
			default:
				if ( is_declaration_identifier(tok) ) {
				 _export:
					if (has_export_default) {
						out_code(S.EXPORT_DEFAULT); // exports.default = expression
					} else {
						out_code(S.MODULE_EXPORT);  // module._export = expression
					}
					out_code(S.ASSIGN); // =
					parse_advance();   // expression
				} else {
					UNEXPECTED_TOKEN_ERROR();
				}
				break;
		}
	}
	
	void parse_import() {
		XX_ASSERT(_scanner->token() == IMPORT);
		Token tok = _scanner->next();

		if (is_import_declaration_identifier(tok)) { // identifier
			out_code(S.CONST); // const
			collapse_scape();
			Ucs2String id = _scanner->string_value();
			tok = _scanner->next();
			
			if (tok == FROM) { // import default
				// import app from 'qgr/app';
				out_code(id);      // app
				out_code(S.ASSIGN); // =
				ASSERT_NEXT(STRIXX_LITERAL);
				out_code(S.REQUIRE); // require('qgr/app').default;
				out_code(S.LPAREN); // (
				fetch_code();
				out_code(S.RPAREN); // )
				out_code(S.PERIOD); // .
				out_code(S.DEFAULT); // default
			} else if (tok == COMMA) { // ,
				// import app, { GUIApplication } from 'qgr/app';
				// Not support `as` keyword `import app, { GUIApplication as App } from 'qgr/app'`
				out_code(S.LBRACE);
				ASSERT_NEXT(LBRACE); // {
				out_code(S.DEFAULT);  // default
				out_code(S.COLON);    // :
				out_code(id);        // app
				out_code(S.COMMA);    // :
				parse_brace_expression(LBRACE, RBRACE);
				out_code(S.RBRACE); // }
				ASSERT_NEXT(FROM);
				out_code(S.ASSIGN); // =
				ASSERT_NEXT(STRIXX_LITERAL);
				out_code(S.REQUIRE); // require('qgr/app');
				out_code(S.LPAREN); // (
				fetch_code();
				out_code(S.RPAREN); // )
			}
			else {
				UNEXPECTED_TOKEN_ERROR();
			}
		}
		else if (tok == MUL) {  // import * as app from 'qgr/app';
			out_code(S.CONST); // const
			ASSERT_NEXT(AS);      // as
			tok = _scanner->next();
			if (is_import_declaration_identifier(tok)) {
				fetch_code();
				ASSERT_NEXT(FROM);
				out_code(S.ASSIGN);  // =
				ASSERT_NEXT(STRIXX_LITERAL);
				out_code(S.REQUIRE); // require('qgr/app');
				out_code(S.LPAREN); // (
				fetch_code();
				out_code(S.RPAREN); // )
			} else {
				UNEXPECTED_TOKEN_ERROR();
			}
		}
		else if (tok == LBRACE) { // {
			out_code(S.CONST); // var
			collapse_scape();
			// import { GUIApplication } from 'qgr/app';
			// Not support `as` keyword `import { GUIApplication as App } from 'qgr/app'`
			parse_advance();
			ASSERT_NEXT(FROM);
			out_code(S.ASSIGN); // =
			ASSERT_NEXT(STRIXX_LITERAL);
			out_code(S.REQUIRE); // require('qgr/app');
			out_code(S.LPAREN); // (
			fetch_code();
			out_code(S.RPAREN); // )
		}
		else if (tok == STRIXX_LITERAL) {
			// qgr private syntax
		
			Ucs2String str = _scanner->string_value();
			if (peek() == AS) {
				// import 'test_gui.jsx' as gui;
				
				out_code(S.CONST); // var
				collapse_scape();
				next(); // as
				if ( is_import_declaration_identifier(peek()) ) {
					out_code(_scanner->next_string_value());
					out_code(S.SPACE); //
					out_code(S.ASSIGN); // =
					next(); // IDENTIFIER
					out_code(S.REQUIRE); // require('qgr/app');
					out_code(S.LPAREN); // (
					out_code(str);
					out_code(S.RPAREN); // )
				} else {
					UNEXPECTED_TOKEN_ERROR();
				}
			} else {
				// import 'test_gui.jsx';
				// find identifier
				
				Ucs2String path = str.substr(1, str.length() - 2).trim();
				String basename = Path::basename(path);
				int i = basename.last_index_of('.');
				if (i != -1) {
					basename = basename.substr(0, i);
				}
				basename = basename.replace_all('.', '_').replace_all('-', '_');
				
				if (is_identifier_start(basename[0])) {
					i = basename.length();
					while (--i) {
						if (!is_identifier_part(basename[i])) {
							basename = String(); break;
						}
					}
					// use the identifier
				} else {
					basename = String();
				}
				
				if (!basename.is_empty()) {
					out_code(S.CONST); // const
					out_code(S.SPACE); //
					out_code(Coder::decoding_to_uint16(Encoding::utf8, basename)); // identifier
					out_code(S.SPACE); //
					out_code(S.ASSIGN); // =
					out_code(S.SPACE);  //
				}
				out_code(S.REQUIRE); // require('qgr/app');
				out_code(S.LPAREN); // (
				out_code(str);
				out_code(S.RPAREN); // )
				collapse_scape();
			}
		}
		else { // 这可能是个函数调用,不理会
			// import();
			fetch_code();
		}
	}

	bool is_legal_literal_begin(bool xml) {
		Token tok = _scanner->prev();
		
		if ( xml ) {
			switch (tok) {
				case INC:         // ++ //if (i++<a.length) { }
				case DEC:         // -- //if (i--<a.length) { }
					return false;
					break;
				default: break;
			}
		}
		
		// 上一个词为这些时,下一个词可以为 literal expression
		
		if ( ASSIGN <= tok && tok <= TYPEOF ) {
			return true;
		}
		
		switch (tok) {
			case OF:              // of
			case LBRACE:          // {
			case RBRACE:          // }
			case LPAREN:          // (
			case LBRACK:          // [
			case COMMA:           // ,
			case RETURN:          // return
			case ELSE:            // else
			case COLON:           // :
			case SEMICOLON:       // ;
			case CONDITIONAL:     // ?
				return true;
			case RPAREN:          // )
				if ( _single_if_expression_before ) {
					return true;
				}
				break;
			case RBRACK:          // ]
				if ( _scanner->before_line_feed() ) {
					return true;
				}
			default: break;
		}
		
		return false;
	}
	
	bool is_object_property_identifier(Token token) {
		switch(token) {
			default: return false;
			case IDENTIFIER:
			case AS:
			case ASYNC:
			case EXPORT:
			case FROM:
			case IMPORT:
			case INSTANCEOF:
			case IN:
			case IF:
			case OF:
			case RETURN:
			case TYPEOF:
			case VAR:
			case CLASS:
			case FUNCTION:
			case LET:
			case DEFAULT:
			case CONST:
			case EXTENDS:
			case EVENT:
			case ELSE:
			case GET:
			case SET:
				return true;
		}
	}
	
	bool is_declaration_identifier(Token token) {
		switch(token) {
			default: return false;
			case IDENTIFIER:
			case AS:
			case ASYNC:
			case OF:
			case FROM:
			case EVENT:
			case GET:
			case SET:
				return true;
		}
	}
	
	bool is_class_member_identifier(Token token) {
		switch(token) {
			default: return false;
			case IDENTIFIER:
			case AS:
			case ASYNC:
			case OF:
			case FROM:
			case EVENT:
			case GET:
			case SET:
			case STRIXX_LITERAL:
				return true;
		}
	}
	
	bool is_import_declaration_identifier(Token token) {
		switch(token) {
			default: return false;
			case IDENTIFIER:
			case ASYNC:
			case OF:
			case EVENT:
			case GET:
			case SET:
				return true;
		}
	}
	
	void parse_xml_element(bool xml_inl) {
		XX_ASSERT(_scanner->token() == XML_ELEMENT_TAG);
		
		if (!_is_jsx || (!xml_inl && !is_legal_literal_begin(true)) ) {
			out_code(S.LT);
			fetch_code();
			return;
		} else {
			Token token = peek();
			if ( token == PERIOD || token == COLON ) { // not xml
				out_code(S.LT);
				fetch_code();
				return;
			}
		}
		
		collapse_scape(); // push scape
		
		// 转换xml为json对像:
		
		Ucs2String tag_name = _scanner->string_value();
		int index = tag_name.index_of(':');
		bool vx_com = false;
		
		if (index != -1) {
			Ucs2String prefix = tag_name.substr(0, index);
			Ucs2String suffix = tag_name.substr(index + 1);
			
			if (prefix == S.VX) {  // <vx:tag
				// __vx(tag,[attrs],vdata)
				// final result:
				// {vx:0,v:[tag,[attrs],[child],vdata]}
				out_code(S.__VX);     // __vx
				out_code(S.LPAREN);  // (
				out_code(suffix);   // tag
				out_code(S.COMMA);   // ,
				vx_com = true;
			} else {                // <prefix:suffix
				// {vx:1,v:[prefix,suffix,[attrs],[child],vdata]}
				out_code(S.LBRACE);    // {
				out_code(S.TYPE);      // t
				out_code(S.COLON);     // :
				out_code(S.NUMBER_1);  // 1
				out_code(S.COMMA);     // ,
				out_code(S.VALUE2);    // v
				out_code(S.COLON);     // :
				out_code(S.LBRACK);    // [
				out_code(prefix);     // prefix
				out_code(S.COMMA);     // ,
				out_code(suffix);     // suffix
				out_code(S.COMMA);     // ,
			}
		} else {              // <tag
			// {vx:0,v:[tag,[attrs],[child],vdata]}
			out_code(S.LBRACE);    // {
			out_code(S.TYPE);      // t
			out_code(S.COLON);     // :
			out_code(S.NUMBER_0);  // 0
			out_code(S.COMMA);     // ,
			out_code(S.VALUE2);    // v
			out_code(S.COLON);     // :
			out_code(S.LBRACK);    // [
			out_code(tag_name);   // tag
			out_code(S.COMMA);     // ,
		}
		
		Map<Ucs2String, bool> attrs;
		Ucs2StringBuilder vdata;
		Token token = _scanner->next();
		bool start_parse_attrs = false;
		
		// 解析xml属性
		if (is_object_property_identifier(token)) {
		 attr:
			if (!_scanner->has_scape_before()) { // xml属性之间必须要有空白符号
				error("Xml Syntax error");
			}
			collapse_scape();  // scape
			
			if (!start_parse_attrs) {
				out_code(S.LBRACK);    // [ parse attributes start
				//out_code(_ATTRS_COMMENT); // add comment
				start_parse_attrs = true;
			}
			
			// 添加属性
			Ucs2StringBuilder* raw_out = _out;
			Ucs2String attribute_name;
			bool is_vdata = (_scanner->string_value() == S.VDATA && peek() != PERIOD);
			if (is_vdata) {
				_out = &vdata;
			}
			out_code(S.LBRACK); // [ // attribute start
			out_code(S.LBRACK); // [ // attribute name start
			
			do { // .
				out_code(S.QUOTES); // "
				out_code(_scanner->string_value());
				out_code(S.QUOTES); // "
				attribute_name.push(_scanner->string_value());
				token = next();
				if (token != PERIOD) break;
				if (!is_object_property_identifier(next())) {
					error("Xml Syntax error");
				}
				out_code(S.COMMA);   // ,
			} while (true);
			
			out_code(S.RBRACK); // ] // attribute name end
			out_code(S.COMMA);  // ,
			
			if (attrs.has(attribute_name)) {
				error(String("Xml Syntax error, attribute repeat: ") + attribute_name.to_string());
			}
			attrs.set(attribute_name, 1);
			
			if (token == ASSIGN) { // =
				// 有=符号,属性必须要有值,否则为异常
				_is_xml_attribute_expression = true;
				if ( peek() == COMMAND_DATA_BIND ) { // %%{
					parse_xml_attribute_data_bind(false);
				} else if (peek() == COMMAND_DATA_BIND_ONCE) { // %{
					parse_xml_attribute_data_bind(true);
				} else {
					out_code(S.NUMBER_0);  // 0
					out_code(S.COMMA);     // ,
					parse_expression(); // 解析属性值表达式
				}
				_is_xml_attribute_expression = false;
				token = _scanner->next();
			} else { // 没有值设置为 ""
				out_code(S.NUMBER_0);  // 0
				out_code(S.COMMA);     // ,
				out_code(S.QUOTES);    // "
				out_code(S.QUOTES);    // "
			}
			out_code(S.RBRACK); // ] // attribute end
			
			if (is_vdata) {
				_out = raw_out;
			}
			
			if (is_object_property_identifier(token)) {
				if (!is_vdata) {
					out_code(S.COMMA);   // ,
				}
				goto attr; // 重新开始新属性
			}
		} else {
			out_code(S.LBRACK);  // [ parse attributes start
		}
		out_code(S.RBRACK);    // ] parse attributes end
		
		// 解析xml内容
		if (token == DIV) {      // /  没有内容结束
			if ( !vx_com ) {  // add chileren
				out_code(S.COMMA);    // ,
				out_code(S.LBRACK);   // [
				out_code(S.RBRACK);   // ]
			}
			collapse_scape();
			if (_scanner->next() != GT) { // >  语法错误
				error("Xml Syntax error");
			}
		} else if (token == GT) {       //   >  闭合标签,开始解析内容
			if ( vx_com ) { // view xml component cannot have child views.
				error("View xml component cannot have child views.");
			} else { // 解析子内容以及子节点
				parse_xml_element_context(tag_name);
			}
		} else {
			error("Xml Syntax error");
		}
		
		if (vdata.length()) {
			out_code(S.COMMA);    // ,
			out_code(move(vdata));
		}
		
		if (vx_com) { // __vx(tag,[attrs],vdata)
			out_code(S.RPAREN); // )
		} else {      // {vx:0,v:[tag,[attrs],[child],vdata]}
			out_code(S.RBRACK); // ]
			out_code(S.RBRACE); // }
		}
	}
	
	void parse_xml_attribute_data_bind(bool once) {
		out_code(S.NUMBER_3);  // 2
		out_code(S.COMMA);     // ,
		next();
		XX_ASSERT(peek() == LBRACE);
		XX_ASSERT(_scanner->string_value().is_empty());
		// %{val}
		out_code(S.DATA_BIND_FUNC); // ($,ctr)=>{return
		next();
		out_code(S.LPAREN);  // (
		parse_brace_expression(LBRACE, RBRACE);
		out_code(S.RPAREN);  // )
		out_code(S.RBRACE);  // }
		if (!once) {
			out_code(S.COMMA);     // ,
			out_code(S.NUMBER_1);  // 1
		}
	}
	
	void complete_xml_content_string(Ucs2StringBuilder& str,
																	 Ucs2StringBuilder& space,
																	 bool& is_once_comma,
																	 bool before_comma, bool ignore_space) 
	{
		if (str.string_length()) {
			Ucs2String s = str.to_basic_string();
			if ( !ignore_space || ! s.is_blank() ) {
				add_xml_children_cut_comma(is_once_comma);
				// {vx:2,v:"s"}
				out_code(S.LBRACE);   // {
				out_code(S.TYPE);     // t
				out_code(S.COLON);    // :
				out_code(S.NUMBER_2); // 2
				out_code(S.COMMA);    // ,
				out_code(S.VALUE2);   // v
				out_code(S.COLON);    // :
				out_code(S.QUOTES);   // "
				out_code(s);
				out_code(S.QUOTES);   // "
				out_code(S.RBRACE);   // }
			}
			str.clear();
		}
		if ( before_comma ) {
			add_xml_children_cut_comma(is_once_comma);
		}
		if ( space.string_length() ) {
			_out->push(move(space));
		}
	}
	
	void add_xml_children_cut_comma(bool& is_once_comma) {
		if (is_once_comma) {
			is_once_comma = false;
		} else {
			out_code(S.COMMA);     // ,
		}
	}
	
	void parse_xml_element_context(cUcs2String& tag_name) {
		XX_ASSERT(_scanner->token() == GT);  // >
		
		// add chileren
		out_code(S.COMMA);    // ,
		collapse_scape();
		out_code(S.LBRACK);   // [
		// out_code(S.CHILDS_COMMENT); // add comment
		
		Token token;// prev = ILLEGAL;
		Ucs2StringBuilder str, scape;
		bool ignore_space = true;
		uint pos = _scanner->location().end_pos;
		bool is_once_comma = true;
		
		while(true) {
			token = _scanner->scan_xml_content(ignore_space, pos);
			pos = _scanner->next_location().end_pos;

			switch (token) {
				case XML_COMMENT:    // <!-- comment -->
					/* ignore comment */
					scape.push(S.XML_COMMENT);
					scape.push(_scanner->next_string_value());
					scape.push(S.XML_COMMENT_END);
					break;
					
				case XML_ELEMENT_TAG: // <xml
					complete_xml_content_string(str, scape, is_once_comma, true, ignore_space);
					_scanner->next();
					parse_xml_element(true);
					pos = _scanner->location().end_pos;
					break;
					
				case XML_ELEMENT_TAG_END:    // </xml>
					complete_xml_content_string(str, scape, is_once_comma, false, ignore_space);
					if (tag_name != _scanner->next_string_value()) {
						error(String::format("Xml Syntax error, The end of the unknown, <%s> ... </%s>",
																 *to_utf8_string(tag_name),
																 *to_utf8_string(_scanner->next_string_value())) );
					}
					out_code(S.RBRACK);     // ]
					_scanner->next();
					return;
					
				case XML_NO_IGNORE_SPACE: // @@
					complete_xml_content_string(str, scape, is_once_comma, false, ignore_space);
					ignore_space = !ignore_space;
					break;
					
				case LT: // <
					str.push(_scanner->next_string_value());
					break;
					
				case COMMAND_DATA_BIND: // %%{command}
				case COMMAND_DATA_BIND_ONCE: // %{command}
					complete_xml_content_string(str, scape, is_once_comma, true, ignore_space);
					_scanner->next();     // command %% or %
					_scanner->next();     // next {
					out_code(S.LBRACE);    // {
					out_code(S.TYPE);      // t
					out_code(S.COLON);     // :
					out_code(S.NUMBER_3);  // 3
					out_code(S.COMMA);     // ,
					out_code(S.VALUE2);    // v
					out_code(S.COLON);     // :
					out_code(S.DATA_BIND_FUNC);      // ($,ctr)=>{return
					out_code(S.LPAREN);  // (
					parse_brace_expression(LBRACE, RBRACE); //
					out_code(S.RPAREN);  // )
					out_code(S.RBRACE);  // }
					if (token == COMMAND_DATA_BIND) { // MULTIPLE
						out_code(S.COMMA);     // ,
						out_code(S.MULTIPLE);  // m
						out_code(S.COLON);     // :
						out_code(S.NUMBER_1);  // 1
					}
					out_code(S.RBRACE);  // }
					pos = _scanner->location().end_pos;
					break;
					
				case COMMAND: // ${command}
					complete_xml_content_string(str, scape, is_once_comma, true, ignore_space);
					_scanner->next();     // command ${
					_scanner->next();     // next {
					out_code(S.LPAREN);    // (
					parse_brace_expression(LBRACE, RBRACE); //
					out_code(S.RPAREN);    // )
					pos = _scanner->location().end_pos;
					break;
					
				case STRIXX_LITERAL:   // xml context text
					if (!_scanner->next_string_space().is_empty())
						scape.push(_scanner->next_string_space());
					str.push(_scanner->next_string_value());
					break;
					
				default:
					error("Xml Syntax error", _scanner->next_location());
					break;
			}
		}
	}
	
	Buffer to_utf8_string(cUcs2String s) {
		return Coder::encoding(Encoding::utf8, s);
	}
	
	void error() {
		error("SyntaxError: Unexpected token", _scanner->location());
	}
	
	void error(cString& msg) {
		error(msg, _scanner->location());
	}
	
	void error(cString& msg, Scanner::Location loc) {
		XX_THROW(ERR_SYNTAX_ERROR,
						 "%s\nline:%d, pos:%d, %s",
						 *msg, loc.line + 1, loc.end_pos, *_path);
	}
	
	Token next() {
		Token tok = _scanner->next();
		collapse_scape();
		return tok;
	}
	
	Token token() {
		return _scanner->token();
	}
	
	Token peek() {
		return _scanner->peek();
	}
	
	void collapse_scape() {
		if (!_scanner->string_space().is_empty()) {
			if ( _is_class_member_data_expression ) {
				_out_class_member_data_expression.push(_scanner->string_space());
			}
			_top_out.push(move(_scanner->string_space()));
		}
	}
	
	void fetch_code() {
		collapse_scape();
		out_code(_scanner->string_value());
	}
	
	void out_code(cUcs2String& code) {
		if ( code.length() ) {
			if ( _is_class_member_data_expression ) {
				_out_class_member_data_expression.push(code);
			}
			_out->push(code);
		}
	}
	
	void out_code(Ucs2String&& code) {
		if ( code.length() ) {
			if ( _is_class_member_data_expression ) {
				_out_class_member_data_expression.push(code);
			}
			_out->push(move(code));
		}
	}
	
	void out_code(Ucs2StringBuilder&& code) {
		if ( code.string_length() ) {
			if ( _is_class_member_data_expression ) {
				_out_class_member_data_expression.push(code);
			}
			_out->push(move(code));
		}
	}
	
	struct MemberDataExpression {
		Ucs2String class_name;
		Map<Ucs2String, Ucs2StringBuilder> expressions;
	};
	
	Scanner*          _scanner;
	Ucs2StringBuilder* _out;
	Ucs2StringBuilder _top_out;
	Ucs2StringBuilder _out_class_member_data_expression;
	cString&          _path;
	Array<Ucs2String> _exports;
	Ucs2String        _export_default;
	Array<MemberDataExpression> _class_member_data_expression;
	uint _level;
	bool _is_jsx;
	bool _is_class_member_data_expression;
	bool _is_xml_attribute_expression;
	bool _single_if_expression_before;
	bool _has_export_default;
	bool _clean_comment;
};

Ucs2String javascript_transform_x(cUcs2String& in, cString& path, bool clean_comment) throw(Error) {
	return Parser(in, path, true, clean_comment).transform();
}

Ucs2String javascript_transform(cUcs2String& in, cString& path, bool clean_comment) throw(Error) {
	return Parser(in, path, false, clean_comment).transform();
}

XX_END
