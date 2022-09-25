#ifndef TUI_TEXTBOX_HPP
#define TUI_TEXTBOX_HPP

#include <stdio.h>

#include "types.hpp"

#include "tui/control.hpp"

/** Textbox control. */
class uitextbox : public uicontrol {
public:
	enum source_type {
		TEXT,
		MARKDOWN
	};

	uitextbox(uint16_t id, int top, int left, int width, int height);
	~uitextbox();

	bool_t set_source(const char *filename, source_type type);

private:
	bool_t wrap_text(const char *filename);
	bool_t preparse(const char *filename);
	bool_t update_lines_offsets(int lines_count);
	void discard_preparsed();

	void scroll(int offset);
	void jump_to_begin();
	void jump_to_end();

	void calc_text_rect(vga::text_rect_t *rect);

	void repaint_line(int index, vga::text_rect_t *rect);
	void repaint_scroll_bar(vga::text_rect_t *rect);
	void repaint();

	void on_message(uiqueue::message_t *msg, tui::action_t *action);

	FILE *m_preparsed;
	source_type m_type;
	int m_lines;
	uint32_t *m_lines_offsets;
	int m_offset;
};

#endif
