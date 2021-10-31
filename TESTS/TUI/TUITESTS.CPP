#include <stdio.h>
#include <conio.h>
#include <dos.h>

#include "keyboard.hpp"

#include "tui/tui.hpp"
#include "tui/tuitests.hpp"
#include "tui/testview.hpp"

bool_t tui_tests(bool_t text_mode) {
	if (keyboard::init() == FALSE) {
		printf("\nCannot initialize keyboard.\n");
		return FALSE;
	}

	if (text_mode == FALSE)
		vga::set_mode(0x12);
	else
		clrscr();

	tui *ui = new tui(text_mode);
	if (ui == NULL) {
		printf("Cannot allocate user interface.");
		return FALSE;
	}

	uiview *test = new testview(2, 2, 76, 21);
	if (test == NULL) {
		printf("Cannot allocate test view.");
		delete ui;
		return FALSE;
	}

	ui->open_view(test);

	while (ui->task() == TRUE) {
		/*
		tui::action_t *action;
		if (action->type == 0)
			continue;

		int x = wherex(), y = wherey();
		gotoxy(1, 1);

		printf("action:%d, id:%d, item:%d control:%p", action->type,
			   action->id, action->item_id, action->control);

		gotoxy(x, y);
		*/
	}

	delete ui;

	keyboard::release();

	return TRUE;
}
