#ifndef MARKDOWN_HPP
#define MARKDOWN_HPP

#include <stdio.h>

#include "types.hpp"

class md_token;

/** Simple markdown parser.
 *
 * This pre-parser support a subset of availabile markdown features.
 * Supported syntax:
 * - ATX headings using 1 to 6 hash characters followed by space: # to ######
 * - paragraphs ending with 2 consecutive eol: \n\n, or between other blocks
 * - ordered lists using number followed by dot: 1.
 * - unordered lists using dashes: -
 * - fenced code blocks using 3+ consecutive backticks or tildes: ``` or ~~~
 * - italic using one asterisk: *
 * - bold using two consecutive asterisks: **
 * - bold and italic using three consecutive asterisks: ***
 * - code/keys spans using backtick: `
 * - inside code/keys spans italic and bold styles will be ignored
 */
class markdown {
public:
	enum block_type {
		B_PARAGRAPH,
		B_HEADING,
		B_OLIST,
		B_ULIST,
		B_LIST_ITEM,
		B_CODE,
		B_COUNT // Count if block types.
	};

	enum styles {
		S_NONE = 0,
		S_EMPHASIS = 1 << 0,
		S_STRONG = 1 << 1,
		S_CODE = 1 << 2
	};

	enum token_type {
		T_BLOCK_START,
		T_BLOCK_END,
		T_SPAN,
		T_NEWLINE
	};

	typedef struct {
		token_type type;
		block_type block;
		styles style;
		char *text;
	} token_t;

	typedef uint8_t indents_t[B_COUNT];
	typedef bool_t (*token_receiver_t)(token_t *);

	markdown(int width, token_receiver_t f, indents_t indents);
	~markdown();

	bool_t set_source(const char *filename);

	int get_lines();

	bool_t parse();

private:
	enum block_delim {
		PARAGRAPH_DELIM,
		HEADING_DELIM,
		OLIST_DELIM,
		ULIST_DELIM,
		CODE_DELIM
	};

	typedef struct delim_info {
		block_delim type;
		int size;
		char ch;
		int level;
	};

	typedef struct style_delim {
		styles style;
		bool_t left;
	};

	bool_t is_block_delim(delim_info *info);
	bool_t is_style_delim(char prev, char curr, style_delim *info);

	bool_t parse_heading();
	bool_t parse_olist_block();
	bool_t parse_ulist_block();
	bool_t parse_code_block();
	bool_t parse_paragraph();

	bool_t parse_text();

	bool_t read_char(char *ch, int *count);
	bool_t unread_chars(int num_chars, int *count);

	bool_t write_char(char ch);
	bool_t unwrite_chars(int num_chars);

	void new_block_token(block_type block);
	void new_block_end_token(block_type block);
	void new_style_token(styles style);
	void new_line_token();
	void emit_curr_token();
	void reset_token_buffer();

	void swap_tokens();

	indents_t m_indents;

	FILE *m_input;
	int m_lines;
	int m_width;
	int m_curr_line_len;

	token_receiver_t m_token_receiver;
	token_t m_curr_token;
	char *m_curr_token_ptr;

	delim_info m_curr_delim;
	block_type m_curr_block;
	styles m_curr_style;

	md_token *m_token;
};

class md_token {
public:
	md_token(int buffer_size, markdown::token_receiver_t f);
	~md_token();

	void reset();
	void set_block(markdown::block_type block);
	void set_indent(int indent);
	void new_style(markdown::styles style);
	void new_line();

	void emit_tokens();

	int len();
	int width();

	bool_t write_char(char ch);
	bool_t unwrite_chars(int num_chars);

private:
	typedef struct {
		markdown::styles style;
		int offset;
	} style_change_t;

	int m_buffer_size;
	markdown::token_receiver_t m_token_receiver;

	markdown::block_type m_block;

	char *m_text;
	char *m_ptr;
	char *m_ws_ptr;

	style_change_t *m_styles;
	int m_styles_count;

	int m_indent;
};

#endif
