#include <conio.h>
#include <stdio.h>

#include "markdown/md_tests.hpp"

#include "markdown.hpp"
#include "vga.hpp"

bool_t receiver(markdown::token_t *token);

static markdown::indents_t indents;

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

	indents[markdown::B_PARAGRAPH] = 0;
	indents[markdown::B_HEADING] = 0;
	indents[markdown::B_OLIST] = 0;
	indents[markdown::B_ULIST] = 0;
	indents[markdown::B_LIST_ITEM] = 4;
	indents[markdown::B_CODE] = 0;

	markdown *parser = new markdown(40, receiver, indents);
	parser->set_source("../readme.md");

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
	textbackground(BLACK);
	textcolor(LIGHTGRAY);

	int x = wherex(), y = wherey();
	gotoxy(41, y);
	putch('|');
	gotoxy(x, y);

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
			textbackground(BLACK);
			if (indents[token->block] > 0)
				cprintf("\r\n%*c", indents[token->block], ' ');
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
			if (indents[token->block] > 0)
				cprintf("%*c", indents[token->block], ' ');
			break;
	}

	return TRUE;
}