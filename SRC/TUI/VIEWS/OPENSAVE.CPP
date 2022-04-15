#include <string.h>
#include <stdio.h>
#include <dir.h>
#include <dos.h>
#include <io.h>

#include "string.hpp"

#include "tui/views/opensave.hpp"
#include "tui/window.hpp"
#include "tui/msgbox.hpp"
#include "tui/button.hpp"
#include "tui/listbox.hpp"
#include "tui/editbox.hpp"
#include "tui/label.hpp"

static const char *months[] = {
	"---",
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

#define SECONDS_PER_DAY 86400L

uiopensave::uiopensave(uiopensave::mode mode, const char *filepath,
					   const char *wildcard)
 : uiview() {
	m_is_modal = TRUE;

	m_canceled = FALSE;
	m_mode = mode;

	bool_t valid_path = FALSE;

	memset(m_curr_path, 0x00, MAXPATH);
	memset(m_start_filename, 0x00, MAXFILE + MAXEXT - 1);

	if (filepath != NULL) {
		strncpy(m_curr_path, filepath, MAXPATH - 1);
		this->normalize_curr_path();

		if (this->is_valid_root_path(m_curr_path) == TRUE)
			valid_path = TRUE;
		else {
			if (string::last_char(m_curr_path) == '\\')
				*(m_curr_path + strlen(m_curr_path) - 1) = NULL;

			char *last_slash = strrchr(m_curr_path, '\\');
			valid_path = this->is_valid_filename(last_slash + 1);

			struct ffblk file;
			if (valid_path && findfirst(m_curr_path, &file, FA_DIREC) == 0) {
				valid_path = TRUE;
				if ((file.ff_attrib & FA_DIREC) == 0) {
					const char *dir = this->pop_directory();
					if (dir != NULL)
						strcpy(m_start_filename, dir);

					valid_path = valid_path && dir != NULL;
				}
			}
		}
	}

	if (valid_path == FALSE)
		getcwd(m_curr_path, MAXPATH - 1);

	strcpy(m_prev_path, m_curr_path);

	strcpy(m_curr_wildcard, wildcard);
}

bool_t uiopensave::init_controls() {
	char *ok_btn_caption, *window_caption;
	char ok_btn_accelerator;
	if (m_mode == MODE_OPEN) {
		window_caption = "Open";
		ok_btn_caption = "Open";
		ok_btn_accelerator = 'O';
	} else {
		window_caption = "Save";
		ok_btn_caption = "Save";
		ok_btn_accelerator = 'S';
	}

	m_window = new uiwindow(ID_WINDOW, 0, 0, 48, 19,
							uiwindow::FRAMED | uiwindow::CAPTIONED |
							uiwindow::CENTERED,
							window_caption);
	if (m_window == NULL)
		return FALSE;

	m_lbl_filename = new uilabel(ID_LBL_FILENAME, 2, 2, "File name:");
	if (m_lbl_filename == NULL)
		return FALSE;

	m_lbl_filename->set_accelerator('N');
	m_window->add_control(m_lbl_filename);

	m_edit_filename = new uieditbox(ID_EDIT_FILENAME, 2, 13, 32,
									MAXFILE + MAXEXT - 2);
	if (m_edit_filename == NULL)
		return FALSE;

	m_edit_filename->set_accelerator('N');
	m_window->add_control(m_edit_filename);

	m_lbl_file_fullpath = new uilabel(ID_LBL_FILE_FULLPATH, 4, 2, 43, 1,
									  m_curr_path);
	if (m_lbl_file_fullpath == NULL)
		return FALSE;

	m_window->add_control(m_lbl_file_fullpath);

	m_lbl_file_info = new uilabel(ID_LBL_FILE_INFO, 5, 2, 43, 1, "");
	if (m_lbl_file_info == NULL)
		return FALSE;

	m_window->add_control(m_lbl_file_info);

	m_lbl_files = new uilabel(ID_LBL_FILES, 7, 2, "Files:");
	if (m_lbl_files == NULL)
		return FALSE;

	m_lbl_files->set_accelerator('F');
	m_window->add_control(m_lbl_files);

	m_list_files = new uilistbox(ID_LIST_FILES, 9, 2, 26, 7);
	if (m_list_files == NULL)
		return FALSE;

	m_list_files->set_accelerator('F');
	m_list_files->set_columns(2);
	m_list_files->set_sort_mode(uilistbox::SORT_ASCENDING);
	m_window->add_control(m_list_files);

	if (this->populate_files() == FALSE)
		return FALSE;

	m_start_filename[0] = NULL;

	m_lbl_dirs = new uilabel(ID_LBL_DIRS, 7, 32, "Directories:");
	if (m_lbl_dirs == NULL)
		return FALSE;

	m_lbl_dirs->set_accelerator('D');
	m_window->add_control(m_lbl_dirs);

	m_list_dirs = new uilistbox(ID_LIST_DIRS, 9, 32, 13, 7);
	if (m_list_dirs == NULL)
		return FALSE;

	m_list_dirs->set_accelerator('D');
	m_list_dirs->set_sort_mode(uilistbox::SORT_ASCENDING);
	m_window->add_control(m_list_dirs);

	if (this->populate_dirs() == FALSE)
		return FALSE;

	m_btn_ok = new uibutton(ID_BTN_OK, 17, 23, 10, ok_btn_caption);
	if (m_btn_ok == NULL)
		return FALSE;

	m_btn_ok->set_accelerator(ok_btn_accelerator);
	m_window->add_control(m_btn_ok);

	m_btn_cancel = new uibutton(ID_BTN_CANCEL, 17, 35, 10, "Cancel");
	if (m_btn_cancel == NULL)
		return FALSE;

	m_btn_cancel->set_accelerator('C');
	m_window->add_control(m_btn_cancel);

	return TRUE;
}

bool_t uiopensave::get_selected_fullpath(char *buffer) {
	if (m_canceled == TRUE)
		return FALSE;

	this->push_directory(m_edit_filename->get_text());
	strcpy(buffer, m_curr_path);
	this->pop_directory();

	return TRUE;
}

void uiopensave::normalize_curr_path() {
	strupr(m_curr_path);

	char *ptr = m_curr_path;
	while (*ptr != NULL) {
		if (*ptr == '/')
			*ptr = '\\';

		++ptr;
	}
}

bool_t uiopensave::is_valid_root_path(const char *path) {
	int curr_drive = getdisk();
	bool_t valid = (strlen(path) == 3 &&
					strcmp(path + 1, ":\\") == 0 &&
					setdisk(path[0] - 'A') &&
					getdisk() == path[0] - 'A');

	setdisk(curr_drive);

	return valid;
}

void uiopensave::push_directory(const char *dir) {
	if (string::last_char(m_curr_path) != '\\')
		strcat(m_curr_path, "\\");

	strcat(m_curr_path, dir);
}

const char *uiopensave::pop_directory() {
	char *last_separator = strrchr(m_curr_path, '\\');
	if (last_separator == NULL)
		return NULL;

	bool_t after_drive = (last_separator > m_curr_path &&
						  *(last_separator - 1) == ':');

	if (after_drive)
		++last_separator;

	*last_separator = NULL;
	return last_separator + 1;
}

bool_t uiopensave::populate_files() {
	m_list_files->clear();

	char buffer[MAXPATH];
	strcpy(buffer, m_curr_path);
	if (string::last_char(buffer) != '\\')
		strcat(buffer, "\\");
	strcat(buffer, m_curr_wildcard);

	struct ffblk file;
	int done = findfirst(buffer, &file, 0);
	bool_t ok = TRUE;

	m_list_files->pause_repaint();

	while (done == 0) {
		if ((ok = m_list_files->add_item(file.ff_name)) == FALSE)
			break;

		done = findnext(&file);
	}

	m_list_files->resume_repaint();

	if (ok == FALSE)
		return FALSE;

	if (strlen(m_start_filename) > 0)
		m_list_files->set_selected_item(m_start_filename);

	const char *filename = m_list_files->get_selected_item_text();
	if (filename != NULL)
		m_edit_filename->set_text(filename);
	else
		m_edit_filename->set_text("");

	this->get_file_info(filename, buffer);
	m_lbl_file_info->set_text(buffer);

	return TRUE;
}

void uiopensave::get_file_info(const char *filename, char *buffer) {
	if (filename == NULL) {
		*buffer = NULL;
		return;
	}

	strcpy(buffer, m_curr_path);

	if (string::last_char(buffer) != '\\')
		strcat(buffer, "\\");

	strcat(buffer, filename);

	struct ffblk file;
	findfirst(buffer, &file, 0);

	int year = (file.ff_fdate >> 9) + 1980;
	int month = (file.ff_fdate >> 5) & 0x0f;
	int day = file.ff_fdate & 0x1f;

	int hours = (file.ff_ftime >> 11);
	int minutes = (file.ff_ftime >> 5) & 0x3f;

	char size_buffer[14], *ptr = size_buffer + 13;
	*ptr = NULL;
	long size = file.ff_fsize;

	if (size >= (1024L * 1024L)) {
		size >>= 10;
		*(--ptr) = 'K';
	}

	int group;
	while ((group = (int)(size % 1000)) > 0) {
		int digits = 3;
		while (group > 0) {
			*(--ptr) = (group % 10) + '0';
			group /= 10;
			--digits;
		}

		if (size < 1000)
			break;

		while (digits > 0) {
			*(--ptr) = '0';
			--digits;
		}

		*(--ptr) = '.';

		size /= 1000L;
	}

	sprintf(buffer, "%-12s %10s  %s %2d,%d  %02d:%02d",
			filename, ptr,
			months[month], day, year,
			hours, minutes);
}

bool_t uiopensave::populate_dirs() {
	m_list_dirs->clear();

	bool_t ok = TRUE;

	m_list_dirs->pause_repaint();

	char *drive_string = "[-A-]";
	int curr_drive = getdisk();
	int drives = setdisk(curr_drive);
	for (int drive = 0; drive < drives; ++drive) {
		setdisk(drive);
		if (drive != getdisk())
			continue;

		drive_string[2] = (char)('A' + drive);
		if ((ok = m_list_dirs->add_item(drive_string)) == FALSE)
			break;
	}

	if (ok == FALSE) {
		m_list_dirs->resume_repaint();
		return FALSE;
	}

	setdisk(curr_drive);

	char buffer[MAXPATH];
	strcpy(buffer, m_curr_path);

	if (string::last_char(buffer) != '\\')
		strcat(buffer, "\\");

	strcat(buffer, "*.*");

	struct ffblk dir;
	int done = findfirst(buffer, &dir, FA_DIREC);
	while (done == 0) {
		if ((dir.ff_attrib & FA_DIREC) && strcmp(dir.ff_name, ".") != 0) {
			if ((ok = m_list_dirs->add_item(dir.ff_name)) == FALSE)
				break;
		}

		done = findnext(&dir);
	}

	m_list_dirs->resume_repaint();

	if (ok == FALSE)
		return FALSE;

	m_lbl_file_fullpath->set_text(m_curr_path);

	return TRUE;
}

bool_t uiopensave::is_valid_filename(const char *filename) {
	if (strpbrk(filename, "<>:\"/\\|?* \t[]") != NULL)
		return FALSE;

	char *dot = strchr(filename, '.');
	int length = strlen(filename);
	if (length == 0 || dot == filename)
		return FALSE;

	if (dot == NULL)
		return (length < MAXFILE) ? TRUE : FALSE;

	if (strchr(dot + 1, '.') != NULL || (dot - (char *)filename) >= MAXFILE ||
		strlen(dot) >= MAXEXT)
		return FALSE;

	return TRUE;
}

bool_t uiopensave::valid_file_selected() {
	if (this->is_valid_filename(m_edit_filename->get_text()) == FALSE) {
		uimsgbox *msgbox = new uimsgbox("Invalid path",
										"Specified filename is invalid.");
		if (msgbox != NULL)
			this->queue_open_view(msgbox);
		return FALSE;
	}

	struct ffblk info;

	if (this->is_valid_root_path(m_curr_path) == FALSE) {
		if (strpbrk(m_curr_path, "?*") != NULL ||
			access(m_curr_path, 0) != 0 ||
			findfirst(m_curr_path, &info, FA_DIREC) != 0 ||
			(info.ff_attrib & FA_DIREC) == 0) {
			uimsgbox *msgbox = new uimsgbox("Invalid path",
											"Specified path not found.");
			if (msgbox != NULL)
				this->queue_open_view(msgbox);
			return FALSE;
		}
	}

	char fullpath[MAXPATH];
	this->get_selected_fullpath(fullpath);
	bool_t found = (strpbrk(fullpath, "?*") != NULL ||
				   (access(fullpath, 0) == 0));
	bool_t is_dir = (found &&
					(findfirst(fullpath, &info, FA_DIREC) != 0 ||
					(info.ff_attrib & FA_DIREC) != 0));

	if (m_mode == MODE_OPEN) {
		if (found == FALSE || is_dir == TRUE) {
			uimsgbox *msgbox = new uimsgbox("Invalid path",
											"Specified filename not found.");
			if (msgbox != NULL)
				this->queue_open_view(msgbox);
			return FALSE;
		}
	} else {
		if (is_dir == TRUE) {
			uimsgbox *msgbox = new uimsgbox("Invalid path",
											"Specified filename not found.");
			if (msgbox != NULL)
				this->queue_open_view(msgbox);
			return FALSE;
		}

		if (found == TRUE) {
			m_msgbox = this->create_overwrite_msgbox();
			if (m_msgbox != NULL)
				this->queue_open_view(m_msgbox);
			return FALSE;
		}
	}

	return TRUE;
}

uimsgbox *uiopensave::create_overwrite_msgbox() {
	uimsgbox *msgbox = new uimsgbox("File overwrite",
									"The specified file already exists, "
									"do you want to overwrite it?");
	if (msgbox == NULL)
		return NULL;

	if (!msgbox->add_button(uimsgbox::BTN_YES)) {
		delete msgbox;
		return NULL;
	}

	if (!msgbox->add_button(uimsgbox::BTN_NO)) {
		delete msgbox;
		return NULL;
	}

	return msgbox;
}

bool_t uiopensave::on_action(tui::action_t *action) {
	switch (action->type) {
		case tui::TUIA_CTRL_DESTROY:
			if (action->id == ID_WINDOW)
				return TRUE;
			break;

		case tui::TUIA_VIEW_CLOSED:
			// TODO: make clear that this case is for closed
			// messagebox of a critical error.
			if (m_msgbox == NULL || ((uimsgbox *)(action->view)) != m_msgbox) {

				/*if (critical->get_selected_button_id() != uimsgbox::BTN_RETRY) {
					strcpy(m_curr_path, m_prev_path);
					this->populate_files();
					this->populate_dirs();
				}*/
				break;
			}

			if (m_msgbox->get_selected_button_id() == uimsgbox::BTN_YES)
				this->queue_close_view();

			m_msgbox = NULL;
			break;

		case tui::TUIA_CTRL_ACTIVATED:
			switch (action->id) {
				case ID_EDIT_FILENAME:
				case ID_BTN_OK:
					if (this->valid_file_selected() == TRUE)
						this->queue_close_view();
					break;

				case ID_BTN_CANCEL:
					m_canceled = TRUE;
					this->queue_close_view();
					break;
			}
			break;

		case tui::TUIA_ITEM_SELECTED:
			switch (action->id) {
				case ID_LIST_FILES:
					const char *file = m_list_files->get_selected_item_text();
					m_edit_filename->set_text(file);
					char buffer[MAXPATH];
					this->get_file_info(file, buffer);
					m_lbl_file_info->set_text(buffer);
					break;
			}
			break;

		case tui::TUIA_ITEM_ACTIVATED:
			switch (action->id) {
				case ID_LIST_FILES:
					if (this->valid_file_selected() == TRUE)
						this->queue_close_view();
					break;

				case ID_LIST_DIRS:
					const char *dir = m_list_dirs->get_selected_item_text();

					strcpy(m_prev_path, m_curr_path);

					if (strcmp(dir, "..") == 0) {
						this->pop_directory();
					} else if (dir[0] == '[') {
						// Drive selected.

						// Selected drive is current displayed drive.
						if (dir[2] == m_curr_path[0])
							break;

						sprintf(m_curr_path, "%c:\\", dir[2]);
						getcurdir(dir[2] - 'A' + 1, m_curr_path + 3);
					} else
						this->push_directory(dir);

					this->populate_files();
					this->populate_dirs();
					break;
			}
			break;
	}

	return TRUE;
}
