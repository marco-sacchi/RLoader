#ifndef TUI_TESTVIEW_HPP
#define TUI_TESTVIEW_HPP

#include "tui/view.hpp"
#include "tui/window.hpp"
#include "tui/button.hpp"
#include "tui/label.hpp"
#include "tui/editbox.hpp"
#include "tui/listbox.hpp"

class testview : public uiview {
public:
	testview(int top, int left, int width, int height);

	const char *get_field_text();
	bool_t get_list_item(int *index, const char **text);

	virtual bool_t init_controls();
	virtual bool_t on_action(tui::action_t *action);

private:
	enum CTRL_ID {
		ID_WINDOW = 1,
		ID_BUTTON_OPEN,
		ID_BUTTON_CLOSE,
		ID_LABEL,
		ID_EDIT,
		ID_LIST
	};

	int m_top, m_left;
	int m_width, m_height;

	uibutton *m_button1, *m_button2;
	uilabel *m_label;
	uieditbox *m_edit;
	uilistbox *m_list;
	testview *m_dialog;
};

#endif
