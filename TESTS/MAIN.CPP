#include <stdio.h>
#include <conio.h>

#include "keyb/keytests.hpp"
#include "critical/critests.hpp"
#include "vga/vgatests.hpp"
#include "tui/tuitests.hpp"
#include "markdown/md_tests.hpp"

#include "string.hpp"
#include "vga.hpp"

int main(int argc, const char *argv[]) {
	int selection = 0;

	if (argc > 1 && string::is_digit(*argv[1])) {
		selection = *argv[1] - '0';
	} else {
		printf("Select test to run:\n"
			   "1 - Keyboard\n"
			   "2 - Custom critical error handler\n"
			   "3 - VGA\n"
			   "4 - Text-mode user interface\n"
			   "5 - Graphics-mode user interface\n"
			   "6 - Text-mode markdown\n"
			   "7 - Graphics-mode markdown\n");

		selection = getch() - '0';
	}

	printf("Test %d selected.\n\n", selection);

	vga::init();

	bool_t result;
	switch (selection) {
		case 1:
			result = keyb_tests();
			break;

		case 2:
			result = critical_tests();
			break;

		case 3:
			result = vga_tests();
			break;

		case 4:
			result = tui_tests(TRUE);
			break;

		case 5:
			result = tui_tests(FALSE);
			break;

		case 6:
			result = md_tests(TRUE);
			break;

		case 7:
			result = md_tests(FALSE);
			break;

		default:
			printf("\nInvalid choice: %d\nExiting.", selection);
			return 1;
	}

	if (result == TRUE)
		printf("\n Test PASSED.\n");
	else
		printf("\n Test FAILED.\n");

	return (result == FALSE);
}

