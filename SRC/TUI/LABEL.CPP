#include <string.h>

#include "tui/label.hpp"
#include "tui/graph.hpp"
#include "math.hpp"
#include "string.hpp"

uilabel::uilabel(uint16_t id, int top, int left, const char *text)
 : uicontrol(id) {
	m_focusable = FALSE;

	int text_length = strlen(text);
	m_text = new char[text_length + 1];
	strcpy(m_text, text);

	m_rect.top = m_rect.bottom = top;
	m_rect.left = left;
	m_rect.right = left + text_length - 1;

	// DEBUG: temp debug values.
	m_paint_attrs = 0x17;
	m_blank_attrs = 0x11;
}

uilabel::~uilabel() {
	delete m_text;
	m_text = NULL;
}

void uilabel::repaint() {
	vga::text_rect_t rect;
	this->get_paint_rect(&rect);

	vga::set_cursor_pos(rect.top, rect.left);
	vga::write_string(m_text, m_paint_attrs,
					  vga::USE_ATTRS_NO_UPDATE_CURSOR);

	if (m_accelerator != NULL && m_text != NULL) {
		char *ch = m_text;
		int accel_index = 0;
		while (*ch != NULL) {
			if (string::icase_compare(*ch, m_accelerator) == TRUE) {
				accel_index += rect.left;
				uigraph::draw_accelerator(rect.top, accel_index,
										  m_paint_attrs);
				break;
			}

			++accel_index;
			++ch;
		}
	}

	uicontrol::repaint();
}
