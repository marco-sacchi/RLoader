#include <string.h>

#include "tui/msgbox.hpp"
#include "math.hpp"
#include "string.hpp"
#include "tui/button.hpp"
#include "tui/label.hpp"
#include "tui/window.hpp"

#define MSGBOX_MAX_BUTTONS		8
#define MSGBOX_MAX_LINES		20
#define MSGBOX_MIN_TEXT_WIDTH	30
#define MSGBOX_MAX_TEXT_WIDTH	50

#define ID_WINDOW	1

const char *uimsgbox::m_captions[] = {
	"Ok",
	"Cancel",
	"Yes",
	"No",
	"Retry",
	"Abort",
	"Ignore"
};

const char uimsgbox::m_accelerators[] = {
	'O', 'C', 'Y', 'N', 'R', 'A', 'I'
};

uimsgbox::uimsgbox(const char *caption, const char *message)
 : uiview() {
	m_is_modal = TRUE;

	m_caption = new char [strlen(caption) + 1];
	strcpy(m_caption, caption);

	m_message = new char [strlen(message) + 1];
	strcpy(m_message, message);

	m_buttons_count = 0;
	memset(m_buttons_infos, 0x00, MSGBOX_MAX_BUTTONS * sizeof(button_info_t));

	m_selected_button_id = 0;
}

uimsgbox::~uimsgbox() {
	delete m_message;
	delete m_caption;

	for (int i = 0; i < MSGBOX_MAX_BUTTONS; ++i)
		if (m_buttons_infos[i].id >= BTN_CUSTOM_FIRST &&
			m_buttons_infos[i].caption != NULL)
			delete m_buttons_infos[i].caption;
}

bool_t uimsgbox::add_button(int id) {
	if (m_buttons_count >= MSGBOX_MAX_BUTTONS)
		return FALSE;

	button_info_t *info = &m_buttons_infos[m_buttons_count++];
	info->id = id;
	info->caption = (char *)uimsgbox::m_captions[id];
	info->accelerator = uimsgbox::m_accelerators[id];

	return TRUE;
}

bool_t uimsgbox::add_button(int id, const char *caption, char accelerator) {
	if (m_buttons_count >= MSGBOX_MAX_BUTTONS)
		return FALSE;

	button_info_t *info = &m_buttons_infos[m_buttons_count++];
	info->id = id;
	info->caption = new char[strlen(caption) + 1];
	strcpy(info->caption, caption);
	info->accelerator = accelerator;

	return TRUE;
}

int uimsgbox::calc_buttons_props() {
	int width = 0;
	button_info_t *info;
	for (int i = 0; i < m_buttons_count; ++i) {
		info = &m_buttons_infos[i];
		info->left = width;
		info->width = strlen(info->caption) + 4;
		width += info->width + 2;
	}

	if (m_buttons_count > 0)
		width -= 2;

	return width;
}

bool_t uimsgbox::init_controls() {
	if (m_buttons_count == 0) {
		if (this->add_button(BTN_OK) == FALSE)
			return FALSE;
	}

	int buttons_width = this->calc_buttons_props();

	int message_width = max(buttons_width, MSGBOX_MIN_TEXT_WIDTH);
	int window_width = max(buttons_width, message_width) + 4;

	uilabel *label = new uilabel(0, 2, 2, message_width, 0, m_message);
	if (label == NULL)
		return FALSE;

	vga::text_rect_t rect;
	label->get_rect(&rect);

	int window_height = rect.bottom - rect.top + 6;
	m_window = new uiwindow(ID_WINDOW, 0, 0, window_width, window_height,
							uiwindow::FRAMED | uiwindow::CAPTIONED |
							uiwindow::CENTERED,
							m_caption);
	if (m_window == NULL)
		return FALSE;

	m_window->add_control(label);

	int buttons_offset = (window_width - buttons_width) >> 1;

	for (int i = 0; i < m_buttons_count; ++i) {
		button_info_t *info = &m_buttons_infos[i];
		uibutton *button = new uibutton(info->id, window_height - 2,
										info->left + buttons_offset,
										info->width, info->caption);

		if (button == NULL)
			return FALSE;

		if (info->accelerator)
			button->set_accelerator(info->accelerator);

		m_window->add_control(button);
	}

	return TRUE;
}

bool_t uimsgbox::on_action(tui::action_t *action) {
	switch (action->type) {
		case tui::TUIA_CTRL_DESTROY:
			if (action->id == ID_WINDOW) {
				this->queue_result(m_selected_button_id);
				return TRUE;
			}
			break;

		case tui::TUIA_CTRL_ACTIVATED:
			m_selected_button_id = action->id;
			this->queue_close_view();
			break;
	}

	return TRUE;
}
