#include <stdio.h>
#include <conio.h>

#include "keyb/keytests.hpp"

#include "vga.hpp"
#include "keyboard.hpp"

#define FLAG_TO_CHAR(value) ((value) ? 'X' : '-')

void print_keyb_read(keyboard::read_data_t *data);
void print_keyb_make_break(keyboard::read_data_t *data);

bool_t keyb_tests() {
	bool_t result = TRUE;

	keyboard::read_data_t data;

	clrscr();

	keyboard::init();

	bool_t loop = TRUE;
	while (loop == TRUE) {
		if (keyboard::read(&data) == FALSE)
			continue;

		switch (data.type) {
			case keyboard::KEV_MAKE:
				print_keyb_make_break(&data);
				break;

			case keyboard::KEV_PRESS:
				print_keyb_read(&data);

				if (data.virt_key == VK_ESC && (data.shifts & VS_CTRL)) {
					loop = FALSE;
					result = TRUE;
					printf("\nCTRL+ESC pressed, exiting.\n");
				}
				break;

			case keyboard::KEV_BREAK:
				print_keyb_make_break(&data);
				break;
		}
	}

	keyboard::release();

	return result;
}

void print_keyb_make_break(keyboard::read_data_t *data) {
	printf("make:[%c] scan:%02x - vkey:%02x\n",
		   FLAG_TO_CHAR(data->ascii),
		   data->scan_code, data->virt_key);
}

void print_keyb_read(keyboard::read_data_t *data) {
	printf("ascii:'%c' %02x - scan:%02x - vkey:%02x\n",
		   data->ascii, data->ascii, data->scan_code, data->virt_key);

	printf(" pressed: SHIFT(L,R):%c,%c CTRL(L,R):%c,%c ALT(L,R):%c,%c\n"
		   "          INS: %c CAPS:%c NUM:%c SCROLL:%c PAUSE:%c\n",
		   FLAG_TO_CHAR(data->shifts & VS_LSHIFT),
		   FLAG_TO_CHAR(data->shifts & VS_RSHIFT),
		   FLAG_TO_CHAR(data->shifts & VS_LCTRL),
		   FLAG_TO_CHAR(data->shifts & VS_RCTRL),
		   FLAG_TO_CHAR(data->shifts & VS_LALT),
		   FLAG_TO_CHAR(data->shifts & VS_RALT),
		   FLAG_TO_CHAR(data->shifts & VS_INSERT),
		   FLAG_TO_CHAR(data->shifts & VS_CAPSLOCK),
		   FLAG_TO_CHAR(data->shifts & VS_NUMLOCK),
		   FLAG_TO_CHAR(data->shifts & VS_SCRLLOCK),
		   FLAG_TO_CHAR(data->shifts & VS_PAUSE));

	printf("  locked: INS: %c CAPS:%c NUM:%c SCROLL:%c\n",
		   FLAG_TO_CHAR(data->locked_shifts & VS_INSERT),
		   FLAG_TO_CHAR(data->locked_shifts & VS_CAPSLOCK),
		   FLAG_TO_CHAR(data->locked_shifts & VS_NUMLOCK),
		   FLAG_TO_CHAR(data->locked_shifts & VS_SCRLLOCK));
}
