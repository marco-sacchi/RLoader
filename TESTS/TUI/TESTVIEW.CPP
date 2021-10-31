#include <string.h>

#include "tui/testview.hpp"

static const char *list_items[] = {
	"Item 0",
	"Item 1",
	"Long text for item 2",
	"Shorter item 3",
	"Very long text for item 4 that overflow",
	"Other item 5",
	"Another item 6",
	"Item 7",
	"Item 8"
};

testview::testview(int top, int left, int width, int height)
 : uiview() {
	m_top = top;
	m_left = left;
	m_width = width;
	m_height = height;
}

bool_t testview::init_controls() {
	m_window = new uiwindow(ID_WINDOW, m_top, m_left, m_width, m_height,
							uiwindow::CAPTIONED, "Test caption");
	if (m_window == NULL)
		return FALSE;

	m_button1 = new uibutton(ID_BUTTON_OPEN, 5, 8, 11, "Open view");
	if (m_button1 == NULL)
		return FALSE;

	m_button1->set_accelerator('O');
	m_window->add_control(m_button1);

	m_button2 = new uibutton(ID_BUTTON_CLOSE, 5, 21, 12, "Close view");
	if (m_button2 == NULL)
		return FALSE;

	m_button2->set_accelerator('V');
	m_window->add_control(m_button2);

	m_label = new uilabel(ID_LABEL, 7, 8, "Label text:");
	if (m_label == NULL)
		return FALSE;

	m_label->set_accelerator('E');
	m_window->add_control(m_label);

	m_edit = new uieditbox(ID_EDIT, 7, 20, 26, 64);
	if (m_edit == NULL)
		return FALSE;

	m_edit->set_text("Long string to test editbox scrolling.");
	m_edit->set_accelerator('E');
	m_window->add_control(m_edit);

	m_list = new uilistbox(ID_LIST, 9, 20, 26, 6);
	if (m_list == NULL)
		return FALSE;

	m_list->set_accelerator('L');

	bool_t populate_ok = TRUE;
	for (int i = 0; i < 9; ++i)
		populate_ok = populate_ok && m_list->add_item(list_items[i]);

	m_window->add_control(m_list);
	if (populate_ok == NULL)
		return FALSE;

	return TRUE;
}

const char *testview::get_field_text() {
	return m_edit->get_text();
}

bool_t testview::get_list_item(int *index, const char **text) {
	*index = m_list->get_selected_item();
	*text = m_list->get_selected_item_text();
	return (*index < 0) ? FALSE : TRUE;
}

bool_t testview::on_action(tui::action_t *action) {
	tui::message_t msg;
	switch (action->type) {
		case tui::TUIA_CTRL_DESTROY:
			if (action->id == ID_WINDOW)
				return TRUE;
			break;

		case tui::TUIA_VIEW_CLOSED:
			if (action->view == m_dialog) {
				const char *field_text = m_dialog->get_field_text();
				int item_index;
				const char *item_text;
				if (m_dialog->get_list_item(&item_index, &item_text) == TRUE)
					m_list->set_selected_item(item_index);

				m_edit->set_text(field_text);
			}

			break;

		case tui::TUIA_CTRL_CLICK:
			switch (action->id) {
				case ID_BUTTON_OPEN:
					if (action->type == tui::TUIA_CTRL_CLICK) {
						m_dialog = new testview(5, 16, 48, 16);
						if (m_dialog != NULL) {
							msg.type = tui::TUIM_OPEN_VIEW;
							msg.data = m_dialog;
							this->queue_message(&msg);
						}
					}
					break;

				case ID_BUTTON_CLOSE:
					if (action->type == tui::TUIA_CTRL_CLICK) {
						msg.type = tui::TUIM_CLOSE_VIEW;
						msg.data = this;
						this->queue_message(&msg);
					}
					break;
			}
			break;
	}

	return TRUE;
}
