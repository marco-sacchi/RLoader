#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <dir.h>
#include <io.h>
#include <dos.h>

#include "math.hpp"
#include "vga.hpp"
#include "graphics.hpp"
#include "bitmap.hpp"
#include "config.hpp"
#include "list.hpp"
#include "ui.hpp"

#ifdef TESTS

bool_t tests();

#endif

#define VERSION		0x0065

/** Batch creation modes. */
enum batch_mode {
	/** Create batch file from entry to launch program. */
	BATCH_EXECUTE = 1,
	/** Create batch file from entry to launch configuration program. */
	BATCH_SETUP = 2
};

/**  Draw logo on right side of the screen when in 80 columns text mode. */
void draw_logo();
/** Draw and fade in/out the splash screen. */
void draw_splash_screen();
/** Create batch file to launch program or its setup executable.
 *
 * @param parent This program.
 * @param entry List entry for which create the batch file.
 * @param mode Batch creation mode (lauch or setup).
 * @return True on success, false on program not found.
 */
bool_t create_batch(const char *parent, const list_entry *entry,
					batch_mode mode);
/** Write commands to change drive, working directory and execute program.
 *
 * @param full_path Fully-qualified path with drive and program filename
 *                  with extension.
 * @param fp File pointer of batch file to be written.
 */
void write_cwd_and_exec(const char *full_path, FILE *fp);
/** Write commands to change drive and current working directory.
 *
 * @param full_path Fully-qualified path with drive. Filename, if
 *                  specified will be ignored.
 * @param fp File pointer of batch file to be written.
 */
void write_cwd(const char *full_path, FILE *fp);
/** Parse program arguments.
 *
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @param list_name Valorized with list name to load, readed from
 *        arguments or environment variable RLOADER_LIST
 */
void parse_args(int argc, const char *argv[], const char **list_name);
/** Display usage short help. */
void print_usage();

const char *config_file = "res/rloader.cfg";
const char *splash_file = "res/splash.bmp";
const char *ansi80_splash_file = "res/splash80.bin";
const char *ansi40_splash_file = "res/splash40.bin";
const char *temp_batch = "rloadrun.bat";
const char *help_hint = "type RLOADER /? for help";

/** Flag to track execution inside DOSBox or not, parsed form arguments. */
bool_t dosbox = FALSE;
/** Flag to track need to display the splash screen, parsed from arguments. */
bool_t show_splash = TRUE;
/** Flag to track use of text-mode UI without thumbnails. */
bool_t text_mode = FALSE;
/** Configuration object readed from configuration file. */
config *conf = NULL;

int main(int argc, const char *argv[]) {
	if (access(temp_batch, 0) == 0)
		unlink(temp_batch);

	char *list_name;
	parse_args(argc, argv, &list_name);

	conf = new config();
	if (conf == NULL || conf->load(config_file) == FALSE) {
		printf("Cannot find or parse configuration file %s.\n", config_file);
		if (conf != NULL)
			delete conf;
		exit(-1);
	}

	list *test_list = new list();
	if (test_list == NULL || test_list->load(list_name) == FALSE) {
		printf("Cannot allocate or parse list '%s'.\n", list_name);
		if (test_list != NULL)
			delete test_list;
		delete conf;
		exit(-1);
	}

	vga::init();

	const vga::color_t *bg_color = conf->get_background_color();
	const vga::color_t *fg_color = conf->get_foreground_color();
	uint8_t bg_color_index = 0, fg_color_index = 1;

	const vga::state_t *vga_state;
	vga_state = vga::get_current_state();

	if (text_mode) {
		fg_color_index = 7;

		if (vga_state->is_grayscale == TRUE)
			bg_color_index = 0;
		else
			bg_color_index = 1;
	}

	ui *interface = new ui(test_list, text_mode, bg_color_index,
						   fg_color_index);
	if (interface == NULL) {
		printf("Cannot allocate user interface.\n");
		delete test_list;
		delete conf;
		exit(-1);
	}

	if (text_mode == FALSE)
		vga::set_mode(0x12);

	vga_state = vga::get_current_state();

	vga::set_blinking(FALSE);
	vga::enable_cursor(FALSE);

	if (show_splash)
		draw_splash_screen();

	if (text_mode == FALSE) {
		vga::set_ega_color(bg_color_index, *bg_color);
		vga::set_ega_color(fg_color_index, *bg_color);
	} else {
		if (vga_state->columns == 80)
			draw_logo();
	}

	vga::text_rect_t rect;
	rect.top = 0;
	rect.left = 0;
	rect.right = max(40, vga_state->columns >> 1) - 1;
	rect.bottom = vga_state->rows - 1;
	interface->set_panel_rect(&rect);

	int ch;
	char search[UI_FILTER_STR_SIZE];
	char *search_ptr = search;
	bool_t exit = FALSE;
	memset(search, 0, UI_FILTER_STR_SIZE);

	interface->draw_panel();
	interface->filter_list(search);

	if (text_mode == FALSE && show_splash == TRUE) {
		vga::color_t lerp;

		for (float t = 0.0f; t <= 1.0f; t += 0.1f) {
			graphics::lerp_color(*bg_color, *fg_color, t, &lerp);
			vga::set_ega_color(1, lerp);
			delay(25);
		}
		vga::set_ega_color(1, *fg_color);
	}

	while (exit == FALSE) {
		ch = getch();
		if (ch == 0x1b && interface->is_info_displayed() == TRUE) {
			interface->hide_info();
			continue;
		}

		if (ch == 0) {
			ch = getch();

			switch (ch) {
				case 0x3b:
					interface->display_help();
					break;

				case 0x3c:
					interface->loop_thumbnail();
					break;

				case 0x3d:
					interface->display_selected_entry_info();
					break;

				case 0x3f:
					if (interface->is_info_displayed())
						break;

					const list_entry *entry = interface->get_selected_entry();

					if (entry->get_setup()[0] == '-') {
						interface->display_no_setup();
						break;
					}

					if (create_batch(argv[0], entry, BATCH_SETUP) == TRUE)
						exit = TRUE;

					break;

				case 0x44:
					exit = TRUE;
					break;

				case 0x47:
					interface->jump_to_begin();
					break;

				case 0x4f:
					interface->jump_to_end();
					break;

				case 0x49:
					interface->scroll(-10);
					break;

				case 0x51:
					interface->scroll(10);
					break;

				case 0x48:
				case 0x4b:
					interface->scroll(-1);
					break;

				case 0x4d:
				case 0x50:
					interface->scroll(1);
					break;
			}
		} else if (interface->is_info_displayed() == FALSE) {
			if (ch == 0x0d) {
				const list_entry *entry = interface->get_selected_entry();
				if (entry != NULL) {
					if (create_batch(argv[0], entry, BATCH_EXECUTE) == TRUE)
						exit = TRUE;

					interface->display_cannot_launch(entry);
				}
			} else if (ch == 0x08) {
				if (search_ptr > search) {
					*(--search_ptr) = 0x00;
					interface->filter_list(search);
				}
			} else if (ch >= 0x20 && (search_ptr - search) < UI_FILTER_STR_SIZE - 1) {
				*search_ptr++ = (char)ch;
				*search_ptr = 0x00;
				interface->filter_list(search);
			}
		}
	}

	delete interface;
	delete test_list;

	if (text_mode == TRUE) {
		clrscr();
		vga::set_blinking(TRUE);
		vga::enable_cursor(TRUE);
	} else
		vga::set_mode(0x03);

	return 0;
}

void draw_logo() {
	ansi_picture *pic = new ansi_picture();
	if (pic == NULL || pic->load(ansi40_splash_file, 40) == FALSE) {
		if (pic != NULL)
			delete pic;

		return;
	}

	graphics::draw(pic, 0, 40);
	delete pic;
}

void draw_splash_screen() {
	bitmap *bmp;
	ansi_picture *pic;

	if (text_mode == FALSE) {
		if (access(splash_file, 0) != 0)
			return;

		bmp = new bitmap();
		if (bmp == NULL || bmp->load(splash_file) == FALSE) {
			if (bmp != NULL)
				delete bmp;

			return;
		}

		graphics::set_ega_palette(bmp);
		graphics::draw(bmp, (640 >> 1) - (bmp->get_width() >> 1),
					   (480 >> 1) - (bmp->get_height() >> 1));

		vga::color_t black = {0, 0, 0};
		graphics::ega_fade_in(bmp, black, 500);
	} else {
		const char *ansi_file = ansi80_splash_file;
		const vga::state_t *vga_state;

		vga_state = vga::get_current_state();

		if (vga_state->columns < 80)
			ansi_file = ansi40_splash_file;

		if (access(ansi_file, 0) != 0)
			return;

		pic = new ansi_picture();
		if (pic == NULL || pic->load(ansi_file, vga_state->columns) == FALSE) {
			if (pic != NULL)
				delete pic;

			return;
		}

		clrscr();
		graphics::draw(pic,
					   (vga_state->rows - pic->get_height()) >> 1,
					   (vga_state->columns - pic->get_width()) >> 1);
	}

	for (int i = 0; i < 20; ++i) {
		if (kbhit() != 0) {
			getch();
			break;
		}

		delay(100);
	}

	if (text_mode == FALSE) {
		const vga::color_t *color = conf->get_background_color();
		graphics::ega_fade_out(bmp, *color, 500);

		delete bmp;
	} else
		delete pic;
}

bool_t create_batch(const char *parent, const list_entry *entry, batch_mode mode) {
	char full_path[MAXPATH];

	strcpy(full_path, entry->get_path());
	strcat(full_path, "\\");

	if (mode == BATCH_EXECUTE)
		strcat(full_path, entry->get_executable());
	else if (mode == BATCH_SETUP)
		strcat(full_path, entry->get_setup());
	else
		return FALSE;

	if (access(full_path, 0) != 0)
		return FALSE;

	FILE *fp = fopen(temp_batch, "wt");
	if (!fp)
		return FALSE;

	fputs("@echo off\n", fp);

	if (dosbox) {
		if (entry->get_cycles() > 0)
			fprintf(fp, "cycles fixed %d\n", entry->get_cycles());
		else
			fprintf(fp, "cycles auto\n");
	}

	*full_path = conf->get_drive_mapping(*full_path);

	write_cwd_and_exec(full_path, fp);
	fputc('\n', fp);

	write_cwd(parent, fp);
	fputc('\n', fp);

	if (dosbox)
		fputs("cycles fixed 20000\n", fp);

	fflush(fp);
	fclose(fp);

	return TRUE;
}

void write_cwd_and_exec(const char *full_path, FILE *fp) {
	// Set current working directory.
	write_cwd(full_path, fp);

	const char *extension = strrchr(full_path, '.');

	// Ensure that a runned batch file will return to this generated batch.
	if (extension != NULL && stricmp(extension, ".bat") == 0)
		fputs("call ", fp);

	// Run program.
	const char *path_split = strrchr(full_path, '\\');
	fputs(path_split + 1, fp);
	fputc('\n', fp);
}

void write_cwd(const char *full_path, FILE *fp) {
	// Set drive.
	fputc(full_path[0], fp);
	fputc(full_path[1], fp);
	fputc('\n', fp);

	// Set current working directory.
	const char *path_split = strrchr(full_path, '\\');

	long dir_len = path_split - full_path - 3;
	if (dir_len > 0) {
		fputs("cd\\", fp);
		fwrite(full_path + 3, (size_t)dir_len, 1, fp);
		fputc('\n', fp);
	}
}

void parse_args(int argc, const char *argv[], const char **list_name) {
	*list_name = getenv("RLOADER_LIST");

	if (argc > 1) {
		for (int i = 1; i < argc; ++i) {
			if (argv[i][0] == '/') {
				if (argv[i][1] == '?' || argv[i][1] == 'h') {
					print_usage();
					exit(0);
				} else if (strcmp(argv[i], "/dosbox") == 0) {
					dosbox = TRUE;
				} else if (strcmp(argv[i], "/textmode") == 0) {
					text_mode = TRUE;
				} else if (strcmp(argv[i], "/nosplash") == 0) {
					show_splash = FALSE;
				} else {
					printf("Unknown option %s, %s.\n",
						argv[i], help_hint);
					exit(-1);
				}
			} else {
				*list_name = argv[i];
			}
		}
	}

	if (*list_name == NULL) {
		print_usage();
		exit(-1);
	}

	if (strpbrk(*list_name, "<>,\\|/") != NULL) {
		printf("Invalid list name '%s', %s.\n", *list_name, help_hint);
		exit(-1);
	}
}

void print_usage() {
	printf("RLOADER Version %x.%02x\n2021 Marco Sacchi\n\n"
		   "Syntax: RLOADER [/?|/h|/dosbox] [list_name]\n\n"
		   "/?, /h     Display this help.\n"
		   "/dosbox    Can be specified when this program run on DOSBox.\n"
		   "/textmode  Use a text-based UI intead of a graphical one.\n"
		   "/nosplash  Do not show splash screen.\n"
		   "           it sets correct CPU cycles for launched item.\n"
		   "list_name  name of list to load, automatically readed also from\n"
		   "           RLOADER_LIST environment variable if set.\n\n"
		   "NOTE: options are case-sensitive.\n\n",
		   VERSION >> 8, VERSION & 0xFF);
}

#ifdef TESTS

bool_t tests() {
	vga::set_cursor_type(0, 15);
	vga::set_cursor_pos(3, 16);

	uint8_t attrs = vga::encode_attrs(0, 1, 0);

	vga::write_char('c', attrs, 40);

	getch();

	vga::text_rect_t rect;
	rect.top = 0;
	rect.left = 0;
	rect.bottom = 24;
	rect.right = 39;
	vga::scroll_page_up(1, attrs, &rect);

	vga::set_cursor_pos(24, 16);
	vga::write_char('a', attrs, 2);
	vga::write_string("Test string", attrs, vga::USE_ATTRS_UPDATE_CURSOR);

	getch();

	bitmap *bmp = new bitmap();

	if (bmp->load("lists/games/titles/doom.bmp") == FALSE) {
		delete bmp;
		vga::set_mode(0x03);
		return FALSE;
	}

	graphics::set_ega_palette(bmp);
	graphics::draw(bmp, 320, 30 * 16 - bmp->get_height() / 2);
	delete bmp;

	getch();

	rect.top = 0;
	rect.left = 40;
	rect.bottom = 39;
	rect.right = 79;
	vga::set_map_mask_reg(0x0f);

	attrs = vga::encode_attrs(0, 0, 0);

	int i = 15;
	while (i-- > 0) {
		vga::scroll_page_up(1, attrs, &rect);
		delay(10);
	}

	getch();

	i = 15;
	while (i-- > 0) {
		vga::scroll_page_up(1, attrs, &rect);
		delay(10);
	}

	getch();

	return TRUE;
}

#endif