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

#include "./jsx.h"
#include "flare/util/dict.h"
#include "flare/util/fs.h"
#include "flare/util/codec.h"

namespace flare {

	#ifdef CHECK
	# undef CHECK
	#endif

	#define UNEXPECTED_TOKEN_ERROR() error()

	#define CHECK_NEXT(tok, ...) if (next() != tok) error(__VA_ARGS__)

	#define CHECK_TOKEN(tok, ...) if (_scanner->token() != tok) error(__VA_ARGS__)

	#define CHECK_PEEK(tok, ...) if (_scanner->peek() != tok) error(__VA_ARGS__)

	#define CHECK(con, ...) if(!(con)) error(__VA_ARGS__)

	#define DEF_STATIC_STR_LIST(F) \
		F(SPACE, " ") \
		F(INDENT, "  ") \
		F(LT, "<") \
		F(GT, ">") \
		F(ADD, "+") \
		F(SUB, "-") \
		F(DIV, "/") \
		F(ASSIGN, "=") \
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
		F(PERIOD, ".") \
		F(COMMAND, "${") \
		F(LBRACE, "{") \
		F(RBRACE, "}") \
		F(LBRACK, "[") \
		F(RBRACK, "]") \
		F(LPAREN, "(") \
		F(RPAREN, ")") \
		F(CONDITIONAL, "?") \
		F(NOT, "!") \
		F(BIT_OR, "|") \
		F(BIT_NOT, "~") \
		F(BIT_XOR, "^") \
		F(MUL, "*") \
		F(POWER, "**") \
		F(BIT_AND, "&") \
		F(MOD, "%") \
		F(AT, "@") \
		F(QUOTES, "\"") \
		F(NEWLINE, "\n") \
		F(CONST, "const"); /*const*/ \
		F(VAR, "var");     /*var*/ \
		F(REQUIRE, "require");   /*require*/ \
		F(COMMA, ",") \
		F(COLON, ":") \
		F(SEMICOLON, ";") \
		F(XML_COMMENT, "/***") \
		F(XML_COMMENT_END, "**/") \
		F(COMMENT, "/*") \
		F(COMMENT_END, "*/") \
		F(EXPORT_COMMENT, "/*export*/") \
		F(EXPORTS, "exports") \
		F(EXPORT_DEFAULT, "exports.default") \
		F(MODULE_EXPORT, "module._export") \
		F(DEFAULT, "default") \
		F(EXTEND, "Object.assign") \
		F(PROTOTYPE, "prototype") \
		F(NUMBER_0, "0") \
		F(NUMBER_1, "1") \
		F(NUMBER_2, "2") \
		F(NUMBER_3, "3") \
		F(STATIC, "static") \
		F(JSX_HEADER, "const { _VV, _VVT, _VVD } = require('flare/ctr');") \
		F(_VV, "_VV") \
		F(_VVT, "_VVT") \
		F(_VVD, "_VVD") \
		F(AS, "as") \
		F(ARROW, "=>") \

	struct static_str_list_t {
#define F(N,V) String16 N = String16::format("%s", V);
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
		ASSERT(lower_limit <= higher_limit);
		return (uint32_t)(value - lower_limit) <= (uint32_t)(higher_limit - lower_limit);
	}

	static inline bool is_decimal_digit(int c) {
		// ECMA-262, 3rd, 7.8.3 (p 16)
		return is_in_range(c, '0', '9');
	}

	static inline bool is_hex_digit(int c) {
		// ECMA-262, 3rd, 7.6 (p 15)
		return is_decimal_digit(c) || is_in_range(ascii_alpha_to_lower(c), 'a', 'f');
	}

	static inline bool is_int(int64_t i) {
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
		return is_in_range(ascii_alpha_to_lower(c), 'a', 'z');
	}

	static inline bool is_identifier_part(int c) {
		return is_identifier_start(c) || is_decimal_digit(c);
	}

	static inline int64_t int64_multiplication(int64_t i, int multiple, int add) {
		
		double f = 1.0 * i / Int64::limit_max;
		
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
		STRIFX_LITERAL,         // string
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
		/* Other */
		SHELL_HEADER,           // !#/bin/sh
		AT,                     // @
		XML_ELEMENT_TAG,        // <xml
		XML_ELEMENT_TAG_END,    // </xml>
		XML_COMMENT,            // <!-- comment -->
		COMMAND,                // `str ${
		COMMAND_END,            //  str`
		ARROW,									// =>
	};

	inline static bool isAssignmentOp(Token tok) {
		return ASSIGN <= tok && tok <= ASSIGN_MOD;
	}

	inline static bool isBinaryOrCompareOp(Token op){
		return OR <= op && op <= IN;
	}

	/**
	* @class Scanner jsx 词法分析器
	*/
	class Scanner : public Object {
	public:
		
		Scanner(const uint16_t* code, uint32_t size, bool clean_comment)
		: code_(code), size_(size)
		, pos_(0), line_(0)
		, current_(new TokenDesc())
		, next_(new TokenDesc()), clean_comment_(clean_comment)
		{ //
			c0_ = size_ == 0 ? -1: *code_;
			strip_bom();
			current_->token = ILLEGAL;
			
			// scan shell header
			// #!/bin/sh
			if ( c0_ == '#' ) {
				advance();
				if ( c0_ == '!' ) {
					next_->token = SHELL_HEADER;
					next_->string_value.append('#');
					do {
						next_->string_value.append(c0_);
						advance();
					} while(c0_ != '\n' && c0_ != EOS);
					if (c0_ == '\n') {
						advance();
						next_->string_value.append('\n');
					}
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
			uint32_t beg_pos;
			uint32_t end_pos;
			uint32_t line;
		};
		
		Token next() {
			
			// Table of one-character tokens, by character (0x00..0x7f only).
			static uint8_t one_char_tokens[] = {
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
			
			TokenDesc* cur = current_;
			prev_ = cur->token;
			current_ = next_;
			next_ = cur;
			next_->location.beg_pos = pos_;
			next_->location.line = line_;
			next_->string_space = String16();
			next_->string_value = String16();
			next_->before_line_feed = false;
			
			if ((uint32_t)c0_ <= 0x7f) {
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
		
		Token scan_xml_content(uint32_t pos, bool& ignore_space) {
			if ( !set_pos(pos) ) {
				return ILLEGAL;
			}
			
			next_->location.beg_pos = pos_;
			next_->location.line = line_;
			next_->string_value = next_->string_space = String16();
			
			Token token = ILLEGAL;
			
			if (c0_ == '<') {
				// <!-- xml comment --> OR <xml OR </xml> OR < <= << <<=
				token = scan_lt_and_xml_element();
			}
			else if (c0_ == '{') {  // jsx xml command block
				token = COMMAND;
			}
			else { // string
				token = scan_xml_content_string(ignore_space);
			}
			next_->location.end_pos = pos_;
			next_->token = token;

			return token;
		}
		
		Token scan_regexp_content(uint32_t pos) {
			if ( !set_pos(pos) ) {
				return ILLEGAL;
			}
			
			Token token = REGEXP_LITERAL;
			next_->location.beg_pos = pos_;
			next_->location.line = line_;
			next_->string_value = next_->string_space = String16();
			
			ASSERT(c0_ == '/');
			
			advance();
			if (c0_ == '/' || c0_ == '*') { // ILLEGAL
				token = ILLEGAL;
			} else {
				next_->string_value.append('/');
				
				bool is_LBRACK = false; // [
				
				do {
					next_->string_value.append(c0_);
					if ( c0_ == '\\' ) { // 正则转义符
						advance();
						if (c0_ < 0) break;
						next_->string_value.append(c0_);
					} else if ( c0_ == '[' ) {
						is_LBRACK = true;
					} else if ( c0_ == ']' ) {
						is_LBRACK = false;
					}
					advance();
				} while( c0_ >= 0  && (is_LBRACK || c0_ != '/') && !is_line_terminator(c0_) );
				
				if (c0_ == '/') {
					next_->string_value.append('/');
					
					int i = 0, m = 0, g = 0, y = 0, u = 0;
					
					while (true) { // regexp flags
						advance();
						if (c0_ == 'i') {
							if (i) break; else { i = 1; next_->string_value.append('i'); }
						} else if (c0_ == 'm') {
							if (m) break; else { m = 1; next_->string_value.append('m'); }
						} else if (c0_ == 'g') {
							if (g) break; else { g = 1; next_->string_value.append('g'); }
						} else if (c0_ == 'y') {
							if (y) break; else { y = 1; next_->string_value.append('y'); }
						} else if (c0_ == 'u') {
							if (u) break; else { u = 1; next_->string_value.append('u'); }
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
		
		Token scan_command_string(uint32_t pos) {
			if ( ! set_pos(pos) ) {
				return ILLEGAL;
			}
			
			Token token = COMMAND_END;
			next_->location.beg_pos = pos_;
			next_->location.line = line_;
			
			while ( c0_ != '`' && c0_ >= 0 ) {
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
					next_->string_space.append(c);
				}
				next_->string_value.append(c);
			}

			if (c0_ == '`') {
				advance();  // consume quote
				next_->string_value.append('`');
			} else if (c0_ != '{') { // err
				token = ILLEGAL;
			}
			
			next_->location.end_pos = pos_;
			next_->token = token;

			return token;
		}
		
		inline Token        token()             { return current_->token; }
		inline Location     location()          { return current_->location; }
		inline String16&  string_space()      { return current_->string_space; }
		inline String16&  string_value()      { return current_->string_value; }
		inline bool         has_scape_before()  { return current_->before_scape; }
		inline bool         before_line_feed()  { return current_->before_line_feed; }
		inline Token        prev()              { return prev_; }
		inline Token        peek()              { return next_->token; }
		inline Location     next_location()     { return next_->location; }
		inline String16&  next_string_space() { return next_->string_space; }
		inline String16&  next_string_value() { return next_->string_value; }
		inline bool         next_before_line_feed() { return next_->before_line_feed; }
		inline bool         has_scape_before_next() { return next_->before_scape; }
		
	private:

		struct TokenDesc {
			Token token;
			Location location;
			String16 string_space;
			String16 string_value;
			bool before_line_feed;
			bool before_scape;
		};
		
		void scan() { // scan javascript code
			
			Token token;
			next_->string_value = String16();
			next_->string_space = String16();
			next_->before_line_feed = false;
			next_->before_scape = false;
			
			do {
				// Remember the position of the next token
				next_->location.beg_pos = pos_;
				next_->location.line = line_;
				
				switch (c0_) {
					case ' ':
					case '\t':
						next_->string_space.append(c0_);
						next_->before_scape = true;
						advance();
						token = Token::WHITESPACE;
						break;
						
					case '\n':
						next_->string_space.append(c0_);
						next_->before_scape = true;
						next_->before_line_feed = true;
						advance();
						token = WHITESPACE;
						break;
						
					case '"':
					case '\'':
						token = scan_string();
						break;
						
					case '`': // 检查指令字符串内部是否有 `str ${
						advance();
						next_->string_value.append('`');
						token = scan_command_string(pos_);
						if ( token == COMMAND_END ) {
							token = STRIFX_LITERAL;
						}
						break;
						
					case '%':
						// % %=
						advance();
						token = select('=', ASSIGN_MOD, MOD);
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
						// = => == ===
						advance();
						if (c0_ == '>') { // =>
							token = select(ARROW);
						} else if (c0_ == '=') {
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
							next_->before_scape = true;
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
				next_->string_value.append(c0_);
				advance();
			} while(is_identifier_part(c0_));
			
			const int input_length = next_->string_value.length();
			const uint16_t* input = next_->string_value.c_str();
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
		ASSERT(keyword_length >= kMinLength);               \
		ASSERT(keyword_length <= kMaxLength);               \
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
										next_->string_value.append('-');
										next_->string_value.append('-');
										if (clean_comment_) {
											if (c0_ == '\n')
												next_->string_value.append(c0_);
										}
										else
											next_->string_value.append(c0_ == '*' ? 'x' : c0_);
									}
								} else break;
							} else {
								next_->string_value.append('-');
								if (clean_comment_) {
									if (c0_ == '\n')
										next_->string_value.append(c0_);
								}
								else
									next_->string_value.append(c0_ == '*' ? 'x' : c0_);
							}
						} else break;
					} else {
						if (clean_comment_) {
							if (c0_ == '\n')
								next_->string_value.append(c0_);
						}
						else
							next_->string_value.append(c0_ == '*' ? 'x' : c0_);
					}
					advance();
				}
				
				// Unterminated multi-line comment.
				return ILLEGAL;
			}
			else if ( is_xml_element_start(c0_) ) { // <xml
				
				scan_xml_tag_identifier();
				
				int c = c0_;
				advance();
				
				if (c == ':' && is_xml_element_start(c0_)) {
					next_->string_value.append(':');
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
						next_->string_value.append(':');
						
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
					next_->string_value.append('.');
					scan_identifier();
				} else {
					back();
					break;
				}
			}
		}
		
		Token scan_xml_content_string(bool& ignore_space) {
			do {
				if ( ignore_space && skip_white_space(true) ) {
					next_->string_value.append(' ');
				}
				if (c0_ == '\\') {
					advance();
					if (c0_ < 0 || !scan_string_escape()) return ILLEGAL;
				} else if (is_line_terminator(c0_)) {
					next_->string_space.append( c0_ );
					// Allow CR+LF newlines in multiline string literals.
					if (is_carriage_return(c0_) && is_line_feed(c0_)) advance();
					// Allow LF+CR newlines in multiline string literals.
					if (is_line_feed(c0_) && is_carriage_return(c0_)) advance();
					next_->string_value.append('\\');
					next_->string_value.append('n');
					advance();
				} else if (c0_ == '"') {
					next_->string_value.append('\\'); // 转义
					next_->string_value.append('"');
					advance();
				}
				else if (c0_ == '`') {
					advance();
					ignore_space = !ignore_space;
				} 
				else if (c0_ == '<' || c0_ == '{') {
					break;
				} else {
					next_->string_value.append(c0_);
					advance();
				}
			} while(c0_ >= 0);
			
			return STRIFX_LITERAL;
		}
		
		Token skip_multi_line_comment() {
			advance();

			if (!clean_comment_) {
				next_->string_space.append('/');
				next_->string_space.append('*');
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
						next_->string_space.append('*');
						next_->string_space.append('/');
					}
					return WHITESPACE;
				} else {
					if (clean_comment_) {
						if (ch == '\n')
							next_->string_space.append(ch);
					}
					else
						next_->string_space.append(ch);
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
				next_->string_space.append('/');
				next_->string_space.append('/');
			}
			while (c0_ >= 0 && !is_line_terminator(c0_)) {
				if (!clean_comment_)
					next_->string_space.append(c0_);
				advance();
			}
			return WHITESPACE;
		}
		
		Token scan_string() {
			uint8_t quote = c0_;
			next_->string_value = String16();
			advance();  // consume quote
			next_->string_value.append(quote);
			
			while (c0_ != quote && c0_ >= 0 && !is_line_terminator(c0_)) {
				int c = c0_;
				advance();
				if (c == '\\') {
					if (c0_ < 0 || !scan_string_escape()) return ILLEGAL;
				} else {
					next_->string_value.append(c);
				}
			}
			if (c0_ != quote) return ILLEGAL;
			
			next_->string_value.append(quote);
			
			advance();  // consume quote
			return STRIFX_LITERAL;
		}
		
		// 扫描字符串转义
		bool scan_string_escape() {
			int c = c0_;
			advance();
			
			next_->string_value.append('\\');
			// Skip escaped newlines.
			if ( is_line_terminator(c) ) {
				// Allow CR+LF newlines in multiline string literals.
				if (is_carriage_return(c) && is_line_feed(c0_)) advance();
				// Allow LF+CR newlines in multiline string literals.
				if (is_line_feed(c) && is_carriage_return(c0_)) advance();
				next_->string_value.append('\n');
				return true;
			}
			if (c == 'u') {
				if (scan_hex_number(4) == -1) return false;
				next_->string_value.append(&code_[pos_ - 5], 5);
				return true;
			}
			next_->string_value.append(c);
			return true;
		}
		
		int scan_hex_number(int expected_length) {
			ASSERT(expected_length <= 4);  // prevent overflow
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
			next_->string_value = String16();
			
			if (seen_period) { // 浮点
				tok = scan_decimal_digit(true);
			}
			else if (c0_ == '0') {
				advance();
				next_->string_value.assign('0');
				
				if (c0_ < 0) { // 结束,10进制 0
					return tok;
				}
				switch (c0_) {
					case 'b': case 'B': // 0b 2进制
						next_->string_value.append(c0_);
						advance();
						if (is_binary_digit(c0_)) {
							do {
								next_->string_value.append(c0_);
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
						next_->string_value.append(c0_);
						advance();
						if (is_hex_digit(c0_)) {
							do {
								next_->string_value.append(c0_);
								advance();
							} while(is_hex_digit(c0_));
						} else {
							return ILLEGAL;
						}
						break;
						
					case '.': // 10进制浮点数
						back();
						next_->string_value = String16();
						tok = scan_decimal_digit(false);
						break;
						
					default:
						if (is_octal_digit(c0_)) { // 0 8进制
							do {
								next_->string_value.append(c0_);
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
				next_->string_value.append('.');
				do {
					next_->string_value.append(c0_);
					advance();
				} while(is_decimal_digit(c0_));
			}
			else {
				while (is_decimal_digit(c0_)) { // 整数
					next_->string_value.append(c0_);
					advance();
				}
				
				if (c0_ == '.') { // 浮点数
					next_->string_value.append(c0_);
					advance();
					while (is_decimal_digit(c0_)) {
						next_->string_value.append(c0_);
						advance();
					}
				}
			}
			
			// int i = 1.9e-2;  科学记数法
			if (c0_ == 'e' || c0_ == 'E') {
				next_->string_value.append(c0_);
				advance();
				
				if (c0_ == '+' || c0_ == '-') {
					next_->string_value.append(c0_);
					advance();
				}
				
				if (is_decimal_digit(c0_)) {
					do {
						next_->string_value.append(c0_);
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
			uint32_t start_position = pos_;
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
								next_->string_space.append(c0_);
							}
						} else {
							next_->string_space.append(c0_);
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
		
		bool set_pos(uint32_t pos) {
			ASSERT(pos >= 0);
			
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
				next_->location = { pos_, pos_, line_ };
				next_->token = ILLEGAL;
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
		
		const uint16_t *code_;
		uint32_t size_, pos_, line_;
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
		
		Parser(cString16& in, cString& path, bool is_jsx, bool clean_comment)
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
		
		String16 transform() {
			parse_document();
			return *_out;
		}
		
	private:
		
		void parse_document() {
			
			if ( peek() == SHELL_HEADER ) {
				next();
				fetch(); // #!/bin/sh
			}

			if (_is_jsx) {
				// add jsx header code
				// import { _VV, _VVT, _VVD } from 'flare/ctr';
				append(S.JSX_HEADER);
			}

			Token tok = next();
			while (tok != EOS) {
				parse_advance();
				tok = next();
			}

			// parse end
			// class member data
			for ( auto& i : _class_member_data_expression ) {
				if ( i.expressions.length() ) {
					append(S.NEWLINE);   // \n
					append(S.EXTEND);   // Object.assign
					append(S.LPAREN);    // (
					append(i.class_name);    // class_name
					append(S.PERIOD);    // .
					append(S.PROTOTYPE);  // prototype
					append(S.COMMA);     // ,
					append(S.LBRACE);    // {
					append(S.NEWLINE);   // \n
					for ( auto& j : i.expressions ) {
						append(S.INDENT);    // \t
						append(j.key);    // identifier
						append(S.COLON);     // :
						append(j.value);  // expression
						append(S.COMMA);     // ,
						append(S.NEWLINE);   // \n
					}
					append(S.RBRACE);    // }
					append(S.RPAREN);    // )
					append(S.SEMICOLON); // ;
				}
			}
			
			// export
			for (uint32_t i = 0; i < _exports.length(); i++) {
				append(S.NEWLINE);
				append(S.EXPORTS);    // exports.xxx=xxx;
				append(S.PERIOD);     // .
				append(_exports[i]); // xxx
				append(S.ASSIGN);     // =
				append(_exports[i]); // xxx
				append(S.SEMICOLON);  // ;
			}
			
			// export default
			if (_has_export_default && !_export_default.is_empty()) {
				append(S.NEWLINE);
				append(S.EXPORT_DEFAULT);  // exports.default=xxx;
				append(S.ASSIGN);          // =
				append(_export_default);  // xxx
				append(S.SEMICOLON);       // ;
			}
			
			append(S.NEWLINE);
		}

		void parse_function() {
			ASSERT(token() == FUNCTION);
			// function f(arg) { block }
			fetch(); // function
			if ( is_declaration_identifier(next()) ) {
				fetch(); // identifier
				next();
			}
			CHECK_TOKEN(LPAREN); // (
			append(S.LPAREN);  // (
			parse_brace_expression(LPAREN, RPAREN); // ()
			append(S.RPAREN);  // (
			CHECK_NEXT(LBRACE); // {
			append(S.LBRACE);  // {
			parse_brace_expression(LBRACE, RBRACE); // {}
			append(S.RBRACE);  // }
		}

		void parse_arrow_function() {
			ASSERT(token() == ARROW);
			// TODO ...
		}

		void parse_advance() {

			switch (token()) {
				case EXPORT:
					parse_export(); break;
				case WHITESPACE:  // space
				case INSTANCEOF:  // instanceof
				case TYPEOF:  // typeof
				case IN:  // in			
				case RETURN:  // return
				case VAR: // var
				case LET: // let
				case DEFAULT: // default
				case CONST: // const
				case ELSE:  // else
				case NUMBER_LITERAL:  // number
				case STRIFX_LITERAL:  // string
					fetch();
					break;
				case IDENTIFIER:  // identifier
				case AS:  // as
				case OF:  // of
				case FROM:  // from
				case EVENT: // event
				case GET: // get
				case SET: // set
					fetch();
					break;
				case ARROW: // =>
					append(S.ARROW);
					break;
				case ASYNC: // async
					fetch();
					break;
				case CLASS: // class
					if ( _scanner->prev() == PERIOD ) { // .class
						fetch();
					} else {
						parse_class(); // class xxxx { }
					}
					break;
				case IF:  // if
					fetch();
					CHECK_NEXT(LPAREN);        // (
					append(S.LPAREN);
					parse_brace_expression(LPAREN, RPAREN);
					append(S.RPAREN);
					if ( peek() != LBRACE ) { // {
						next();
						_single_if_expression_before = true;
						parse_advance();
						_single_if_expression_before = false;
					}
					break;
				case FUNCTION:  // function
					parse_function(); break;
				case LT:                      // <
					append(S.LT); break;
				case GT:                      // >
					append(S.GT); break;
				case ASSIGN:                  // =
					append(S.ASSIGN); break;
				case PERIOD:                  // .
					append(S.PERIOD); break;
				case ADD:                     // +
					append(S.ADD); break;
				case SUB:                     // -
					append(S.SUB); break;
				case COMMA:                   // ,
					append(S.COMMA); break;
				case COLON:                   // :
					append(S.COLON); break;
				case SEMICOLON:               // ;
					append(S.SEMICOLON); break;
				case CONDITIONAL:             // ?
					append(S.CONDITIONAL); break;
				case NOT:                     // !
					append(S.NOT); break;
				case BIT_OR:                  // |
					append(S.BIT_OR); break;
				case BIT_NOT:                 // ~
					append(S.BIT_NOT); break;
				case BIT_XOR:                 // ^
					append(S.BIT_XOR); break;
				case MUL:                     // *
					append(S.MUL); break;
				case POWER:                   // **
					append(S.POWER); break;
				case BIT_AND:                 // &
					append(S.BIT_AND); break;
				case MOD:                     // %
					append(S.MOD); break;
				case INC:                    // ++
					append(S.INC); break;
				case DEC:                    // --
					append(S.DEC); break;
				case ASSIGN_BIT_OR:          // |=
					append(S.ASSIGN_BIT_OR); break;
				case ASSIGN_BIT_XOR:         // ^=
					append(S.ASSIGN_BIT_XOR); break;
				case ASSIGN_BIT_AND:         // &=
					append(S.ASSIGN_BIT_AND); break;
				case ASSIGN_SHL:             // <<=
					append(S.ASSIGN_SHL); break;
				case ASSIGN_SAR:             // >>=
					append(S.ASSIGN_SAR); break;
				case ASSIGN_SHR:             // >>>=
					append(S.ASSIGN_SHR); break;
				case ASSIGN_ADD:             // +=
					append(S.ASSIGN_ADD); break;
				case ASSIGN_SUB:             // -=
					append(S.ASSIGN_SUB); break;
				case ASSIGN_MUL:             // *=
					append(S.ASSIGN_MUL); break;
				case ASSIGN_POWER:           // **=
					append(S.ASSIGN_POWER); break;
				case ASSIGN_MOD:             // %=
					append(S.ASSIGN_MOD); break;
				case OR:                     // ||
					append(S.OR); break;
				case AND:                    // &&
					append(S.AND); break;
				case SHL:                    // <<
					append(S.SHL); break;
				case SAR:                    // >>
					append(S.SAR); break;
				case SHR:                    // >>>
					append(S.SHR); break;
				case EQ:                     // ==
					append(S.EQ); break;
				case NE:                     // !=
					append(S.NE); break;
				case EQ_STRICT:              // ===
					append(S.EQ_STRICT); break;
				case NE_STRICT:              // !==
					append(S.NE_STRICT); break;
				case LTE:                    // <=
					append(S.LTE); break;
				case GTE:                    // >=
					append(S.GTE); break;
				case XML_ELEMENT_TAG:         // <xml
					if ( _is_xml_attribute_expression ) {
						UNEXPECTED_TOKEN_ERROR();
					} else {
						parse_xml_element(false);
					}
					break;
				case XML_COMMENT:             // <!-- comment -->
					if (_is_jsx && !_is_xml_attribute_expression) {
						append(S.XML_COMMENT);
						append(_scanner->string_value());
						append(S.XML_COMMENT_END);
					} else {
						UNEXPECTED_TOKEN_ERROR();
					}
					break;
				case LPAREN:                  // (
					append(S.LPAREN);
					parse_brace_expression(LPAREN, RPAREN);
					append(S.RPAREN);
					break;
				case LBRACK:                  // [
					append(S.LBRACK);
					parse_brace_expression(LBRACK, RBRACK);
					append(S.RBRACK);
					break;
				case LBRACE:                  // {
					append(S.LBRACE);
					parse_brace_expression(LBRACE, RBRACE);
					append(S.RBRACE);
					break;
				case COMMAND:                 // `str ${
					parse_command_string(); break;
				case IMPORT:                  // import
					parse_import(); break;
				case DIV:                     // / OR regexp
				case ASSIGN_DIV:              // /=
					if (is_legal_literal_begin(false)) {
						parse_regexp_expression();
					} else {
						if (token() == DIV) {
							append(S.DIV);
						} else {
							append(S.ASSIGN_DIV);
						}
					}
					break;
				case ILLEGAL:                 // illegal
				default: UNEXPECTED_TOKEN_ERROR(); break;
			}
		}

		void parse_brace_expression(Token begin, Token end) { // 括号表达式
			ASSERT( token() == begin );
			uint32_t level = _level;
			_level++;

			while(true) {
				if (next() == end) {
					break;
				} else if (token() == EOS) {
					UNEXPECTED_TOKEN_ERROR();
				} else {
					parse_advance();
				}
			}
			_level--;
			if ( _level != level ) {
				UNEXPECTED_TOKEN_ERROR();
			}
		}
		
		void parse_expression() { // 表达式
			parse_single_expression();
			parse_operation_expression();
		}

		void parse_single_expression() { // 简单表达式

			Token tok = next();

			if ( is_declaration_identifier(tok) ) {
				parse_variable_visit_expression();
				parse_identifier_expression();
				return;
			}

			switch (tok) {
				// case ASYNC:
				case FUNCTION:
					parse_function();
					break;
				case NUMBER_LITERAL: // 10
				case STRIFX_LITERAL: // "String"
					fetch();
					break;
				case COMMAND:     // `str ${
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
							_out->append(S.COMMENT_END); // */
						}
						append(S.XML_COMMENT);
						append(_scanner->string_value());
						append(S.XML_COMMENT_END);
						if ( _is_class_member_data_expression ) { // 重新开始多行注释
							_out->append(S.COMMENT); // /*
						}
					} else {
						UNEXPECTED_TOKEN_ERROR();
					}
					break;
				case LPAREN:                  // (
					append(S.LPAREN);
					parse_brace_expression(LPAREN, RPAREN);
					append(S.RPAREN);
					break;
				case LBRACK:                  // [
					append(S.LBRACK);
					parse_brace_expression(LBRACK, RBRACK);
					append(S.RBRACK);
					break;
				case LBRACE:                  // {
					append(S.LBRACE);
					parse_brace_expression(LBRACE, RBRACE);
					append(S.RBRACE);
					break;
				case INC:            // ++
				case DEC:            // --
					if ( is_declaration_identifier(peek()) ) {
						append(tok == INC ? S.INC : S.DEC);
						parse_single_expression();
					} else {
						UNEXPECTED_TOKEN_ERROR();
					}
					break;
				case TYPEOF:         // typeof
					fetch(); parse_single_expression(); break;
				case ADD:            // +
					append(S.ADD); parse_single_expression(); break;
				case SUB:            // -
					append(S.SUB); parse_single_expression(); break;
				case NOT:            // !
					append(S.NOT); parse_single_expression(); break;
				case BIT_NOT:        // ~
					append(S.BIT_NOT); parse_single_expression(); break;
				default:
					UNEXPECTED_TOKEN_ERROR(); break;
			}
		}

		void parse_operation_expression() { // 运算符表达式
			
			Token op = peek();
			
			if ( isAssignmentOp(op) ) {
				// ASSIGN,                 // =
				// ASSIGN_BIT_OR,          // |=
				// ASSIGN_BIT_XOR,         // ^=
				// ASSIGN_BIT_AND,         // &=
				// ASSIGN_SHL,             // <<=
				// ASSIGN_SAR,             // >>=
				// ASSIGN_SHR,             // >>>=
				// ASSIGN_ADD,             // +=
				// ASSIGN_SUB,             // -=
				// ASSIGN_MUL,             // *=
				// ASSIGN_POWER,           // **=
				// ASSIGN_DIV,             // /=
				// ASSIGN_MOD,             // %=
				// 无法确定左边表达式是否为一个变量表达式,所以暂时不处理,
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
					
					next(); fetch();
					parse_single_expression();
					op = peek();
				} else {
					
					if ( op == XML_ELEMENT_TAG ) { // <tag
						// parse with compare
						next();
						append(S.LT);  // <
						fetch();   // identifier
						op = peek();
					} else { // 就此终结
						return;
					}
				}
			}
			
		}

		void parse_conditional_expression() { // 三元表达式
			CHECK_NEXT(CONDITIONAL); // ?
			append(S.CONDITIONAL); // ?
			parse_expression();
			CHECK_NEXT(COLON); // :
			append(S.COLON); // ?
			parse_expression();
		}

		void parse_regexp_expression() { // 正则表达式
			ASSERT(_scanner->token() == DIV || _scanner->token() == ASSIGN_DIV);
			
			if (_scanner->scan_regexp_content(_scanner->location().beg_pos) == REGEXP_LITERAL) {
				next();
				fetch();
			} else {
				error("RegExp Syntax error");
			}
		}

		void parse_variable_visit_expression() { // 属性访问表达式
			ASSERT( is_declaration_identifier(token()) );

			fetch(); // identifier

			while(true) {
				switch(peek()) {
					case PERIOD:     // .
						next(); append( S.PERIOD ); // .
						next();
						CHECK( is_declaration_identifier(token()) );
						fetch(); // identifier
						break;
					case LBRACK:     // [
						parse_single_expression();
						break;
					default: return;
				}
			}
		}

		void parse_identifier_expression() { // 
			
			switch (peek()) {
				case INC:    // ++
					next(); append(S.INC);
					return;
				case DEC:    // --
					next(); append(S.DEC);
					return;
				case LPAREN: // ( function call
					break;
				default:
					return;
			}
			
			// parse function call
			
			next();
			append(S.LPAREN); // (
			parse_brace_expression(LPAREN, RPAREN);
			append(S.RPAREN); // )
		}

		void parse_command_string() {
			if ( _scanner->string_value().is_empty()) {
				UNEXPECTED_TOKEN_ERROR();
			}
			while(true) {
				ASSERT(peek() == LBRACE);
				append(_scanner->string_value());
				append(S.COMMAND);  // ${
				// next();
				next();
				parse_command_string_block(); // parse { block }
				ASSERT(peek() == RBRACE);
				append(S.RBRACE);   // }
				_scanner->scan_command_string(_scanner->next_location().end_pos);
				Token tok = next();
				if (tok == COMMAND_END) {
					fetch(); break;
				} else if (tok != COMMAND) {
					UNEXPECTED_TOKEN_ERROR();
				}
			}
		}

		void parse_command_string_block() {
			ASSERT( _scanner->token() == LBRACE );
			while(true) {
				if (peek() == RBRACE)
					break;
				if (next() == EOS) {
					UNEXPECTED_TOKEN_ERROR();
				} else {
					parse_advance();
				}
			}
		}

		String16 to_event_js_code(cString16& name) {
			//  get onchange() { return this.getNoticer('change') }
			//  set onchange(func) { this.addDefaultListener('change', func) }
			//  triggerchange(data, is_event) { return this.$trigger('change', data, is_event) }
			//
			static cString16 a1 = String16::format("get on");
			static cString16 a2 = String16::format("() { return this.getNoticer('");
			static cString16 a3 = String16::format("') }");
			static cString16 b1 = String16::format("set on");
			static cString16 b2 = String16::format("(func) { this.addDefaultListener('");
			static cString16 b3 = String16::format("', func) }");
			static cString16 c1 = String16::format("trigger");
			static cString16 c2 = String16::format("(ev,is_ev) { return this.$trigger('");
			static cString16 c3 = String16::format("',ev,is_ev) }");
			
			String16 rv;
			rv.append(a1); rv.append(name); rv.append(a2); rv.append(name); rv.append(a3);
			rv.append(b1); rv.append(name); rv.append(b2); rv.append(name); rv.append(b3);
			rv.append(c1); rv.append(name); rv.append(c2); rv.append(name); rv.append(c3);
			return rv;
		}

		void parse_class() {
			ASSERT(token() == CLASS);
		
			fetch();
		
			String16 class_name;
			MemberDataExpression* member_data = nullptr;
			
			Token tok = next();
			if (is_declaration_identifier(tok)) {
				fetch();

				class_name = _scanner->string_value();
				
				if ( _level == 0 ) {
					_class_member_data_expression.push({ class_name });
					member_data = &_class_member_data_expression[_class_member_data_expression.length() - 1];
				}
				
				tok = next();
				if (tok == EXTENDS) {
					fetch();
					
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
				fetch();
				// parse_member_data = false; // Anonymous class not parse member data
				CHECK_NEXT(LBRACE); // {
			} else {
				CHECK_TOKEN(LBRACE); // {
			}
			
			ASSERT(_scanner->token() == LBRACE); // {
			
			append(S.LBRACE); // {
			
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
							fetch(); // fetch get or set identifier
							next();	// identifier
							fetch(); // fetch identifier
							if ( next() == LPAREN ) { // (
								append(S.LPAREN); // (
								if ( tok == SET ) {
									if ( is_declaration_identifier(next()) ) { // param identifier
										fetch(); // fetch set param identifier
									} else {
										error("syntax error, define set property accessor");
									}
								}
								if ( next() == RPAREN ) { // )
									append(S.RPAREN); // )
									if ( next() == LBRACE ) {
										append(S.LBRACE); // {
										parse_brace_expression(LBRACE, RBRACE);
										append(S.RBRACE); // }
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
					case STRIFX_LITERAL:
						//class member identifier
						if ( peek() == LPAREN ) { // (
							goto function;
						} else if ( member_data && peek() == ASSIGN ) { // = class member data
							append(S.COMMENT); // /*
							String16 identifier = _scanner->string_value();
							fetch(); // identifier
							next(); // =
							append(S.ASSIGN); // =
							_out_class_member_data_expression = String16();
							_is_class_member_data_expression = true;
							parse_expression();
							_is_class_member_data_expression = false;
							CHECK_NEXT(SEMICOLON); // ;
							member_data->expressions.set(identifier, std::move(_out_class_member_data_expression));
							append(S.COMMENT_END); // */
						} else {
							UNEXPECTED_TOKEN_ERROR();
						}
						break;
					case STATIC: // static
						fetch(); // fetch static
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
						fetch(); // fetch async
						if ( is_class_member_identifier(next()) ) {
							goto function;
						} else if (_scanner->token() != MUL) {
							UNEXPECTED_TOKEN_ERROR();
						}
					case MUL: // * generator function
					mul:
						append(S.MUL);
						if ( !is_class_member_identifier(next()) ) {
							UNEXPECTED_TOKEN_ERROR();
						}
					function:
						fetch(); // fetch function identifier
						if ( next() == LPAREN ) { // arguments
							append(S.LPAREN); // (
							parse_brace_expression(LPAREN, RPAREN);
							append(S.RPAREN); // )
							if ( next() == LBRACE ) { // function body
								append(S.LBRACE); // {
								parse_brace_expression(LBRACE, RBRACE); //
								append(S.RBRACE); // }
								break; // ok
							}
						}
						UNEXPECTED_TOKEN_ERROR();
					case EVENT: {
						// Event declaration
						CHECK_NEXT(IDENTIFIER); // event onevent
						String16 event = _scanner->string_value();
						if (event.length() > 2 &&
								event[0] == 'o' &&
								event[1] == 'n' && is_xml_element_start(event[2])) {
							append(to_event_js_code(event.substr(2)));
							CHECK_NEXT(SEMICOLON); // ;
						} else {
							error("Syntax error, event name incorrect");
						}
						break;
					}
					case SEMICOLON: //;
						append(S.SEMICOLON); break;
					case RBRACE: // }
						goto end;  // Class end
					default:
						UNEXPECTED_TOKEN_ERROR(); break;
				};
			}
			
		end:
			append(S.RBRACE); // {
		}

		void parse_export() {
			ASSERT(_scanner->token() == EXPORT);
			CHECK(_level == 0);
			
			Token tok = next();
			bool has_export_default = false;
			
			if (tok == DEFAULT) {
				if (_has_export_default) {
					UNEXPECTED_TOKEN_ERROR();
				} else {
					_has_export_default = true;
					has_export_default = true;
				}
				tok = next();
			}
			
			switch (tok) {
				case ASYNC:
					append(S.EXPORT_COMMENT);
					fetch();
					CHECK_NEXT(FUNCTION);
					tok = token();
					goto identifier;
				case VAR:       // var
				case CLASS:     // class
				case FUNCTION:  // function
				case LET:       // let
				case CONST:     // const
					append(S.EXPORT_COMMENT);
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
							append(S.EXPORT_DEFAULT); // exports.default = expression
						} else {
							append(S.MODULE_EXPORT);  // module._export = expression
						}
						append(S.ASSIGN); // =
						parse_advance();   // expression
					} else {
						UNEXPECTED_TOKEN_ERROR();
					}
					break;
			}
		}

		void parse_import_block(String16* defaultId) {
			// import { Application } from 'flare/app';
			// import { Application as App } from 'flare/app';
			// import app, { Application as App } from 'flare/app';

			ASSERT(token() == LBRACE);
			append(S.LBRACE);     // {

			if (defaultId) {
				append(S.DEFAULT);  // default
				append(S.COLON);    // :
				append(*defaultId); // app
				append(S.COMMA);    // ,
			}

			while (true) {
				CHECK( is_import_declaration_identifier(next()) );
				fetch();
				if (next() == AS) { // as
					append(S.COLON); // : 
					CHECK( is_import_declaration_identifier(next()) );
					fetch();   // IDENTIFIER
					next();
				}
				if (token() == COMMA) { // ,
					append(S.COMMA);   // ,
					if (peek() == RBRACE) { // }
						next();
						break;
					}
				} else {
					break;
				}
			}

			CHECK_TOKEN(RBRACE); // }
			append(S.RBRACE); // }
		}

		void parse_import() {
			ASSERT(_scanner->token() == IMPORT);
			Token tok = next();

			if (is_import_declaration_identifier(tok)) { // identifier
				append(S.CONST); // const
				String16 id = _scanner->string_value();
				tok = next();
				
				if (tok == FROM) { // import app from 'flare/app';
					append(id);       // app   // TODO ... developer evn modify id
					append(S.ASSIGN); // =
					CHECK_NEXT(STRIFX_LITERAL);
					append(S.REQUIRE); // require('flare/app').default;
					append(S.LPAREN); // (
					fetch();
					append(S.RPAREN); // )
					append(S.PERIOD); // .
					append(S.DEFAULT); // default
				} else if (tok == COMMA) { // import app, { Application as App } from 'flare/app';
					CHECK_NEXT(LBRACE);
					parse_import_block(&id);
					CHECK_NEXT(FROM);
					append(S.ASSIGN); // =
					CHECK_NEXT(STRIFX_LITERAL);
					append(S.REQUIRE); // require('flare/app');
					append(S.LPAREN); // (
					fetch();
					append(S.RPAREN); // )
				}
				else {
					UNEXPECTED_TOKEN_ERROR();
				}
			}
			else if (tok == MUL) {  // import * as app from 'flare/app';
				append(S.CONST);    // const
				CHECK_NEXT(AS);       // as
				tok = next();
				if (is_import_declaration_identifier(tok)) {
					fetch();
					CHECK_NEXT(FROM);
					append(S.ASSIGN);  // =
					CHECK_NEXT(STRIFX_LITERAL);
					append(S.REQUIRE); // require('flare/app');
					append(S.LPAREN); // (
					fetch();
					append(S.RPAREN); // )
				} else {
					UNEXPECTED_TOKEN_ERROR();
				}
			}
			else if (tok == LBRACE) { // import { Application as app } from 'flare/app';
				append(S.CONST); // const
				parse_import_block(nullptr);
				CHECK_NEXT(FROM);  // from
				append(S.ASSIGN); // =
				CHECK_NEXT(STRIFX_LITERAL);
				append(S.REQUIRE);// require('flare/app');
				append(S.LPAREN); // (
				fetch();
				append(S.RPAREN); // )
			}
			else if (tok == STRIFX_LITERAL) { // flare private syntax

				String16 str = _scanner->string_value();
				if (peek() == AS) { // import 'test_gui.jsx' as gui;  ---->>>> import * as gui from 'test_gui.jsx';
					append(S.CONST); // var
					next(); // as
					if ( is_import_declaration_identifier(peek()) ) {
						append(_scanner->next_string_value());
						append(S.SPACE); //
						append(S.ASSIGN); // =
						next(); // IDENTIFIER
						append(S.REQUIRE); // require('flare/app');
						append(S.LPAREN); // (
						append(str);
						append(S.RPAREN); // )
					} else {
						UNEXPECTED_TOKEN_ERROR();
					}
				} else { // import 'test_gui.jsx';   ---->>>>    import * as test_gui from 'test_gui.jsx';
					// find identifier
					String16 path = str.substr(1, str.length() - 2).trim();
					String basename = Path::basename(path.to_string());
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
						append(S.CONST); // const
						append(S.SPACE); //
						append(Coder::decode_to_uint16(Encoding::utf8, basename)); // identifier
						append(S.SPACE); //
						append(S.ASSIGN); // =
					}
					append(S.REQUIRE); // require('flare/app');
					append(S.LPAREN); // (
					append(str);
					append(S.RPAREN); // )
				}
			}
			else { // 这可能是个函数调用,不理会
				// import();
				fetch();
			}
		}

		// 是否为合法字面量开始
		bool is_legal_literal_begin(bool isXml) {
			Token tok = _scanner->prev();

			if ( isXml ) {
				switch (tok) {
					// 这是一个合法js语法，但不是一个合法的xml字面量的开始
					case INC:         // if (i++<a.length) {}
					case DEC:         // if (i--<a.length) {}
						return false;
					default: break;
				}
			}

			// 上一个词为这些时,下一个词可以为字面量(literal expression)

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
				case STRIFX_LITERAL:
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

		bool is_xml_element_legal_start(bool inXml/*是否在xml内部*/) {
			if (!_is_jsx)
				return false;

			if (!inXml) { // 非xml内部
				if (!is_legal_literal_begin(true)) // 
					return false;
			}

			Token token = peek();

			if (token == PERIOD || /* . */ token == COLON /* : */) { // not xml
				return false;
			}

			return true;
		}
		
		void parse_xml_element(bool inXml) {
			ASSERT(_scanner->token() == XML_ELEMENT_TAG);

			if ( !is_xml_element_legal_start(inXml) ) { // 是否为合法的xml开始
				append(S.LT);
				fetch();
				return;
			}
		
			// 转换xml为json对像: _VV(Tag,[attrs],[children])
			
			String16 tag_name = _scanner->string_value();
			int index = tag_name.index_of(String16().assign(':'));

			if (index != -1) {
				error(
					"Xml Syntax error, "
					"<prefix:suffix><prefix:suffix>"
					" grammar is not supported", _scanner->next_location());
			} else {              	// <tag
				append(S._VV);    	// _VV
				append(S.LPAREN);   // (
				append(tag_name);   // tag
				append(S.COMMA);    // ,
			}
			
			Dict<String16, bool> attrs;
			bool start_parse_attrs = false;
			Token token = next();
			
			// 解析xml属性
			if (is_object_property_identifier(token)) {
			attr:
				if (!_scanner->has_scape_before()) { // xml属性之间必须要有空白符号
					error("Xml Syntax error, attribute name no scape before");
				}
				
				if (!start_parse_attrs) {
					append(S.LBRACK);    // [ parse attributes start
					start_parse_attrs = true;
				}
				
				// 添加属性
				String16* raw_out = _out;
				String16 attribute_name;
				append(S.LBRACK); // [ // attribute start
				append(S.LBRACK); // [ // attribute name start
				
				do { // .
					append(S.QUOTES); // "
					append(_scanner->string_value());
					append(S.QUOTES); // "
					attribute_name.append(_scanner->string_value());
					token = next();
					if (token != PERIOD) break;
					if (!is_object_property_identifier(next())) {
						error("Xml Syntax error");
					}
					append(S.COMMA);   // ,
				} while (true);
				
				append(S.RBRACK); // ] // attribute name end
				append(S.COMMA);  // ,
				
				if (attrs.find(attribute_name) != attrs.end()) {
					error(String("Xml Syntax error, attribute repeat: ") + attribute_name.to_string());
				}
				attrs.set(attribute_name, 1);
				
				if (token == ASSIGN) { // =
					// 有=符号,属性必须要有值,否则为异常
					_is_xml_attribute_expression = true;
					parse_expression(); // 解析属性值表达式
					_is_xml_attribute_expression = false;
					token = next();
				} else { // 没有值设置为 ""
					append(S.QUOTES);    // "
					append(S.QUOTES);    // "
				}
				append(S.RBRACK); // ] // attribute end
				
				if (is_object_property_identifier(token)) {
					append(S.COMMA);   // ,
					goto attr; // 重新开始新属性
				}
			} else { // attrs empty
				append(S.LBRACK);  // [ parse attributes start
			}
			append(S.RBRACK);    // ] parse attributes end
			
			// 解析xml内容
			if (token == DIV) {      //    /  没有内容结束
				// add empty children
				append(S.COMMA);    // ,
				append(S.LBRACK);   // [
				append(S.RBRACK);   // ]
				if (next() != GT) { // >  语法错误
					error("Xml Syntax error");
				}
			} else if (token == GT) {       //   >  闭合标签,开始解析内容
				parse_xml_element_context(tag_name);
			} else {
				error("Xml Syntax error");
			}
			
			append(S.RPAREN); // )
		}
		
		void complete_xml_content_string(
			String16& str, String16& space,
			bool& is_once_comma, bool before_comma, bool ignore_space) 
		{
			if (str.length()) {
				String16 s(std::move(str));
				if ( !ignore_space || !s.trim().is_empty() ) {
					add_xml_children_cut_comma(is_once_comma);
					// _VVT("str")
					append(S._VVT);   // _VVT
					append(S.LPAREN); // (
					append(S.QUOTES);   // "
					append(std::move(s));
					append(S.QUOTES);   // "
					append(S.RPAREN); // (
				}
				// str = String16();
			}
			if ( before_comma ) {
				add_xml_children_cut_comma(is_once_comma);
			}
			if ( space.length() ) {
				_out->append(std::move(space));
			}
		}
		
		void add_xml_children_cut_comma(bool& is_once_comma) {
			if (is_once_comma) {
				is_once_comma = false;
			} else {
				append(S.COMMA);     // ,
			}
		}
		
		void parse_xml_element_context(cString16& tag_name) {
			ASSERT(_scanner->token() == GT);  // >
			
			// add chileren
			append(S.COMMA);    // ,
			append(S.LBRACK);   // [
			
			Token token;// prev = ILLEGAL;
			String16 str, scape;
			bool ignore_space = true;
			uint32_t pos = _scanner->location().end_pos;
			bool is_once_comma = true;
			
			while(true) {
				// <!-- xml comment --> OR <xml OR </xml> OR < <= << <<= OR ` OR block
				token = _scanner->scan_xml_content(pos, ignore_space);
				pos = _scanner->next_location().end_pos;

				switch (token) {
					case XML_COMMENT:    // <!-- comment -->
						/* ignore comment */
						scape.append(S.XML_COMMENT);
						scape.append(_scanner->next_string_value());
						scape.append(S.XML_COMMENT_END);
						break;
						
					case XML_ELEMENT_TAG: // <xml
						complete_xml_content_string(str, scape, is_once_comma, true, ignore_space);
						next();
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
						append(S.RBRACK);     // ]
						next();
						return;
						
					case LT: // <
						str.append(_scanner->next_string_value());
						break;

					case COMMAND: // {command block}
						// _VVD(block)
						complete_xml_content_string(str, scape, is_once_comma, true, ignore_space);
						next();             // {
						next();
						append(S._VVD);     // _VVD
						append(S.LPAREN);   // (
						parse_brace_expression(LBRACE, RBRACE); //
						append(S.RPAREN);   // )
						pos = _scanner->location().end_pos;
						break;
						
					case STRIFX_LITERAL:   // xml context text
						if (!_scanner->next_string_space().is_empty())
							scape.append(_scanner->next_string_space());
						str.append(_scanner->next_string_value());
						break;
						
					default: // <= << <<=
						error("Xml Syntax error", _scanner->next_location());
						break;
				}
			}
		}
		
		Buffer to_utf8_string(cString16 s) {
			return Coder::encode(Encoding::utf8, s);
		}
		
		inline void error() {
			error("SyntaxError: Unexpected token", _scanner->location());
		}
		
		inline void error(cString& msg) {
			error(msg, _scanner->location());
		}
		
		void error(cString& msg, Scanner::Location loc) {
			FX_THROW(ERR_SYNTAX_ERROR,
							"%s\nline:%d, pos:%d, %s",
							*msg, loc.line + 1, loc.end_pos, *_path);
		}
		
		Token next() {
			_collapse_scape_();
			Token tok = _scanner->next();
			_collapse_scape_();
			return tok;
		}

		inline Token token() {
			return _scanner->token();
		}
		
		inline Token peek() {
			return _scanner->peek();
		}

		void _collapse_scape_() {
			if (!_scanner->string_space().is_empty()) {
				if ( _is_class_member_data_expression ) {
					_out_class_member_data_expression.append(_scanner->string_space());
				}
				_top_out.append(std::move(_scanner->string_space()));
			}
		}
		
		void fetch() {
			_collapse_scape_();
			append(_scanner->string_value());
		}
		
		void append(cString16& code) {
			if ( code.length() ) {
				if ( _is_class_member_data_expression ) {
					_out_class_member_data_expression.append(code);
				}
				_out->append(code);
			}
		}
		
		void append(String16&& code) {
			if ( code.length() ) {
				if ( _is_class_member_data_expression ) {
					_out_class_member_data_expression.append(code);
				}
				_out->append(std::move(code));
			}
		}
		
		struct MemberDataExpression {
			String16 class_name;
			Dict<String16, String16> expressions;
		};
		
		Scanner*        _scanner;
		String16*       _out;
		String16        _top_out;
		String16        _out_class_member_data_expression;
		cString&        _path;
		Array<String16> _exports;
		String16        _export_default;
		Array<MemberDataExpression> _class_member_data_expression;
		uint32_t _level;
		bool _is_jsx;
		bool _is_class_member_data_expression;
		bool _is_xml_attribute_expression;
		bool _single_if_expression_before;
		bool _has_export_default;
		bool _clean_comment;
	};

    String16 javascript_transform_x(cString16& in, cString& path, bool clean_comment) throw(Error) {
		return Parser(in, path, true, clean_comment).transform();
	}

    String16 javascript_transform(cString16& in, cString& path, bool clean_comment) throw(Error) {
		return Parser(in, path, false, clean_comment).transform();
	}

}
