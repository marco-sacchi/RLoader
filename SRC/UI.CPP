#include <string.h>
#include <alloc.h>
#include <dir.h>
#include <io.h>

#include "ui.hpp"
#include "math.hpp"
#include "graphics.hpp"
#include "string.hpp"
#include "encoder.hpp"

#define THUMBNAIL_DIRS_COUNT 2

const char *thumbnail_dirs[THUMBNAIL_DIRS_COUNT] = {
	"/titles/",
	"/in_progs/"
};

ui::ui(list *list, bool_t text_mode, uint8_t back_color,
	   uint8_t fore_color) {
	m_text_mode = text_mode;

	memset(&m_panel_rect, 0x00, sizeof(vga::text_rect_t));
	m_panel_attrs = vga::encode_attrs(back_color, fore_color, 0);
	m_blank_attrs = vga::encode_attrs(back_color, back_color, 0);

	m_list = list;
	m_offset = m_selected_entry = 0;
	m_filtered = NULL;
	m_filtered_count = 0;
	memset(m_filtered_string, 0x00, UI_FILTER_STR_SIZE);
	m_thumbnail_loop = 0;
	memset(&m_list_rect, 0x00, sizeof(vga::text_rect_t));

	m_info_displayed = FALSE;
	m_info_offset = 0;
	m_info_lines = 0;
	m_info_file = NULL;
	memset(&m_info_rect, 0x00, sizeof(vga::text_rect_t));

	memset(m_thumbnail_path, 0x00, UI_THUMBNAIL_PATH_SIZE);

	m_encoder = NULL;
}

ui::~ui() {
	m_list = NULL;
	m_filtered = NULL;
	m_filtered_count = 0;
	m_panel_attrs = 0;
	m_offset = m_selected_entry = 0;
	m_thumbnail_loop = 0;
}

void ui::set_panel_rect(vga::text_rect_t *rect) {
	memcpy(&m_panel_rect, rect, sizeof(vga::text_rect_t));

	m_list_rect.top = rect->top + 3;
	m_list_rect.left = rect->left + 1;
	m_list_rect.right = rect->right - 1;
	m_list_rect.bottom = rect->bottom - 1;

	m_info_rect.top = rect->top + 1;
	m_info_rect.left = rect->left + 1;
	m_info_rect.right = rect->right - 1;
	m_info_rect.bottom = rect->bottom - 1;

	vga::scroll_page_up(rect->bottom - rect->top + 1,
						m_panel_attrs & 0xf0, rect);
}

void ui::filter_list(const char *search) {
	strcpy(m_filtered_string, search);

	m_selected_entry = 0;
	m_filtered = m_list->filter(search, &m_filtered_count);

	this->draw_search_box();
	this->draw_list();
}

void ui::scroll_list(int offset) {
	if (m_filtered_count < 1)
		return;

	int new_selected_entry = clamp(m_selected_entry + offset, 0,
								   m_filtered_count - 1);
	int new_offset = this->calc_list_offset(new_selected_entry);

	m_thumbnail_loop = 0;

	if (new_offset == m_offset) {
		if (new_selected_entry != m_selected_entry) {
			this->xor_selected_entry();
			m_selected_entry = new_selected_entry;
			this->xor_selected_entry();
			this->draw_thumbnail(this->get_selected_entry());
		}

		return;
	}

	int delta = new_offset - m_offset;
	int start = 0;

	this->xor_selected_entry();

	m_offset = new_offset;
	m_selected_entry = new_selected_entry;

	if (delta > 0) {
		vga::scroll_page_up((uint8_t)delta, m_blank_attrs, &m_list_rect);
		int displayed_lines = m_list_rect.bottom - m_list_rect.top + 1;
		start = displayed_lines - delta;
	} else {
		delta = -delta;
		vga::scroll_page_down((uint8_t)delta, m_blank_attrs, &m_list_rect);
	}

	for (int i = start; i < start + delta; ++i)
		this->draw_list_entry(i);

	this->xor_selected_entry();
	this->draw_thumbnail(this->get_selected_entry());

	this->draw_list_scroll_bar();
}

void ui::scroll(int offset) {
	if (m_info_displayed)
		this->scroll_info(offset);
	else
		this->scroll_list(offset);
}

void ui::jump_to_begin() {
	if (m_info_displayed) {
		m_info_offset = 0;
		this->draw_info();
	} else {
		this->xor_selected_entry();
		m_selected_entry = 0;
		this->draw_list();
	}
}

void ui::jump_to_end() {
	if (m_info_displayed) {
		m_info_offset = max(0, m_info_lines -
							(m_info_rect.bottom - m_info_rect.top + 1));
		this->draw_info();
	} else {
		this->xor_selected_entry();
		m_selected_entry = max(0, m_filtered_count - 1);
		this->draw_list();
	}
}

void ui::draw_panel() {
	graphics::draw_frame(&m_panel_rect, m_panel_attrs);

	int width = m_panel_rect.right - m_panel_rect.left + 1;
	vga::set_cursor_pos(m_panel_rect.top + 2, m_panel_rect.left);
	vga::write_char(0xc3, m_panel_attrs, 1);

	vga::set_cursor_pos(m_panel_rect.top + 2, m_panel_rect.left + 1);
	vga::write_char(0xc4, m_panel_attrs, width - 2);

	vga::set_cursor_pos(m_panel_rect.top + 2, m_panel_rect.right);
	vga::write_char(0xb4, m_panel_attrs, 1);

	this->draw_shortcut("F1", "Help");
}

void ui::draw_shortcut(const char *key, const char *text) {
	int key_length = strlen(key) + 2;
	int length = key_length + strlen(text);
	uint8_t row = m_panel_rect.bottom;
	uint8_t left = m_panel_rect.right - length - 2;

	vga::set_cursor_pos(row, left);
	vga::write_char(' ', m_panel_attrs, 1);
	vga::set_cursor_pos(row, left + 1);
	vga::write_string(key, m_panel_attrs,
					  vga::USE_ATTRS_UPDATE_CURSOR);
	vga::write_char(' ', m_panel_attrs, 1);

	vga::set_cursor_pos(row, left + key_length);
	vga::write_char(' ', m_panel_attrs, 1);
	vga::set_cursor_pos(row, left + key_length + 1);
	vga::write_string(text, m_panel_attrs,
					  vga::USE_ATTRS_UPDATE_CURSOR);
	vga::write_char(' ', m_panel_attrs, 1);

	this->xor_chars(row, left, key_length);
}

void ui::draw_search_box() {
	int top = m_panel_rect.top + 1;
	int left = m_panel_rect.left + 2;

	vga::set_cursor_pos(top, left);
	vga::write_string("Search: ", m_panel_attrs, vga::USE_ATTRS_UPDATE_CURSOR);
	vga::write_string(m_filtered_string, m_panel_attrs,
					  vga::USE_ATTRS_UPDATE_CURSOR);
	vga::write_string("_ ", m_panel_attrs, vga::USE_ATTRS_UPDATE_CURSOR);
}

void ui::draw_list() {
	int lines = m_list_rect.bottom - m_list_rect.top + 1;
	vga::scroll_page_up((uint8_t)lines, m_blank_attrs, &m_list_rect);

	m_thumbnail_loop = 0;

	if (m_filtered_count < 1) {
		this->draw_list_scroll_bar();
		this->clear_thumbnail_area();
		return;
	}

	m_offset = this->calc_list_offset(m_selected_entry);
	for (int i = 0; i < lines; ++i) {
		if ((m_offset + i) >= m_filtered_count)
			break;

		this->draw_list_entry(i);
	}

	this->draw_list_scroll_bar();
	this->xor_selected_entry();

	this->draw_thumbnail(this->get_selected_entry());
}

void ui::draw_list_scroll_bar() {
	graphics::draw_scroll_bar(&m_list_rect, m_panel_attrs,
							  m_filtered_count, m_offset);
}

void ui::draw_list_entry(int line) {
	char buffer[UI_FILTER_STR_SIZE];
	int max_length = min(UI_FILTER_STR_SIZE - 1,
						 m_list_rect.right - m_list_rect.left + 1);

	strncpy(buffer, m_filtered[m_offset + line]->get_title(), max_length);
	buffer[max_length] = NULL;

	vga::set_cursor_pos(m_list_rect.top + line, m_list_rect.left);
	vga::write_string(buffer, m_panel_attrs, vga::USE_ATTRS_NO_UPDATE_CURSOR);

	if (strlen(m_filtered_string) == 0)
		return;

	const char *matches[UI_FILTER_STR_SIZE];
	int match_count = string::icase_match_all(buffer, m_filtered_string,
											  matches);
	if (match_count > 0) {
		int search_len = strlen(m_filtered_string);

		if (m_text_mode == TRUE) {
			for (int i = 0; i < match_count; ++i)
				vga::xor_attrs(m_list_rect.top + line,
							   m_list_rect.left + matches[i] - buffer,
							   0x08, search_len);
		} else {
			for (int i = 0; i < match_count; ++i) {
				vga::set_cursor_pos(m_list_rect.top + line,
									m_list_rect.left + matches[i] - buffer);
				vga::write_char('_', (m_panel_attrs & 0x0f) | 0x80,
								search_len);
			}
		}
	}
}

void ui::xor_selected_entry() {
	int width = m_list_rect.right - m_list_rect.left + 1;

	this->xor_chars(m_list_rect.top + m_selected_entry - m_offset,
					m_list_rect.left, width);
}

int ui::calc_list_offset(int selected_entry) {
	int displayed_lines = m_list_rect.bottom - m_list_rect.top + 1;
	int offset = selected_entry - (displayed_lines >> 1);
	return clamp(offset, 0, max(0, m_filtered_count - displayed_lines));
}

void ui::draw_thumbnail(const list_entry *entry) {
	if (m_text_mode == TRUE || entry == NULL)
		return;

	if (m_encoder) {
		delete m_encoder->get_bitmap();
		delete m_encoder;
		m_encoder = NULL;
	}

	strcpy(m_thumbnail_path, m_list->get_path());
	strcat(m_thumbnail_path, thumbnail_dirs[m_thumbnail_loop]);
	strcat(m_thumbnail_path, entry->get_folder());
	strcat(m_thumbnail_path, ".bmp");

	if (access(m_thumbnail_path, 0) != 0) {
		this->draw_thumbnail_message("No thumbnail available.");
		return;
	}

	bitmap *bmp = new bitmap();
	if (bmp == NULL || bmp->load(m_thumbnail_path) == FALSE) {
		if (bmp != NULL) {
			switch (bmp->get_last_error()) {
				case BMP_ERR_NOTFOUND:
					this->draw_thumbnail_message("Thumbnail not found.");
					break;

				case BMP_ERR_INVALID:
					this->draw_thumbnail_message("Thumbnail corrupted.");
					break;

				case BMP_ERR_NOMEM:
					this->draw_thumbnail_message("Cannot allocate thumbnail.");
					break;

				default:
					this->draw_thumbnail_message("Thumbnail corrupted.");

			}
			delete bmp;
		}
		return;
	}

	m_encoder = new bitmap_encoder(bmp, 2);
	if (m_encoder == NULL) {
		delete bmp;
		this->draw_thumbnail_message("Cannot allocate thumbnail.");
	}
}

void ui::task() {
	if (m_encoder == NULL || m_encoder->task() == FALSE)
		return;

	this->clear_thumbnail_area();
	bitmap *bmp = m_encoder->get_bitmap();
	graphics::set_ega_palette(bmp, 2);
	graphics::draw(bmp, 320, (480 >> 1) - (bmp->get_height() >> 1));
	delete bmp;

	delete m_encoder;
	m_encoder = NULL;
}

void ui::loop_thumbnail() {
	if (m_text_mode == TRUE || m_filtered == NULL)
		return;

	m_thumbnail_loop = (m_thumbnail_loop + 1) % THUMBNAIL_DIRS_COUNT;
	this->draw_thumbnail(this->get_selected_entry());
}

void ui::draw_thumbnail_message(const char *message) {
	const vga::state_t *state = vga::get_current_state();
	if (state->columns <= 40)
		return;

	vga::text_rect_t rect;
	rect.top = 0;
	rect.left = m_panel_rect.right + 1;
	rect.right = state->columns - 1;
	rect.bottom = state->rows - 1;

	this->clear_thumbnail_area();

	vga::set_cursor_pos(
		(rect.bottom + rect.top) >> 1,
		((rect.right + rect.left) >> 1) - (strlen(message) >> 1)
	);
	vga::write_string(message, m_panel_attrs,
					  vga::USE_ATTRS_NO_UPDATE_CURSOR);
}

void ui::clear_thumbnail_area() {
	const vga::state_t *state = vga::get_current_state();
	if (state->columns <= 40)
		return;

	vga::text_rect_t rect;
	rect.top = 0;
	rect.left = m_panel_rect.right + 1;
	rect.right = state->columns - 1;
	rect.bottom = state->rows - 1;

	vga::set_map_mask_reg(0x0f);
	vga::scroll_page_up(rect.bottom - rect.top + 1,
						m_panel_attrs & 0xf0, &rect);
}

const list_entry *ui::get_selected_entry() {
	if (m_filtered_count < 1)
		return NULL;

	return m_filtered[m_selected_entry];
}

bool_t ui::display_help() {
	return this->display_info("res/help.md");
}

bool_t ui::display_selected_entry_info() {
	const char *folder = this->get_selected_entry()->get_folder();
	char path[MAXPATH];

	strcpy(path, m_list->get_path());
	strcat(path, "/info/");
	strcat(path, folder);
	strcat(path, ".md");

	if (access(path, 0) != 0)
		return TRUE;

	return this->display_info(path);
}

bool_t ui::display_info(const char *filename) {
	if (m_info_displayed == TRUE)
		return FALSE;

	m_info_file = fopen(filename, "rb");
	if (m_info_file == NULL)
		return FALSE;

	char line[50];
	m_info_displayed = TRUE;
	m_info_offset = 0;
	m_info_lines = 0;
	while (fgets(line, 50, m_info_file) != NULL)
		++m_info_lines;

	m_info_lines_offset = (long *)malloc(sizeof(long) * m_info_lines);
	if (m_info_lines_offset == NULL) {
		this->release_info_resources();
		return FALSE;
	}

	fseek(m_info_file, 0, SEEK_SET);
	long *offset = m_info_lines_offset;
	*offset++ = 0;
	while (fgets(line, 50, m_info_file) != NULL)
		*offset++ = ftell(m_info_file);

	vga::set_map_mask_reg(0x0f);
	graphics::draw_frame(&m_panel_rect, m_panel_attrs);

	this->draw_shortcut("ESC", "Close");

	fseek(m_info_file, 0, SEEK_SET);

	this->draw_info();

	return TRUE;
}

void ui::draw_info() {
	vga::scroll_page_up(m_info_rect.bottom - m_info_rect.top + 1,
						m_panel_attrs & 0xf0, &m_info_rect);

	graphics::draw_scroll_bar(&m_info_rect, m_panel_attrs,
							  m_info_lines, m_info_offset);

	int lines = min(m_info_lines, m_info_rect.bottom - m_info_rect.top + 1);
	for (int i = 0; i < lines; ++i)
		this->draw_info_line(i);
}

void ui::draw_info_line(int row) {
	char line[50];
	if (m_info_file == NULL)
		return;

	fseek(m_info_file, m_info_lines_offset[m_info_offset + row], SEEK_SET);
	if (fgets(line, 50, m_info_file) == NULL)
		return;

	row += m_info_rect.top;
	uint8_t left = m_info_rect.left;

	char *rendered = line;

	// Header.
	bool_t is_header = FALSE;
	int header_level = 0;
	if (*line == '#') {
		while (*(line + header_level) == '#' && header_level < 4)
			++header_level;

		if (*(line + header_level) == ' ') {
			rendered += header_level + 1;
			is_header = TRUE;
		}
	}

	// Unordered list.
	bool_t is_list = FALSE;
	if ((*line == '*' || *line == '-' || *line == '+') &&
		*(line + 1) == ' ') {
		*line = ' ';
		is_list = TRUE;
	}

	char *new_line = strpbrk(rendered, "\n\r");
	if (new_line != NULL)
		*new_line = NULL;

	int rendered_length = strlen(rendered);

	// Strip trailing white-spaces.
	new_line = rendered + rendered_length - 1;
	while (rendered_length > 0 && string::is_white_space(*new_line)) {
		*new_line-- = NULL;
		--rendered_length;
	}

	if (rendered_length == 0)
		return;

	// Remove formatting characters and store bolds/italics ranges.
	uint8_t bolds[30];
	uint8_t italics[30];
	int bolds_count = 0, italics_count = 0;
	char *chr = rendered, *marker_chr;
	char marker_count = 0, marker_open = 0;
	while (*chr != NULL && rendered_length > 0) {
		if (*chr != '*') {
			++chr;
			continue;
		}

		marker_chr = chr;
		marker_count = 0;
		while (*marker_chr == '*') {
			++marker_chr;
			++marker_count;
		}

		bool_t opened = (marker_open & marker_count);

		if ((opened && !this->is_marker_boundary(*marker_chr)) ||
			(!opened && chr > rendered &&
			 !this->is_marker_boundary(*(chr - 1)))) {
			++chr;
			continue;
		}

		if (marker_count > 0) {
			memmove(chr, marker_chr,
					rendered_length - (size_t)(marker_chr - rendered));
			rendered_length -= marker_count;
			*(rendered + rendered_length) = NULL;

			int offset = 0;
			if (marker_open & marker_count)
				offset = 1;

			if (marker_count & 1) {
				italics[italics_count * 2 + offset] = chr - rendered - offset;
				if (offset == 1) {
					++italics_count;
					marker_open &= ~1;
				} else
					marker_open |= 1;
			}

			if (marker_count & 2) {
				bolds[bolds_count * 2 + offset] = chr - rendered - offset;
				if (offset == 1) {
					++bolds_count;
					marker_open &= ~2;
				} else
					marker_open |= 2;
			}
		}
	}

	vga::set_cursor_pos(row, left);
	vga::write_string(rendered, m_panel_attrs,
					  vga::USE_ATTRS_NO_UPDATE_CURSOR);

	if (is_list == TRUE)
		vga::write_char(0x07, m_panel_attrs, 1);

	int i;
	if (is_header == TRUE) {
		if (m_text_mode == TRUE) {
			vga::xor_attrs(row, left, 0x88, rendered_length);
		} else {
			switch (header_level) {
				case 1:
					graphics::bold_effect(row, left, rendered_length);
					graphics::italic_effect(row, left, rendered_length);
					break;

				case 2:
					graphics::bold_effect(row, left, rendered_length);
					break;

				case 3:
					graphics::bold_effect(row, left, rendered_length);
					break;

				case 4:
					graphics::italic_effect(row, left, rendered_length);
					break;
			}

			if (header_level < 3)
				vga::write_char('_', (m_panel_attrs & 0x0f) | 0x80,
								rendered_length);
		}
	} else {
		if (m_text_mode == TRUE) {
			for (i = 0; i < bolds_count * 2; i += 2)
				vga::xor_attrs(row, left + bolds[i], 0x08,
							   bolds[i + 1] - bolds[i] + 1);
		} else {
			for (i = 0; i < bolds_count * 2; i += 2)
				graphics::bold_effect(row, left + bolds[i],
									  bolds[i + 1] - bolds[i] + 1);
		}
	}

	if (m_text_mode == TRUE) {
		for (i = 0; i < italics_count * 2; i += 2)
			vga::xor_attrs(row, left + italics[i], 0x08,
						   italics[i + 1] - italics[i] + 1);
	} else {
		for (i = 0; i < italics_count * 2; i += 2)
			graphics::italic_effect(row, left + italics[i],
									italics[i + 1] - italics[i] + 1);
	}

}

bool_t ui::is_marker_boundary(char chr) {
	return chr == NULL || string::is_white_space(chr) ||
		   string::is_punctuation(chr);
}

void ui::hide_info() {
	if (m_info_displayed == FALSE)
		return;

	this->release_info_resources();
	m_info_displayed = FALSE;

	vga::set_map_mask_reg(0x0f);
	vga::scroll_page_up(m_panel_rect.bottom - m_panel_rect.top + 1,
						m_panel_attrs & 0xf0, &m_panel_rect);
	this->draw_panel();
	this->draw_search_box();
	this->draw_list();
}

void ui::release_info_resources() {
	if (m_info_file != NULL) {
		fclose(m_info_file);
		m_info_file = NULL;
	}

	if (m_info_lines_offset != NULL) {
		free(m_info_lines_offset);
		m_info_lines_offset = NULL;
	}

	m_info_offset = m_info_lines = 0;
}

void ui::scroll_info(int offset) {
	if (m_info_displayed == FALSE)
		return;

	int displayed_lines = m_info_rect.bottom - m_info_rect.top + 1;
	int new_offset = clamp(m_info_offset + offset, 0,
						   max(0, m_info_lines - displayed_lines));

	if (new_offset == m_info_offset)
		return;

	int delta = new_offset - m_info_offset;
	int start = 0;

	m_info_offset = new_offset;

	if (delta > 0) {
		vga::scroll_page_up((uint8_t)delta, m_blank_attrs, &m_info_rect);
		start = displayed_lines - delta;
	} else {
		delta = -delta;
		vga::scroll_page_down((uint8_t)delta, m_blank_attrs, &m_info_rect);
	}

	for (int i = start; i < start + delta; ++i)
		this->draw_info_line(i);

	graphics::draw_scroll_bar(&m_info_rect, m_panel_attrs,
							  m_info_lines, m_info_offset);
}

void ui::display_cannot_launch(const list_entry *entry) {
	strcpy(m_thumbnail_path, entry->get_path());
	strcat(m_thumbnail_path, "\\");
	strcat(m_thumbnail_path, entry->get_executable());
	strcat(m_thumbnail_path, " not found.");

	this->draw_thumbnail_message(m_thumbnail_path);
}

void ui::display_cannot_setup(const list_entry *entry) {
	strcpy(m_thumbnail_path, entry->get_path());
	strcat(m_thumbnail_path, "\\");
	strcat(m_thumbnail_path, entry->get_executable());
	strcat(m_thumbnail_path, " not found.");

	this->draw_thumbnail_message(m_thumbnail_path);
}

void ui::display_no_setup() {
	this->draw_thumbnail_message("No configuration program specified.");
}

void ui::xor_chars(uint8_t row, uint8_t col, uint16_t count) {
	if (m_text_mode == TRUE) {
		uint8_t reverse = (m_panel_attrs & 0x0f) ^
						  ((m_panel_attrs & 0xf0) >> 4);
		reverse |= reverse << 4;
		vga::xor_attrs(row, col, reverse, count);
	} else {
		vga::set_cursor_pos(row, col);
		vga::write_char(0xdb, (m_panel_attrs & 0x0f) | 0x80, count);
	}
}
