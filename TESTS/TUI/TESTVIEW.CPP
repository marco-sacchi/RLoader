#include <string.h>
#include <dir.h>

#include "tui/testview.hpp"
#include "tui/msgbox.hpp"
#include "tui/views/opensave.hpp"

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
							uiwindow::FRAMED | uiwindow::CAPTIONED,
							"Test caption");
	if (m_window == NULL)
		return FALSE;

	m_button1 = new uibutton(ID_BUTTON_OPEN, 3, 4, 11, "Open view");
	if (m_button1 == NULL)
		return FALSE;

	m_button1->set_accelerator('O');
	m_window->add_control(m_button1);

	m_button2 = new uibutton(ID_BUTTON_CLOSE, 3, 17, 12, "Close view");
	if (m_button2 == NULL)
		return FALSE;

	m_button2->set_fore_color(0x0c);
	m_button2->set_back_color(4);
	m_button2->set_accelerator('V');
	m_window->add_control(m_button2);

	m_button3 = new uibutton(ID_BUTTON_MSGBOX, 3, 31, 9, "Msg box");
	if (m_button3 == NULL)
		return FALSE;

	m_button3->set_fore_color(0x0a);
	m_button3->set_back_color(2);
	m_button3->set_accelerator('M');
	m_window->add_control(m_button3);

	m_button4 = new uibutton(ID_BUTTON_DLGOPEN, 5, 4, 13, "Open dialog");
	if (m_button4 == NULL)
		return FALSE;

	m_button4->set_accelerator('P');
	m_window->add_control(m_button4);

	m_button5 = new uibutton(ID_BUTTON_DLGSAVE, 5, 19, 13, "Save dialog");

	if (m_button5 == NULL)
		return FALSE;

	m_button5->set_accelerator('S');
	m_window->add_control(m_button5);

	m_label = new uilabel(ID_LABEL, 7, 4, "Label text:");
	if (m_label == NULL)
		return FALSE;

	m_label->set_accelerator('E');
	m_window->add_control(m_label);

	m_edit = new uieditbox(ID_EDIT, 7, 16, 22, 64);
	if (m_edit == NULL)
		return FALSE;

	m_edit->set_text("Long string to test editbox scrolling.");
	m_edit->set_text("D:\\CPROGS\\RLOADER\\");
	m_edit->set_accelerator('E');
	m_window->add_control(m_edit);

	m_list = new uilistbox(ID_LIST, 9, 16, 22, 6);
	if (m_list == NULL)
		return FALSE;

	m_list->set_accelerator('L');

	bool_t populate_ok = TRUE;
	for (int i = 0; i < 9; ++i)
		populate_ok = populate_ok && m_list->add_item(list_items[i]);

	m_window->add_control(m_list);
	if (populate_ok == FALSE)
		return FALSE;

	m_text = new uitextbox(ID_TEXT, 3, 42, 32, 8);
	if (m_text == NULL)
		return FALSE;

	m_text->set_source("../chglog.md", uitextbox::TEXT);
	m_window->add_control(m_text);

	m_markdown = new uitextbox(ID_MARKDOWN, 12, 42, 32, 8);
	if (m_markdown == NULL)
		return FALSE;

	m_markdown->set_source("../readme.md", uitextbox::MARKDOWN);
	m_window->add_control(m_markdown);

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

uimsgbox *testview::create_messagebox() {
	uimsgbox *msgbox = new uimsgbox("Message caption",
									"Long message text to test "
									"word-wrapping, correct line counting, "
									"window sizing and buttons rendering.");
	if (msgbox == NULL)
		return NULL;

	if (!msgbox->add_button(uimsgbox::BTN_OK)) {
		delete msgbox;
		return NULL;
	}

	if (!msgbox->add_button(uimsgbox::BTN_CANCEL)) {
		delete msgbox;
		return NULL;
	}

	if (!msgbox->add_button(uimsgbox::BTN_CUSTOM_FIRST, "Custom", 'S')) {
		delete msgbox;
		return NULL;
	}

	return msgbox;
}

uiopensave *testview::create_opensave(uiopensave::mode mode) {
	return new uiopensave(mode, m_edit->get_text(), "*.*");
}

bool_t testview::on_action(tui::action_t *action) {
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
				m_dialog = NULL;
			} else if (action->view == m_opensave) {
				char buffer[MAXPATH];
				if (m_opensave->get_selected_fullpath(buffer) == TRUE)
					m_edit->set_text(buffer);
			}

			break;

		case tui::TUIA_CTRL_ACTIVATED:
			switch (action->id) {
				case ID_BUTTON_OPEN:
					m_dialog = new testview(5, 16, 50, 16);
					if (m_dialog != NULL)
						this->queue_open_view(m_dialog);
					break;

				case ID_BUTTON_CLOSE:
					this->queue_close_view();
					break;

				case ID_BUTTON_MSGBOX:
					uimsgbox *msgbox = this->create_messagebox();
					if (msgbox != NULL)
						this->queue_open_view(msgbox);
					break;

				case ID_BUTTON_DLGOPEN:
					m_opensave = this->create_opensave(
						uiopensave::MODE_OPEN
					);
					if (m_opensave != NULL)
						this->queue_open_view(m_opensave);
					break;

				case ID_BUTTON_DLGSAVE:
					m_opensave = this->create_opensave(
						uiopensave::MODE_SAVE
					);
					if (m_opensave != NULL)
						this->queue_open_view(m_opensave);
					break;
			}
			break;

		case tui::TUIA_CTRL_VALUE_CHANGED:
			if (action->id == ID_EDIT)
				m_list->set_highlight(m_edit->get_text());
			break;
	}

	return TRUE;
}
