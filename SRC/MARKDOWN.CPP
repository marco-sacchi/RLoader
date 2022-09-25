#include <mem.h>
#include <string.h>

#include "markdown.hpp"
#include "string.hpp"

/* TODO
 *
 * emettere:
 * - un token di tipo T_NEWLINE ogni volta che si supera il numero
 *   massimo di caratteri per linea m_width ad eccezione dei blocchi B_CODE
 *
 * - troncare le linee dei blocchi di codice a m_width (no wordwrap)
 * - aggiungere configurazione del rientro per i tipi di blocchi in modo
 *   da calcolare correttamente la lunghezza delle righe (es. liste) e
 *   relativa emissione di T_NEWLINE
 * - valutare se usare parse_text anche sugli heading ignorando la gestione
 *   delle le serie di # di chiusura, diventeranno testo visibile.
 * - gestire correttamente il wordwrap evitando di spezzare parole e
 *   tralasciando i whitespace quando sono a inizio/fine riga.
 */

/*
 * - headings using 1 to 6 hash characters followed by space: # to ######
 * - paragraphs ending with 2 consecutive eol: \n\n, or between other blocks
 * - ordered lists using number followed by dot: 1.
 * - unordered lists using dashes: -
 * - fenced code blocks using three consecutive backticks: ```
 * - italic using one asterisk: *
 * - bold using two consecutive asterisks: **
 * - bold and italic using three consecutive asterisks: ***
 * - code/keys spans using backtick: `
 */

markdown::markdown() {
	m_input = NULL;
	m_lines = m_width = 0;

	m_curr_block = markdown::B_PARAGRAPH;
	m_curr_style = markdown::S_NONE;
	m_curr_line_len = 0;

	m_token_receiver = NULL;
	memset(&m_curr_token, 0x00, sizeof(token_t));
	m_curr_token.type = T_BLOCK_START;
	m_curr_token.block = B_HEADING;

	m_curr_token_ptr = NULL;

	memset(&m_curr_delim, 0x00, sizeof(m_curr_delim));
}

markdown::~markdown() {
	if (m_input != NULL)
		fclose(m_input);
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

void markdown::set_width(int width) {
	m_width = width;

	if (m_curr_token.text != NULL)
		delete m_curr_token.text;

	m_curr_token.text = new char[m_width + 1];
	memset(m_curr_token.text, 0x00, m_width + 1);
	m_curr_token_ptr = m_curr_token.text;
}

void markdown::set_token_receiver(token_receiver_t f) {
	m_token_receiver = f;
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

bool_t markdown::is_style_delim(char prev, char curr, styles *new_style) {
	if (curr != '*' && curr != '`')
		return FALSE;

	int read_count = 0;
	char ch, delim = curr;
	int run = 1;
	bool_t ok;
	while ((ok = this->read_char(&ch, &read_count)) == TRUE && ch == delim)
		++run;

	char next = ' ';
	if (ok == TRUE) {
		this->unread_chars(1, &read_count);
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

	*new_style = m_curr_token.style;

	if (delim == '`') {
		if (left == TRUE)
			*new_style = *new_style | S_CODE;
		else if (right == TRUE)
			*new_style = *new_style & ~S_CODE;
	} else {
		if (left == TRUE) {
			if (run & 0x01)
				*new_style = *new_style | S_EMPHASIS;
			if (run & 0x02)
				*new_style = *new_style | S_STRONG;
		} else if (right == TRUE) {
			if (run & 0x01)
				*new_style = *new_style & ~S_EMPHASIS;
			if (run & 0x02)
				*new_style = *new_style & ~S_STRONG;
		}
	}

	return TRUE;
}

bool_t markdown::parse_heading() {
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

		delim_info delim;
		ok = this->is_block_delim(&delim);
		if (ok == FALSE || delim.type != ULIST_DELIM) {
			this->unread_chars(delim.size, &read_count);
			break;
		}

		this->new_line_token();
		this->emit_curr_token();
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

			if (delim.type != CODE_DELIM) {
				this->unread_chars(delim.size, &read_count);
				break;
			}

			// Ignore code delimiters using spaces inside a fenced
			// code block to proper retain code indendation.
			this->unread_chars(delim.size, &read_count);
		}

		if (ch == '\t') {
			// Expand tabs to 4 spaces.
			for (int i = 0; i < 4 && ok == TRUE; ++i)
				ok = this->write_char(' ');

			if (ok == FALSE)
				break;
		} else
			ok = this->write_char(ch);
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
	bool_t ok = TRUE, last_written_is_ws = FALSE;
	char ch;
	styles style;

	this->new_style_token(S_NONE);

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
				this->write_char(' ');
				if (++m_curr_line_len == m_width) {
					this->emit_curr_token();

					this->new_line_token();
					this->emit_curr_token();

					this->new_style_token(m_curr_token.style);
					previous = ' ';
				}
				last_written_is_ws = TRUE;
			}
		} else if (this->is_style_delim(previous, ch, &style)) {
			// Inline style.
			this->emit_curr_token();

			this->new_style_token(style);
		} else {
			last_written_is_ws = FALSE;
			this->write_char(ch);

			if (++m_curr_line_len == m_width) {
				this->emit_curr_token();

				this->new_line_token();
				this->emit_curr_token();

				this->new_style_token(m_curr_token.style);
				previous = ' ';
			}
		}

		previous = ch;
	}

	if (feof(m_input))
		ok = TRUE;

	if (ok == TRUE) {
		if (last_written_is_ws == TRUE)
			this->unwrite_chars(1);

		this->emit_curr_token();
	}

	return ok;
}

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

	if (m_curr_token.text != NULL) {
		memset(m_curr_token.text, 0x00, m_width + 1);
		m_curr_token_ptr = m_curr_token.text;
	}

	m_curr_line_len = 0;
}

void markdown::new_block_end_token(markdown::block_type block) {
	m_curr_token.type = T_BLOCK_END;
	m_curr_token.block = block;

	if (m_curr_token.text != NULL) {
		memset(m_curr_token.text, 0x00, m_width + 1);
		m_curr_token_ptr = m_curr_token.text;
	}
}

void markdown::new_style_token(markdown::styles style) {
	m_curr_token.type = T_SPAN;
	m_curr_token.style = style;

	if (m_curr_token.text != NULL) {
		memset(m_curr_token.text, 0x00, m_width + 1);
		m_curr_token_ptr = m_curr_token.text;
	}
}

void markdown::new_line_token() {
	if (m_curr_token.text != NULL) {
		memset(m_curr_token.text, 0x00, m_width + 1);
		m_curr_token_ptr = m_curr_token.text;
	}

	m_curr_line_len = 0;
	m_curr_token.type = T_NEWLINE;
}

void markdown::emit_curr_token() {
	if (m_token_receiver != NULL)
		m_token_receiver(&m_curr_token);
}
