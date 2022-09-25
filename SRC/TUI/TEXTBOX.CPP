#include <alloc.h>
#include <string.h>

#include "tui/textbox.hpp"
#include "tui/graph.hpp"
#include "math.hpp"
#include "string.hpp"
#include "markdown.hpp"

uitextbox::uitextbox(uint16_t id, int top, int left, int width, int height)
 : uicontrol(id) {
	m_rect.top = top;
	m_rect.left = left;
	m_rect.right = left + width - 1;
	m_rect.bottom = top + height - 1;

	m_preparsed = NULL;
	m_type = uitextbox::TEXT;
	m_lines = 0;
	m_lines_offsets = NULL;
	m_offset = 0;

	this->set_back_color_internal(uitheme::textbox::back);
	this->set_fore_color_internal(uitheme::textbox::fore);
}

uitextbox::~uitextbox() {
	this->discard_preparsed();
}

bool_t uitextbox::set_source(const char *filename,
							 uitextbox::source_type type) {
	if (filename == NULL)
		return FALSE;

	switch (type) {
		case uitextbox::TEXT:
			m_lines = 0;
			m_offset = 0;
			m_type = type;
			return wrap_text(filename);

		case uitextbox::MARKDOWN:
			m_lines = 0;
			m_offset = 0;
			m_type = type;
			return preparse(filename);
	}

	return FALSE;
}

bool_t uitextbox::wrap_text(const char *filename) {
	this->discard_preparsed();

	FILE *input = fopen(filename, "rb");
	if (input == NULL)
		return FALSE;

	m_preparsed = tmpfile();
	if (m_preparsed == NULL) {
		fclose(input);
		return FALSE;
	}

	vga::text_rect_t rect;
	this->calc_text_rect(&rect);
	int columns = rect.right - rect.left + 1;

	char *buffer = (char *)malloc(columns + 2);
	if (buffer == NULL) {
		fclose(input);
		fclose(m_preparsed);
		m_preparsed = NULL;
		return FALSE;
	}

	int lines = 0;
	// Dummy value.
	fwrite(&lines, sizeof(lines), 1, m_preparsed);

	while (fgets(buffer, columns + 2, input) != NULL) {
		int length = strlen(buffer);
		char *end = buffer + length - 1;
		// The end of buffer is in the middle of a word.
		if (length == (columns + 1) && !string::is_white_space(*end)) {
			// Go back to nearest whitespace and seek back, so that each
			// buffer starts at the beginning of a word.
			while (end > buffer && !string::is_white_space(*end))
				--end;

			// The buffer contains only part of one word longer than the
			// buffer itself.
			if (end == buffer)
				end = buffer + columns - 1;

			fseek(input, (end - buffer + 1) - length, SEEK_CUR);
			// Skip white-spaces on line end.
			while (end > buffer && string::is_white_space(*end))
				--end;
		} else {
			while (end > buffer &&
				   (string::is_white_space(*end) || string::is_eol(*end)))
				--end;
		}

		length = (int)(end - buffer + 1);

		++lines;
		fputc(length, m_preparsed);
		if (length > 0)
			fwrite(buffer, length, 1, m_preparsed);
	}

	free(buffer);

	fclose(input);

	fseek(m_preparsed, 0, SEEK_SET);
	fwrite(&lines, sizeof(int), 1, m_preparsed);

	fflush(m_preparsed);

	return this->update_lines_offsets(lines);
}

bool_t uitextbox::preparse(const char *filename) {
	this->discard_preparsed();

	m_preparsed = tmpfile();
	if (m_preparsed == NULL)
		return FALSE;

	vga::text_rect_t rect;
	this->calc_text_rect(&rect);

	markdown *parser = new markdown();
	parser->set_source(filename);
	parser->set_width(rect.right - rect.left + 1);
	parser->set_output(m_preparsed);

	if (parser->parse() == FALSE) {
		delete parser;
		return FALSE;
	}

	int lines = parser->get_lines();
	delete parser;

	return this->update_lines_offsets(lines);
}

bool_t uitextbox::update_lines_offsets(int lines_count) {
	if (m_lines_offsets != NULL)
		delete m_lines_offsets;

	m_lines = lines_count;
	m_lines_offsets = new uint32_t[m_lines];
	if (m_lines_offsets == NULL) {
		m_lines = 0;
		return FALSE;
	}

	fseek(m_preparsed, sizeof(int), SEEK_SET);

	uint32_t offset = sizeof(int);
	for (int i = 0; i < lines_count; ++i) {
		int length = fgetc(m_preparsed);
		m_lines_offsets[i] = offset;
		offset += length + 1;
		fseek(m_preparsed, length, SEEK_CUR);
	}

	fseek(m_preparsed, 0, SEEK_SET);

	return TRUE;
}

void uitextbox::discard_preparsed() {
	if (m_preparsed != NULL) {
		// Automatically deleted on close because is a temporary file.
		fclose(m_preparsed);
		m_preparsed = NULL;
	}

	if (m_lines_offsets != NULL) {
		delete m_lines_offsets;
		m_lines_offsets = NULL;
	}
}

void uitextbox::scroll(int offset) {
	if (m_lines < 1)
		return;

	vga::text_rect_t rect;
	this->calc_text_rect(&rect);

	int displayed_lines = rect.bottom - rect.top + 1;
	int new_offset = max(0, min(m_lines - displayed_lines, m_offset + offset));

	if (new_offset == m_offset)
		return;

	int delta = new_offset - m_offset;
	int start = 0;

	m_offset = new_offset;

	if (delta > 0) {
		vga::scroll_page_up((uint8_t)delta, m_blank_attrs, &rect);
		start = displayed_lines - delta;
	} else {
		delta = -delta;
		vga::scroll_page_down((uint8_t)delta, m_blank_attrs, &rect);
	}

	for (int i = start; i < start + delta; ++i)
		this->repaint_line(m_offset + i, &rect);

	this->repaint_scroll_bar(&rect);
}

void uitextbox::jump_to_begin() {
	if (m_offset == 0)
		return;

	m_offset = 0;
	this->repaint();
}

void uitextbox::jump_to_end() {
	vga::text_rect_t rect;
	this->calc_text_rect(&rect);
	int displayed_lines = rect.bottom - rect.top + 1;

	int new_offset = max(0, m_lines - displayed_lines);
	if (m_offset == new_offset)
		return;

	m_offset = new_offset;
	this->repaint();
}

void uitextbox::calc_text_rect(vga::text_rect_t *rect) {
	this->get_paint_rect(rect);
	// Exclude the vertical scrollbar.
	--rect->right;
}

void uitextbox::repaint_line(int index, vga::text_rect_t *rect) {
	fseek(m_preparsed, m_lines_offsets[index], SEEK_SET);

	int length = fgetc(m_preparsed);
	if (length == 0)
		return;

	int max_length = rect->right - rect->left + 1;
	length = min(length, max_length);

	char buffer[90];
	if (fread(buffer, length, 1, m_preparsed) == 0)
		return;

	buffer[length] = '\0';

	vga::set_cursor_pos(rect->top + index - m_offset, rect->left);
	vga::write_string(buffer, m_paint_attrs, vga::USE_ATTRS_UPDATE_CURSOR);
}

void uitextbox::repaint_scroll_bar(vga::text_rect_t *rect) {
	graphics::draw_scroll_bar(rect, m_scroll_bar_attrs,
							  m_lines, m_offset);
}

void uitextbox::repaint() {
	if (this->can_repaint() == FALSE) {
		m_repaint_pending = TRUE;
		return;
	}

	vga::text_rect_t rect;
	this->calc_text_rect(&rect);

	int lines = rect.bottom - rect.top + 1;
	vga::scroll_page_up((uint8_t)lines, m_blank_attrs, &rect);

	++rect.right;

	if (m_focused == TRUE)
		uigraph::apply_focus(&rect, m_paint_attrs, uigraph::FOCUS_OUTLINE);
	else
		uigraph::remove_focus(&rect, m_paint_attrs, uigraph::FOCUS_OUTLINE);

	--rect.right;

	this->repaint_scroll_bar(&rect);

	if (m_lines < 1)
		return;

	for (int i = m_offset; i < m_offset + lines; ++i) {
		if (i >= m_lines)
			break;

		this->repaint_line(i, &rect);
	}

	uicontrol::repaint();
}

void uitextbox::on_message(uiqueue::message_t *msg, tui::action_t *action) {
	if (m_lines == 0 || msg->type != uiqueue::TUIM_KEYPRESS)
		return;

	keyboard::read_data_t *data = (keyboard::read_data_t *)msg->data;

	if (msg->type == uiqueue::TUIM_KEYPRESS) {
		vga::text_rect_t rect;
		this->calc_text_rect(&rect);
		int displayed_lines = rect.bottom - rect.top + 1;

		switch (data->virt_key) {
			case VK_HOME:
				this->jump_to_begin();
				break;

			case VK_END:
				this->jump_to_end();
				break;

			case VK_UP:
			case VK_LEFT:
				this->scroll(-1);
				break;

			case VK_DOWN:
			case VK_RIGHT:
				this->scroll(1);
				break;

			case VK_PGUP:
				this->scroll(-displayed_lines);
				break;

			case VK_PGDN:
				this->scroll(displayed_lines);
				break;
		}
	}
}
