#ifndef MARKDOWN_HPP
#define MARKDOWN_HPP

#include <stdio.h>

#include "types.hpp"

/** Simple markdown pre-parser.
 *
 * This pre-parser support a subset of availabile markdown features.
 * Supported syntax:
 * - headings using 1 to 6 hash characters followed by space: # to ######
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
		B_CODE
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

	typedef bool_t (*token_receiver_t)(token_t *);

	markdown();
	~markdown();

	bool_t set_source(const char *filename);
	void set_width(int width);

	void set_token_receiver(token_receiver_t f);

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

	bool_t is_block_delim(delim_info *info);
	bool_t is_style_delim(char prev, char curr, styles *new_style);

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
};

#endif
