#ifndef OPENSAVE_HPP
#define OPENSAVE_HPP

#include <dir.h>

#include "types.hpp"
#include "tui/view.hpp"

// Forward declarations.
class uilabel;
class uieditbox;
class uilistbox;
class uibutton;
class uimsgbox;

/** Open/Save file view. */
class uiopensave : public uiview {
public:
	/** Working modes. */
	enum mode {
		/** Open/Select file. */
		MODE_OPEN,
		/** Save existing or new file. */
		MODE_SAVE
	};

	/** Open/Save file view.
	 *
	 * @see uiopensave::mode
	 * @param mode Open or save working mode.
	 * @param filepath Start fully qualified path or filename,
	 *                 null for current working directory.
	 * @param Wildcard to filter displayed files.
	 */
	uiopensave(uiopensave::mode mode, const char *filepath,
			   const char *wildcard);

	/** Get selected file.
	 *
	 * @param buffer Filled with fully qualified path to selected filename
	 * @return TRUE if file has been selected, FALSE otherwise.
	 */
	bool_t get_selected_fullpath(char *buffer);

	virtual bool_t init_controls();
	virtual bool_t on_action(tui::action_t *action);

private:
	enum CTRL_ID {
		ID_WINDOW = 1,
		ID_LBL_FILENAME,
		ID_EDIT_FILENAME,
		ID_LBL_FILE_FULLPATH,
		ID_LBL_FILE_INFO,
		ID_LBL_FILES,
		ID_LIST_FILES,
		ID_LBL_DIRS,
		ID_LIST_DIRS,
		ID_BTN_OK,
		ID_BTN_CANCEL
	};

	/** Normalize current path to uppercase and backslash as separators. */
	void uiopensave::normalize_curr_path();

	/** Check if specified path is a valid root path on existing drive.
	 *
	 * @param path Path to test.
	 * @return TRUE if valid root path (X:\), FALSE otherwise.
	 */
	bool_t uiopensave::is_valid_root_path(const char *path);

	/** Append directory to m_curr_path.
	 *
	 * @param dir directory to append.
	 */
	void push_directory(const char *dir);
	/** Remove inner directory from path.
	 *
	 * @return Removed directory, NULL when on root directory.
	 */
	const char *pop_directory();

	/** Populate files listbox control.
	 *
	 * @return TRUE on success, FALSE on allocation error.
	 */
	bool_t populate_files();
	/** Populate directories and drives listbox control.
	 *
	 * @return TRUE on success, FALSE on allocation error.
	 */
	bool_t populate_dirs();

	/** Get and format file info.
	 *
	 * @param filename Fully qualified filename.
	 * @param buffer Filled with formatted info.
	 */
	void get_file_info(const char *filename, char *buffer);

	/** Check if given filename is valid.
	 *
	 * @param filename Filename to test.
	 * @return TRUE if valid, FALSE otherwise.
	 */
	bool_t is_valid_filename(const char *filename);
	/** Check if current selected file is valid.
	 *
	 * @return TRUE if file have a valid filename and is in an existing
	 *         path, FALSE otherwise.
	 */
	bool_t valid_file_selected();

	/** Create the messagebox to ask the user whether to overwrite the file.
	 *
	 * @return Message box object, NULL on allocation error.
	 */
	uimsgbox *create_overwrite_msgbox();

	/** Current working mode. */
	mode m_mode;

	//TODO: char *m_caption;
	/** Previous path to be restored after a critical error. */
	char m_prev_path[MAXPATH];
	/** Path for which files are displayed. */
	char m_curr_path[MAXPATH];
	/** Wildcard to filter displayed files. */
	char m_curr_wildcard[MAXFILE + MAXEXT - 1];
	/** TRUE if cancel button is pressed, FALSE otherwise. */
	bool_t m_canceled;
	char m_start_filename[MAXFILE + MAXEXT - 1];

	uilabel *m_lbl_filename;
	uieditbox *m_edit_filename;
	uilabel *m_lbl_file_fullpath;
	uilabel *m_lbl_file_info;
	uilabel *m_lbl_files;
	uilistbox *m_list_files;
	uilabel *m_lbl_dirs;
	uilistbox *m_list_dirs;
	uibutton *m_btn_ok;
	uibutton *m_btn_cancel;

	uimsgbox *m_msgbox;
};

#endif
