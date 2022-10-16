#include <mem.h>
#include <string.h>

#include "math.hpp"
#include "markdown.hpp"
#include "string.hpp"

/* TODO
 *
 * emettere:
 * - rimuovere vecchio codice su parse_heading dopo aver rimosso lo spazio
 *   in eccesso a fine testo che resta da parse_text.
 * - verificare le occorrenze di m_curr_* e rimuovere quelle ormai superflue.
 */

/*
 * - ATX headings using 1 to 6 hash characters followed by space: # to ######
 * - paragraphs ending with 2 consecutive eol: \n\n, or between other blocks
 * - ordered lists using number followed by dot: 1.
 * - unordered lists using dashes: -
 * - fenced code blocks using three consecutive backticks: ```
 * - italic using one asterisk: *
 * - bold using two consecutive asterisks: **
 * - bold and italic using three consecutive asterisks: ***
 * - code/keys spans using backtick: `
 */

markdown::markdown(int width, token_receiver_t f, indents_t indents) {
	memcpy(m_indents, indents, sizeof(indents_t));

	m_width = width;
	m_input = NULL;
	m_lines = 0;

	m_curr_block = markdown::B_PARAGRAPH;
	m_curr_style = markdown::S_NONE;
	m_curr_line_len = 0;

	m_token_receiver = f;
	memset(&m_curr_token, 0x00, sizeof(token_t));
	m_curr_token.type = T_BLOCK_START;
	m_curr_token.block = B_HEADING;

	// +2 is needed to correctly handle white-space collapsing
	// on parse_text.
	m_curr_token.text = new char[m_width + 2];
	memset(m_curr_token.text, 0x00, m_width + 2);
	m_curr_token_ptr = m_curr_token.text;

	memset(&m_curr_delim, 0x00, sizeof(m_curr_delim));
	m_token = new md_token(40, f);
}

markdown::~markdown() {
	if (m_input != NULL)
		fclose(m_input);

	delete m_curr_token.text;
	delete m_token;
}

bool_t markdown::set_source(const char *filename) {
	m_input = fopen(filename, "rb");
	if (m_input == NULL)
		return FALSE;

	if (setvbuf(m_input, NULL, _IOFBF, 1024) != 0) {
		fclose(m_input);
		return FALSE;
	}

	return TRUE;
}

int markdown::get_lines() {
	return m_lines;
}

bool_t markdown::parse() {
	if (m_input == NULL)
		return FALSE;

	m_lines = 0;
	m_curr_block = B_PARAGRAPH;
	m_curr_style = S_NONE;

	fseek(m_input, 0, SEEK_SET);

	bool_t ok = TRUE;
	while ((ok = this->is_block_delim(&m_curr_delim)) == TRUE) {
		switch (m_curr_delim.type) {
			case HEADING_DELIM:
				ok = this->parse_heading();
				break;

			case OLIST_DELIM:
				ok = this->parse_olist_block();
				break;

			case ULIST_DELIM:
				ok = this->parse_ulist_block();
				break;

			case CODE_DELIM:
				ok = this->parse_code_block();
				break;

			case PARAGRAPH_DELIM:
				ok = this->parse_paragraph();
				break;
		}

		if (feof(m_input) || ok == FALSE)
			break;
	}

	return ok;
}

bool_t markdown::is_block_delim(markdown::delim_info *info) {
	int read_count = 0;
	int white_space_count = 0;
	bool_t ok;
	char ch;

	// Skip empty lines and starting white-spaces.
	while ((ok = this->read_char(&ch, &read_count)) == TRUE) {
		if (string::is_eol(ch) == TRUE) {
			white_space_count = 0;
			continue;
		}

		if (string::is_white_space(ch) == TRUE) {
			++white_space_count;
			// Four initial white-spaces is recognized as code block.
			if (white_space_count == 4) {
				info->type = CODE_DELIM;
				info->size = read_count;
				info->ch = ' ';
				info->level = white_space_count;

				return TRUE;
			}

			continue;
		}

		break;
	}

	// Exit on read error.
	if (ok == FALSE) {
		fseek(m_input, -read_count, SEEK_CUR);
		return FALSE;
	}

	switch (ch) {
		case '#':
			// Heading delimiter check.
			int level = 1;
			while ((ok = this->read_char(&ch, &read_count)) == TRUE &&
				   ch == '#') {
				++level;

				if (level > 6)
					break;
			}

			// Exit on read error.
			if (ok == FALSE) {
				fseek(m_input, -read_count, SEEK_CUR);
				return FALSE;
			}

			// Not an heading, defaults to paragraph.
			if (level > 6 || (ch != ' ' && string::is_eol(ch) == FALSE))
				break;

			// Valid heading delimiter.
			info->type = HEADING_DELIM;
			info->size = read_count;
			info->ch = '#';
			info->level = level;

			return TRUE;

		case '1':
			const char *seq = ". ";
			for (int i = 0; i < 2; ++i) {
				ok = this->read_char(&ch, &read_count);
				// Exit on read error.
				if (ok == FALSE) {
					fseek(m_input, -read_count, SEEK_CUR);
					return FALSE;
				}

				// Not an orederd list delimiter.
				if (ch != seq[i])
					break;
			}

			// Not an orederd list delimiter, defaults to paragraph.
			if (i != 2)
				break;

			// Valid ordered list delimiter.
			info->type = OLIST_DELIM;
			info->size = read_count;
			info->ch = '1';
			info->level = 0;

			return TRUE;

		case '-':
			ok = this->read_char(&ch, &read_count);
			// Exit on read error.
			if (ok == FALSE) {
				fseek(m_input, -read_count, SEEK_CUR);
				return FALSE;
			}

			// Not an unorederd list delimiter, defaults to paragraph.
			if (ch != ' ')
				break;

			// Valid unordered list delimiter.
			info->type = ULIST_DELIM;
			info->size = read_count;
			info->ch = '-';
			info->level = 0;

			return TRUE;

		case '~':
		case '`':
			char fence_char = ch;
			int count = 0;
			while (ok == TRUE && ch == fence_char) {
				++count;
				ok = this->read_char(&ch, &read_count);
			}

			// Drop the info string.
			while (ok == TRUE && ch != '\n')
				ok = this->read_char(&ch, &read_count);

			// Exit on read error.
			if (ok == FALSE) {
				fseek(m_input, -read_count, SEEK_CUR);
				return FALSE;
			}

			// Not a fenced code block, defaults to paragraph.
			if (count < 3)
				break;

			// Valid fenced code block delimiter.
			info->type = CODE_DELIM;
			info->size = read_count;
			info->ch = fence_char;
			info->level = count;

			return TRUE;
	}

	info->type = PARAGRAPH_DELIM;
	info->size = 0;
	info->ch = ch;
	info->level = 0;

	// For paragraph always re-seek otherwise some chars will be lost.
	this->unread_chars(read_count, &read_count);
	return TRUE;
}

bool_t markdown::is_style_delim(char prev, char curr, style_delim *info) {
	if (curr != '*' && curr != '`')
		return FALSE;

	char ch, delim = curr;
	int read_count = 0, run = 1;
	bool_t ok;
	while ((ok = this->read_char(&ch, &read_count)) == TRUE && ch == delim)
		++run;

	char next = ' ';
	if (ok == TRUE) {
		this->unread_chars(1, &read_count);
		if (string::is_eol(ch) == FALSE)
			next = ch;
	}

	bool_t left = !string::is_white_space(next) &&
				  (!string::is_punctuation(next) ||
					(string::is_white_space(prev) ||
					string::is_punctuation(prev))
				  );

	bool_t right = !string::is_white_space(prev) &&
				   (!string::is_punctuation(prev) ||
					 (string::is_white_space(next) ||
					 string::is_punctuation(next))
				   );

	if ((left == FALSE && right == FALSE) || (delim == '`' && run > 1)) {
		this->unread_chars(read_count, &read_count);
		return FALSE;
	}

	info->style = m_curr_token.style;
	info->left = left;

	if (delim == '`') {
		if (left == TRUE)
			info->style = info->style | S_CODE;
		else if (right == TRUE)
			info->style = info->style & ~S_CODE;
	} else {
		if (left == TRUE) {
			if (run & 0x01)
				info->style = info->style | S_EMPHASIS;
			if (run & 0x02)
				info->style = info->style | S_STRONG;
		} else if (right == TRUE) {
			if (run & 0x01)
				info->style = info->style & ~S_EMPHASIS;
			if (run & 0x02)
				info->style = info->style & ~S_STRONG;
		}
	}

	return TRUE;
}

bool_t markdown::parse_heading() {
	this->new_block_token(B_HEADING);
	this->emit_curr_token();

	bool_t ok = this->parse_text();

	if (feof(m_input))
		ok = TRUE;

	if (ok == TRUE) {
		this->new_block_end_token(B_HEADING);
		this->emit_curr_token();
	}

	return ok;
/*
	int read_count = 0;
	char ch;
	bool_t ok;

	this->new_block_token(B_HEADING);
	this->emit_curr_token();

	this->new_style_token(S_NONE);

	char previous = ' ';
	while ((ok = this->read_char(&ch, &read_count)) == TRUE &&
		string::is_eol(ch) == FALSE) {

		if (string::is_white_space(ch) == TRUE) {
			// Collapse multiple white-spaces.
			if (string::is_white_space(previous) == FALSE)
				this->write_char(' ');
		} else if (ch == '#' && previous == ' ') {
			// Check for closing sequence.
			int hash_count = 1;
			while ((ok = this->read_char(&ch, &read_count)) == TRUE &&
				   ch == '#')
				++hash_count;

			if (ok == FALSE)
				return FALSE;

			bool_t white_space = FALSE;
			while ((ok = this->read_char(&ch, &read_count)) == TRUE &&
				   string::is_white_space(ch))
				   white_space = TRUE;

			if (ok == FALSE)
				return FALSE;

			// Closing sequence and white-spaces, heading is ended.
			if (string::is_eol(ch) == TRUE)
				break;

			// More non-whitespace characters after closing sequence.
			while (hash_count > 0) {
				if (this->write_char('#') == FALSE)
					return FALSE;

				--hash_count;
			}

			if (white_space == TRUE)
				if (this->write_char(' ') == FALSE)
					return FALSE;

			// Continue checking for closing sequence.
		} else
			this->write_char(ch);

		previous = ch;
	}

	if (feof(m_input))
		ok = TRUE;

	if (ok == TRUE) {
		this->emit_curr_token();

		this->new_block_end_token(B_HEADING);
		this->emit_curr_token();
	}

	return ok;
*/
}

bool_t markdown::parse_olist_block() {

	return TRUE;
}

bool_t markdown::parse_ulist_block() {
	int read_count = 0;
	bool_t ok = TRUE;

	this->new_block_token(B_ULIST);
	this->emit_curr_token();

	while (ok == TRUE) {
		this->new_block_token(B_LIST_ITEM);
		this->emit_curr_token();

		ok = this->parse_text();
		if (ok == FALSE)
			break;

		this->new_block_end_token(B_LIST_ITEM);

		delim_info delim;
		ok = this->is_block_delim(&delim);
		if (ok == FALSE || delim.type != ULIST_DELIM) {
			this->unread_chars(delim.size, &read_count);
			break;
		}
	}

	if (feof(m_input))
		ok = TRUE;

	if (ok == TRUE) {
		this->new_block_end_token(B_ULIST);
		this->emit_curr_token();
	}

	return ok;
}

bool_t markdown::parse_code_block() {
	int read_count = 0;
	bool_t ok = TRUE;
	char ch;

	this->new_block_token(B_CODE);
	this->emit_curr_token();

	this->new_style_token(S_NONE);

	int line_len = 0;
	while ((ok = this->read_char(&ch, &read_count)) == TRUE) {
		if (ch == '\n') {
			delim_info delim;
			ok = this->is_block_delim(&delim);
			if (ok == FALSE) {
				this->unread_chars(delim.size, &read_count);
				break;
			}

			if (delim.type == CODE_DELIM &&
				m_curr_delim.ch != ' ' &&
				delim.ch == m_curr_delim.ch &&
				delim.level >= m_curr_delim.level) {
				break;
			}

			if (m_curr_delim.ch == ' ' && delim.type != CODE_DELIM) {
				this->unread_chars(delim.size, &read_count);
				break;
			}

			// Ignore code delimiters using spaces inside a fenced
			// code block to proper retain code indendation.
			if (m_curr_delim.ch != ' ')
				this->unread_chars(delim.size, &read_count);

			this->emit_curr_token();

			this->new_line_token();
			this->emit_curr_token();

			this->new_style_token(S_NONE);
			line_len = 0;
			continue;
		}

		if (ch == '\t') {
			// Expand tabs to 4 spaces.
			for (int i = 0; i < 4 && ok == TRUE; ++i) {
				if (line_len < m_width)
					ok = this->write_char(' ');
				++line_len;
			}

			if (ok == FALSE)
				break;
		} else if (line_len < m_width) {
			ok = this->write_char(ch);
			++line_len;
		}
	}

	if (feof(m_input))
		ok = TRUE;

	if (ok == TRUE) {
		this->emit_curr_token();

		this->new_block_end_token(B_CODE);
		this->emit_curr_token();
	}

	return ok;
}

bool_t markdown::parse_paragraph() {
	this->new_block_token(B_PARAGRAPH);
	this->emit_curr_token();

	bool_t ok = this->parse_text();

	if (feof(m_input))
		ok = TRUE;

	if (ok == TRUE) {
		this->new_block_end_token(B_PARAGRAPH);
		this->emit_curr_token();
	}

	return ok;
}

bool_t markdown::parse_text() {
	int read_count = 0;
	bool_t ok = TRUE;
	char ch;

	style_delim sdelim;

	m_token->set_block(m_curr_token.block);
	m_token->set_indent(m_indents[m_curr_token.block]);
	m_token->reset();
	m_token->new_style(S_NONE);

	char previous = ' ';
	while ((ok = this->read_char(&ch, &read_count)) == TRUE) {
		if (ch == '\n') {
			if (previous == ch)
				break;

			delim_info delim;
			if ((ok = this->is_block_delim(&delim)) == TRUE) {
				if (delim.type != PARAGRAPH_DELIM) {
					this->unread_chars(delim.size, &read_count);
					break;
				}
			}
		}

		if (string::is_white_space(ch) == TRUE ||
			string::is_eol(ch) == TRUE) {
			// Collapse multiple white-spaces.
			if (string::is_white_space(previous) == FALSE &&
				string::is_eol(previous) == FALSE) {
				m_token->write_char(' ');
			}
		} else if (this->is_style_delim(previous, ch, &sdelim)) {
			m_token->new_style(sdelim.style);
		} else {
			m_token->write_char(ch);
		}

		if (m_token->width() > m_width)
			m_token->new_line();

		previous = ch;
	}

	if (feof(m_input))
		ok = TRUE;

	if (ok == TRUE)
		m_token->emit_tokens();

	return ok;
}

/* Versione a un token, non del tutto funzionante.
bool_t markdown::parse_text() {
	int read_count = 0;
	bool_t ok = TRUE, prev_is_ws = FALSE;
	char ch;

	style_delim sdelim;
	bool_t style_pending = FALSE;

	this->new_style_token(S_NONE);

	char previous = ' ';
	int prev_ws_read_count = -1;
	char *prev_ws_ptr = NULL;
	styles prev_ws_style = S_NONE;
	while ((ok = this->read_char(&ch, &read_count)) == TRUE) {
		if (ch == '\n') {
			if (previous == ch)
				break;

			delim_info delim;
			if ((ok = this->is_block_delim(&delim)) == TRUE) {
				if (delim.type != PARAGRAPH_DELIM) {
					this->unread_chars(delim.size, &read_count);
					break;
				}
			}
		}

		if (string::is_white_space(ch) == TRUE ||
			string::is_eol(ch) == TRUE) {
			// Collapse multiple white-spaces.
			if (string::is_white_space(previous) == FALSE &&
				string::is_eol(previous) == FALSE) {
				prev_is_ws = TRUE;
			}
			prev_ws_read_count = read_count;
			prev_ws_style = m_curr_token.style;
		} else if (this->is_style_delim(previous, ch, &sdelim)) {
			style_pending = sdelim.left;
			if (sdelim.left == FALSE) {
				// Closing inline style delimiter.
				this->emit_curr_token();
				this->new_style_token(sdelim.style);
			}
		} else {
			if (prev_is_ws == TRUE) {
				prev_ws_ptr = m_curr_token_ptr;
				this->write_char(' ');
				++m_curr_line_len;
				prev_is_ws = FALSE;
			}

			if (style_pending == TRUE) {
				// Opening inline style delimiter.
				this->emit_curr_token();
				this->new_style_token(sdelim.style);
				prev_ws_ptr = NULL;
				style_pending = FALSE;
			}

			this->write_char(ch);
			++m_curr_line_len;
		}

		if (m_curr_line_len > m_width) {
			uint16_t unread_count;
			if (prev_ws_read_count >= 0) {
				unread_count = read_count - prev_ws_read_count;
				if (prev_ws_ptr != NULL)
					m_curr_token_ptr = prev_ws_ptr;
				else
					m_curr_token_ptr = m_curr_token.text;

				*m_curr_token_ptr = '\0';
			} else {
				unread_count = m_curr_line_len - m_width;
				*(m_curr_token.text + m_width) = '\0';
			}

			this->unread_chars(unread_count, &read_count);

			if (m_curr_token_ptr > m_curr_token.text)
				this->emit_curr_token();

			this->new_line_token();
			this->emit_curr_token();

			this->new_style_token(prev_ws_style);
			ch = ' ';
			prev_is_ws = FALSE;
			prev_ws_ptr = NULL;
			prev_ws_read_count = -1;
		}

		previous = ch;
	}

	if (feof(m_input))
		ok = TRUE;

	if (ok == TRUE)
		this->emit_curr_token();

	return ok;
}
*/

bool_t markdown::read_char(char *ch, int *count) {
	int c = fgetc(m_input);
	if (c == EOF)
		return FALSE;

	if (c == '\r') {
		c = fgetc(m_input);
		if (c == EOF)
			return FALSE;

		if (count != NULL)
			++*count;
	}

	*ch = (char)c;
	if (count != NULL)
		++*count;

	return TRUE;
}

bool_t markdown::unread_chars(int num_chars, int *count) {
	if ((*count - num_chars) < 0)
		return FALSE;

	if (num_chars == 0)
		return TRUE;

	if (fseek(m_input, -num_chars, SEEK_CUR) == 0) {
		*count -= num_chars;
		return TRUE;
	}

	return FALSE;
}

bool_t markdown::write_char(char ch) {
	*m_curr_token_ptr++ = ch;
	return TRUE;
}

bool_t markdown::unwrite_chars(int num_chars) {
	char *newptr = m_curr_token_ptr - num_chars;

	if (newptr < m_curr_token.text)
		return FALSE;

	m_curr_token_ptr = newptr;
	*m_curr_token_ptr = '\0';
	return TRUE;
}

void markdown::new_block_token(markdown::block_type block) {
	m_curr_token.type = T_BLOCK_START;
	m_curr_token.block = block;
	m_curr_token.style = S_NONE;

	this->reset_token_buffer();

	m_curr_line_len = m_indents[block];
}

void markdown::new_block_end_token(markdown::block_type block) {
	m_curr_token.type = T_BLOCK_END;
	m_curr_token.block = block;

	this->reset_token_buffer();
}

void markdown::new_style_token(markdown::styles style) {
	m_curr_token.type = T_SPAN;
	m_curr_token.style = style;

	this->reset_token_buffer();
}

void markdown::new_line_token() {
	this->reset_token_buffer();

	m_curr_line_len = m_indents[m_curr_token.block];
	m_curr_token.type = T_NEWLINE;
}

void markdown::emit_curr_token() {
	if (m_token_receiver != NULL)
		m_token_receiver(&m_curr_token);
}

void markdown::reset_token_buffer() {
	if (m_curr_token.text == NULL)
		return;

	memset(m_curr_token.text, 0x00, m_width + 2);
	m_curr_token_ptr = m_curr_token.text;
}

/******************/

md_token::md_token(int buffer_size, markdown::token_receiver_t f) {
	m_buffer_size = buffer_size;
	m_token_receiver = f;
	// +2 is needed to correctly handle white-space collapsing
	// on parse_text.
	m_text = new char[buffer_size + 2];
	m_styles = new style_change_t[buffer_size];
	this->reset();
	m_indent = 0;
}

md_token::~md_token() {
	delete m_text;
	delete m_styles;
}

void md_token::reset() {
	m_styles_count = 0;
	m_ptr = m_text;
	*m_ptr = '\0';
	m_ws_ptr = NULL;
}

void md_token::set_block(markdown::block_type block) {
	m_block = block;
}

void md_token::set_indent(int indent) {
	m_indent = indent;
}

void md_token::new_style(markdown::styles style) {
	int offset = (int)(m_ptr - m_text);
	// Check if style change is requested multiple
	// times without writing characters, updating current.
	if (m_styles_count > 0) {
		int i = m_styles_count - 1;
		if (m_styles[i].offset == offset) {
			m_styles[i].style = style;
			return;
		}
	}

	m_styles[m_styles_count].style = style;
	m_styles[m_styles_count].offset = offset;
	++m_styles_count;
}

void md_token::new_line() {
	int unwrite_count;
	if (m_ws_ptr != NULL)
		unwrite_count = (int)(m_ptr - (m_ws_ptr + 1));
	else
		unwrite_count = (int)(m_ptr - (m_text + m_buffer_size));

	m_ptr -= unwrite_count;
	char ch = *m_ptr;
	*m_ptr = '\0';

	if (m_ws_ptr != NULL)
		*(m_ptr - 1) = '\0';

	// Emit tokens up to the new line.
	this->emit_tokens();

	// Emit new line token.
	if (m_token_receiver != NULL) {
		markdown::token_t token;
		token.block = m_block;
		token.type = markdown::T_NEWLINE;
		token.text = NULL;
		m_token_receiver(&token);
	}

	// It deletes text of emitted tokens and styles
	// changes preceding the new line from buffer.

	*m_ptr = ch;

	int styles_delta = (int)(m_ptr - m_text);
	memmove(m_text, m_ptr, unwrite_count);
	m_ptr = m_text + unwrite_count;
	*m_ptr = '\0';

	int j = 0;
	for (int i = 0; i < m_styles_count; ++i) {
		if ((i < (m_styles_count - 1)) &&
			((m_styles[i + 1].offset - styles_delta) < 0))
			continue;

		m_styles[j].style = m_styles[i].style;
		m_styles[j].offset = max(0, m_styles[i].offset - styles_delta);
		++j;
	}

	m_styles_count = j;
}

void md_token::emit_tokens() {
	markdown::token_t token;
	token.block = m_block;
	token.type = markdown::T_SPAN;

	style_change_t *style = m_styles,
		*last_style = &m_styles[m_styles_count - 1];

	char *ptr = m_text, *last = m_ptr, *end;
	char ch;
	while (ptr < last) {
		end = last;
		if (style < last_style)
			end = m_text + (style + 1)->offset;

		ch = *end;
		*end = '\0';

		if (m_token_receiver != NULL) {
			token.style = style->style;
			token.text = ptr;
			m_token_receiver(&token);
		}

		*end = ch;
		ptr = end;
		++style;
	}
}

int md_token::len() {
	return (int)(m_ptr - m_text);
}

int md_token::width() {
	return this->len() + m_indent;
}

bool_t md_token::write_char(char ch) {
	if (ch == ' ')
		m_ws_ptr = m_ptr;

	*m_ptr++ = ch;
	return TRUE;
}

bool_t md_token::unwrite_chars(int num_chars) {
	char *newptr = m_ptr - num_chars;

	if (newptr < m_text)
		return FALSE;

	m_ptr = newptr;
	*m_ptr = '\0';

	while (m_styles_count > 0 &&
		m_styles[m_styles_count - 1].offset > this->len())
		--m_styles_count;

	return TRUE;
}


