#include <string.h>
#include <search.h>

#include "tui/graph.hpp"
#include "tui/listbox.hpp"
#include "math.hpp"
#include "string.hpp"

#define TUI_ITEM_MAXLEN		128

/** Ascending sorting function.
 *
 * @param a Pointer to list item on m_items array.
 * @param b Pointer to list item on m_items_array.
 * @return strcmp result.
 */
static int items_asc_sorter(const void *a, const void *b) {
   return strcmp(*(char **)a, *(char **)b);
}

/** Descending sorting function.
 *
 * @param a Pointer to list item on m_items array.
 * @param b Pointer to list item on m_items_array.
 * @return Reversed strcmp result.
 */
static int items_desc_sorter(const void *a, const void *b) {
   return -strcmp(*(char **)a, *(char **)b);
}

uilistbox::uilistbox(uint16_t id, int top, int left, int width, int height)
 : uicontrol(id) {
	m_rect.top = top;
	m_rect.left = left;
	m_rect.right = left + width - 1;
	m_rect.bottom = top + height - 1;

	m_items_count = 0;
	m_items_capacity_step = 4;
	m_items_capacity = m_items_capacity_step;
	m_items = new char *[m_items_capacity];

	memset(m_items, 0x00, m_items_capacity * sizeof(char *));

	m_offset = 0;
	m_selected_item = -1;

	m_columns = 1;
	memset(m_columns_offsets, 0x00, (LISTBOX_MAX_COLS + 1) * sizeof(int));
	m_columns_offsets[1] = width;

	m_sort_mode = SORT_NONE;

	m_mnemonic_maxlen = width;
	m_mnemonic_string = new char [m_mnemonic_maxlen + 1];
	m_mnemonic_ptr = m_mnemonic_string;

	memset(m_mnemonic_string, 0x00, (m_mnemonic_maxlen + 1));

	m_highlight_maxlen = width;
	m_highlight_string = new char [m_highlight_maxlen + 1];

	memset(m_highlight_string, 0x00, (m_highlight_maxlen + 1));

	this->set_back_color_internal(uitheme::listbox::back);
	this->set_fore_color_internal(uitheme::listbox::fore);

	this->set_selected_back_color_internal(uitheme::listbox::selected_back);
	this->set_selected_fore_color_internal(uitheme::listbox::selected_fore);
}

uilistbox::~uilistbox() {
	for (int i = 0; i < m_items_count; ++i)
		delete m_items[i];

	delete m_items;
	delete m_mnemonic_string;
}

void uilistbox::clear() {
	for (int i = 0; i < m_items_count; ++i)
		delete m_items[i];

	memset(m_items, 0x00, m_items_capacity * sizeof(char *));
	m_items_count = 0;

	m_selected_item = -1;
	m_offset = 0;

	this->repaint();
}

bool_t uilistbox::add_item(const char *text) {
	if ((m_items_count + 1) > m_items_capacity) {
		char **new_array = new char *[m_items_capacity +
									  m_items_capacity_step];
		if (new_array == NULL)
			return FALSE;

		memcpy(new_array, m_items, m_items_count * sizeof(char *));
		memset(new_array + m_items_capacity, 0x00,
			   m_items_capacity_step * sizeof(char *));

		m_items_capacity += m_items_capacity_step;

		delete m_items;
		m_items = new_array;
	}

	m_items[m_items_count] = new char[strlen(text) + 1];

	if (m_items[m_items_count] == NULL)
		return FALSE;

	strcpy(m_items[m_items_count], text);

	++m_items_count;

	if (m_items_count == 1)
		m_selected_item = 0;

	this->sort_items();
	this->repaint();

	return TRUE;
}

bool_t uilistbox::set_selected_item(int index) {
	if (index < 0 || index > m_items_count) {
		m_selected_item = -1;
		return FALSE;
	}

	if (index == m_selected_item)
		return TRUE;

	m_selected_item = index;
	this->repaint();
	return TRUE;
}

bool_t uilistbox::set_selected_item(const char *text) {
	for (int i = 0; i < m_items_count; ++i) {
		if (strcmp(m_items[i], text) == 0) {
			m_selected_item = i;
			this->repaint();
			return TRUE;
		}
	}

	m_selected_item = -1;
	this->repaint();
	return FALSE;
}

int uilistbox::get_selected_item() {
	return m_selected_item;
}

const char *uilistbox::get_selected_item_text() {
	if (m_selected_item < 0)
		return NULL;

	return m_items[m_selected_item];
}

void uilistbox::set_columns(int columns) {
	if (columns == m_columns)
		return;

	m_columns = columns;

	vga::text_rect_t rect;
	this->calc_list_rect(&rect);
	int width = rect.right - rect.left + 1;

	for (int i = 0; i < m_columns; ++i)
		m_columns_offsets[i] = width * i / m_columns + i;

	m_columns_offsets[i] = width;

	this->repaint();
}

uilistbox::sorting uilistbox::get_sort_mode() {
	return m_sort_mode;
}

void uilistbox::set_sort_mode(uilistbox::sorting mode) {
	if (m_sort_mode == mode)
		return;

	m_sort_mode = mode;
	this->sort_items();
	this->repaint();
}

void uilistbox::sort_items() {
	if (m_items_count < 2)
		return;

	if (m_sort_mode == SORT_ASCENDING)
		qsort((void *)m_items, m_items_count, sizeof(char *),
			  items_asc_sorter);
	else if (m_sort_mode == SORT_DESCENDING)
		qsort((void *)m_items, m_items_count, sizeof(char *),
			  items_desc_sorter);
}

void uilistbox::mnemonic_push(char c) {
	if ((m_mnemonic_ptr - m_mnemonic_string) >= m_mnemonic_maxlen)
		return;

	*m_mnemonic_ptr++ = c;
	*m_mnemonic_ptr = NULL;

	int index = this->find_first_item(m_mnemonic_string);
	if (index >= 0)
		this->set_selected_item(index);
}

void uilistbox::mnemonic_pop() {
	if (m_mnemonic_ptr == m_mnemonic_string)
		return;

	*(--m_mnemonic_ptr) = NULL;

	if (m_mnemonic_ptr == m_mnemonic_string)
		return;

	int index = this->find_first_item(m_mnemonic_string);
	if (index >= 0)
		this->set_selected_item(index);
}

void uilistbox::mnemonic_reset() {
	*m_mnemonic_string = NULL;
	m_mnemonic_ptr = m_mnemonic_string;
}

int uilistbox::find_first_item(char *search) {
	if (m_items_count == 0)
		return -1;

	int search_len = strlen(search);
	if (search_len == 0)
		return -1;

	for (int i = 0; i < m_items_count; ++i)
		if (strnicmp(m_items[i], search, search_len) == 0)
			return i;

	return -1;
}

void uilistbox::set_highlight(const char *string) {
	if (string == NULL)
		memset(m_highlight_string, 0x00, m_highlight_maxlen);
	else
		strncpy(m_highlight_string, string, m_highlight_maxlen);

	this->repaint();
}

bool_t uilistbox::scroll(int offset) {
	this->mnemonic_reset();

	if (m_items_count < 1)
		return FALSE;

	int new_selected_entry = clamp(m_selected_item + offset, 0,
								   m_items_count - 1);
	int new_offset = this->calc_list_offset(new_selected_entry);
	int old_item = m_selected_item;

	vga::text_rect_t rect;
	this->calc_list_rect(&rect);

	if (new_offset == m_offset) {
		if (new_selected_entry != m_selected_item) {
			m_selected_item = new_selected_entry;
			this->repaint_item(old_item, &rect);
			this->repaint_item(m_selected_item, &rect);
		}

		return (old_item != m_selected_item);
	} else if (m_columns > 1) {
		// On multi-column view a repaint is needed each time the offset is
		// changed because is paged to simplify the cursor movement through
		// items as a grid.
		m_selected_item = new_selected_entry;
		m_offset = new_offset;
		this->repaint();
		return (old_item != m_selected_item);
	}

	int delta = new_offset - m_offset;
	int start = 0;

	m_selected_item = new_selected_entry;

	this->repaint_item(old_item, &rect);

	m_offset = new_offset;

	if (delta > 0) {
		vga::scroll_page_up((uint8_t)delta, m_blank_attrs, &rect);
		int displayed_lines = rect.bottom - rect.top + 1;
		start = displayed_lines - delta;
	} else {
		delta = -delta;
		vga::scroll_page_down((uint8_t)delta, m_blank_attrs, &rect);
	}

	for (int i = start; i < start + delta; ++i)
		this->repaint_item(m_offset + i, &rect);

	this->repaint_item(m_selected_item, &rect);
	this->repaint_scroll_bar(&rect);

	return (old_item != m_selected_item);
}

bool_t uilistbox::jump_to_begin() {
	this->mnemonic_reset();

	if (m_selected_item == 0)
		return FALSE;

	m_selected_item = 0;
	this->repaint();

	return TRUE;
}

bool_t uilistbox::jump_to_end() {
	this->mnemonic_reset();

	int last_item = max(0, m_items_count - 1);
	if (m_selected_item == last_item)
		return FALSE;

	m_selected_item = last_item;
	this->repaint();

	return TRUE;
}

int uilistbox::calc_list_offset(int selected_item) {
	vga::text_rect_t rect;
	this->calc_list_rect(&rect);

	int displayed_lines = rect.bottom - rect.top + 1;
	int offset;

	if (m_columns > 1) {
		// Multi-column layout moves by pages.
		offset = m_offset;
		if ((selected_item < m_offset) ||
			(selected_item >= m_offset + displayed_lines * m_columns)) {
			offset = selected_item - selected_item % displayed_lines;

			int last_column_items = m_items_count % displayed_lines;
			int last_page_offset = m_items_count -
								   displayed_lines * (m_columns - 1) -
								   last_column_items;

			offset = clamp(offset, 0, last_page_offset);
		}

		return offset;
	}

	offset = selected_item - (displayed_lines >> 1);
	return clamp(offset, 0, m_items_count - displayed_lines);
}

void uilistbox::calc_list_rect(vga::text_rect_t *rect) {
	this->get_paint_rect(rect);
	// Exclude the vertical scrollbar.
	--rect->right;
}

void uilistbox::repaint_scroll_bar(vga::text_rect_t *rect) {
	graphics::draw_scroll_bar(rect, m_scroll_bar_attrs,
							  m_items_count / m_columns,
							  m_offset / m_columns);
}

void uilistbox::repaint_item(int index, vga::text_rect_t *rect) {
	if (this->can_repaint() == FALSE)
		return;

	int line = index - m_offset;
	int column = 0;

	int max_length = rect->right - rect->left + 1;

	if (m_columns > 1) {
		int height = rect->bottom - rect->top + 1;
		column = line / height;
		line %= height;
		max_length = m_columns_offsets[column + 1] -
					 m_columns_offsets[column];
		column = m_columns_offsets[column];
	}

	max_length = min(TUI_ITEM_MAXLEN - 1, max_length);

	char buffer[TUI_ITEM_MAXLEN];

	strncpy(buffer, m_items[index], max_length);
	buffer[max_length] = NULL;

	vga::set_cursor_pos(rect->top + line, rect->left + column);
	vga::write_string(buffer, m_paint_attrs, vga::USE_ATTRS_UPDATE_CURSOR);

	int pad_size = max_length - strlen(buffer);
	vga::write_char(' ', m_paint_attrs, pad_size);

	uint8_t item_attrs = m_paint_attrs;

	if (index == m_selected_item) {
		if (tui::text_mode == TRUE) {
			item_attrs = m_selected_attrs;
			if (m_focused == FALSE)
				item_attrs = vga::swap_attrs(item_attrs);

			uigraph::apply_focus(rect->top + line, rect->left + column,
								 item_attrs, max_length);
		} else {
			uigraph::reverse_item(rect->top + line, rect->left + column,
								  item_attrs, max_length);
		}
	}

	int highlight_len = strlen(m_highlight_string);
	if (highlight_len > 0) {
		const char *matches[TUI_ITEM_MAXLEN];
		int match_count = string::icase_match_all(buffer, m_highlight_string,
												  matches);
		if (match_count > 0) {
			for (int i = 0; i < match_count; ++i)
				if (tui::text_mode == TRUE && index == m_selected_item)
					uigraph::highlight_chars(rect->top + line,
											 rect->left + column +
												matches[i] - buffer,
											 item_attrs ^ 0x88,
											 highlight_len);
				else
					uigraph::highlight_chars(rect->top + line,
											 rect->left + column +
												matches[i] - buffer,
											 item_attrs, highlight_len);
		}
	}
}

void uilistbox::repaint() {
	if (this->can_repaint() == FALSE) {
		m_repaint_pending = TRUE;
		return;
	}

	vga::text_rect_t rect;
	this->calc_list_rect(&rect);

	int lines = rect.bottom - rect.top + 1;
	vga::scroll_page_up((uint8_t)lines, m_blank_attrs, &rect);

	++rect.right;

	if (m_focused == TRUE)
		uigraph::apply_focus(&rect, m_paint_attrs, uigraph::FOCUS_OUTLINE);
	else
		uigraph::remove_focus(&rect, m_paint_attrs, uigraph::FOCUS_OUTLINE);

	--rect.right;

	m_offset = this->calc_list_offset(m_selected_item);

	this->repaint_scroll_bar(&rect);

	if (m_items_count < 1)
		return;

	for (int i = m_offset; i < m_offset + lines * m_columns; ++i) {
		if (i >= m_items_count)
			break;

		this->repaint_item(i, &rect);
	}

	uicontrol::repaint();
}

void uilistbox::on_message(uiqueue::message_t *msg, tui::action_t *action) {
	if (msg->type != uiqueue::TUIM_KEYPRESS && msg->type != uiqueue::TUIM_KEYUP)
		return;

	if (m_items_count == 0)
		return;

	keyboard::read_data_t *data = (keyboard::read_data_t *)msg->data;
	bool_t new_item_selected = FALSE;

	if (msg->type == uiqueue::TUIM_KEYPRESS) {
		vga::text_rect_t rect;
		this->calc_list_rect(&rect);
		int displayed_lines = rect.bottom - rect.top + 1;

		switch (data->virt_key) {
			case VK_HOME:
				new_item_selected = this->jump_to_begin();
				break;

			case VK_END:
				new_item_selected = this->jump_to_end();
				break;

			case VK_UP:
				new_item_selected = this->scroll(-1);
				break;

			case VK_LEFT:
				if (m_columns == 1)
					new_item_selected = this->scroll(-1);
				else
					new_item_selected = this->scroll(-displayed_lines);
				break;

			case VK_DOWN:
				new_item_selected = this->scroll(1);
				break;

			case VK_RIGHT:
				if (m_columns == 1)
					new_item_selected = this->scroll(1);
				else
					new_item_selected = this->scroll(displayed_lines);
				break;

			case VK_PGUP:
				new_item_selected = this->scroll(-displayed_lines * m_columns);
				break;

			case VK_PGDN:
				new_item_selected = this->scroll(displayed_lines * m_columns);
				break;

			case VK_ENTER:
				action->type = tui::TUIA_ITEM_ACTIVATED;
				action->id = m_id;
				action->item_id = m_selected_item;
				action->control = this;
				break;

			default:
				uint16_t mnemonic_length = m_mnemonic_ptr - m_mnemonic_string;
				int prev_selected_item = m_selected_item;
				if (data->ascii >= 0x20 &&
					mnemonic_length < m_mnemonic_maxlen) {
					this->mnemonic_push(data->ascii);
				} else if (data->virt_key == VK_BKSPC &&
						   mnemonic_length > 0)
					this->mnemonic_pop();

				new_item_selected = (m_selected_item != prev_selected_item);
				break;
		}
	}

	if (new_item_selected == TRUE ||
		m_last_action == tui::TUIA_ITEM_SELECTING) {
		if (msg->type == uiqueue::TUIM_KEYPRESS)
			action->type = tui::TUIA_ITEM_SELECTING;
		else
			action->type = tui::TUIA_ITEM_SELECTED;

		m_last_action = action->type;

		action->id = m_id;
		action->item_id = m_selected_item;
		action->control = this;
	}
}
