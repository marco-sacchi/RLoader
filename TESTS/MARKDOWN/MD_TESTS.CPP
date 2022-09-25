#include <conio.h>
#include <stdio.h>

#include "markdown/md_tests.hpp"

#include "markdown.hpp"
#include "vga.hpp"

bool_t receiver(markdown::token_t *token);

bool_t md_tests(bool_t text_mode) {
	if (text_mode == FALSE)
		vga::set_mode(0x12);
	else
		clrscr();

	FILE *output = fopen("preparse.md$", "wb");
	if (output == NULL) {
		printf("Cannot create preparse.md$ file.\n");
		return FALSE;
	}

	markdown *parser = new markdown();
	parser->set_source("../readme.md");
	parser->set_width(40);
	parser->set_token_receiver(receiver);

    clrscr();

	if (parser->parse() == FALSE) {
		fclose(output);
		delete parser;
		return FALSE;
	}

	int lines = parser->get_lines();
	delete parser;

	fclose(output);

	return TRUE;
}

bool_t receiver(markdown::token_t *token) {


	textbackground(token->block);
	/*
		B_PARAGRAPH,
		B_HEADING,
		B_OLIST,
		B_ULIST,
		B_LIST_ITEM,
		B_CODE
	*/
	textcolor(LIGHTGRAY);

	if (token->style & markdown::S_EMPHASIS)
		textcolor(MAGENTA);

	if (token->style & markdown::S_STRONG)
		textcolor(WHITE);

	if (token->style & markdown::S_CODE) {
		textcolor(BLACK);
		textbackground(LIGHTGRAY);
	}

	switch (token->type) {
		case markdown::T_BLOCK_START:
			break;

		case markdown::T_BLOCK_END:
	        textbackground(BLACK);
			cprintf("\r\n\r\n");
			break;

		case markdown::T_SPAN:
			if (token->block == markdown::B_CODE) {
				int row = wherey();
				for (int i = 0; i < 40; ++i)
					putch(' ');
				gotoxy(1, row);
			}

			cputs(token->text);
			break;

		case markdown::T_NEWLINE:
			textbackground(BLACK);
			cprintf("\r\n");
			break;
	}

	return TRUE;
}