#ifndef UI_HPP
#define UI_HPP

#include "list.hpp"
#include "vga.hpp"
#include "encoder.hpp"

#define UI_THUMBNAIL_PATH_SIZE	80
#define UI_FILTER_STR_SIZE		80

/** User interface handling object. */
class ui {

public:
	/** Initialize user interface.
	 *
	 * @param list List to be displayed.
	 * @param text_mode True for text-base UI, false for graphical UI.
	 * @param back_color Background color used to render UI elements.
	 * @param fore_color Foreground color used to render UI elements.
	 */
	ui(list *list, bool_t text_mode, uint8_t back_color, uint8_t fore_color);
	~ui();

	/** Run background tasks. */
	void task();

	/** Set and clear the rectangle that defines the UI area.
	 *
	 * @param rect Rectangle of UI area.
	 */
	void set_panel_rect(vga::text_rect_t *rect);
	/** Draw panel graphics. */
	void draw_panel();

	/** Filter rendered list displaying only matching elements.
	 *
	 * @param search String to search in list entries.
	 */
	void filter_list(const char *search);
	/** Scroll displayed list or info file.
	 *
	 * @param offset Number of rows to scroll up (negative) or down (positive)
	 *               from current position.
	 */
	void scroll(int offset);
	/** Jump to the beginning of displayed list or info file. */
	void jump_to_begin();
	/** Jump to the end of displayed list or info file. */
	void jump_to_end();

	/** Get selected list entry.
	 *
	 * @return Pointer to selected entry, NULL if no entry is selected.
	 */
	const list_entry *get_selected_entry();
	/** Set seleted list entry.
	 *
	 * @param entry Entry to select, if not in list first entry
	 *              will be selected.
	 */
	void set_selected_entry(list_entry *entry);
	/** Loop between titile and in-program thumbnails. */
	void loop_thumbnail();
	/** Get thumbnail loop index.
	 *
	 * @return Index of current thumbnail loop directory.
	 */
	int get_thumbnail_loop() { return m_thumbnail_loop; }
	/** Set thumbnail loop index.
	 */
	void set_thumbnail_loop(int index);

	/** Test if info file is displayed.
	 *
	 * @return TRUE if info file is displayed, FALSE otherwise.
	 */
	bool_t is_info_displayed() { return m_info_displayed; }
	/** Display help for this program.
	 *
	 * @return TRUE on success, FALSE otherwise.
	 */
	bool_t display_help();
	/** Display info file for selected entry (if exists).
	 *
	 * @return TRUE on success, FALSE otherwise.
	 */
	bool_t display_selected_entry_info();
	/** Hide info file panel and restore list view. */
	void hide_info();

	/** Display message for entry that cannot be launch.
	 *
	 * @param entry Entry to launch.
	 */
	void display_cannot_launch(const list_entry *entry);
	/** Display message for entry that cannot be setup.
	 *
	 * @param entry Entry to setup.
	 */
	void display_cannot_setup(const list_entry *entry);
	/** Display message to inform user that entry has no setup program. */
	void display_no_setup();

	int show_cycles_changed_dialog();
private:
	/** Scroll entry list.
	 *
	 * @param offset Number of entries to scroll up (negative) or down
	 *               (positive) from current position.
	 */
	void scroll_list(int offset);

	/** Draw search box with current search string. */
	void draw_search_box();
	/** Draw a shortcut on the bottom-right corner of panel rectangle.
	 *
	 * @param key String representation of key.
	 * @param text Descriptive text.
	 */
	void draw_shortcut(const char *key, const char *text);
	/** Draw list at current position .*/
	void draw_list();
	/** Draw list entry.
	 *
	 * @param line Line position inside list rectangle.
	 */
	void draw_list_entry(int line);
	/** Draw list scroll bar. */
	void draw_list_scroll_bar();
	/** XOR selected list entry to reverse colors. */
	void xor_selected_entry();
	/** Calculate list offset from selected entry index.
	 *
	 * @param selected_entry Selected entry index.
	 * @return Index of first list entry visible on top of list rectangle.
	 */
	int calc_list_offset(int selected_entry);

	/** Display specified info file.
	 *
	 * @param filename Fully-qualified filename with extension.
	 * @return TRUE on success, FALSE otherwise.
	 */
	bool_t display_info(const char *filename);
	/** Draw info panel content. */
	void draw_info();
	/** Draw info content line.
	 *
	 * @param row Line position inside info rectangle.
	 */
	void draw_info_line(int row);
	/** Test for a formatting marker boundary character.
	 *
	 * @param chr Character to test.
	 * @return TRUE if charater is a marker boundary, FALSE otherwise.
	 */
	bool_t is_marker_boundary(char chr);
	/** Release info file resources. */
	void release_info_resources();
	/** Scroll info file.
	 *
	 * @param offset Number of lines to scroll up (negative) or down
	 *               (positive) from current position.
	 */
	void scroll_info(int offset);

	/** Draw thumbnail for specified entry.
	 *
	 * @param entry Entry for which draw thumbnail.
	 */
	void draw_thumbnail(const list_entry *entry);

	/** Display message in thumbnail area clearing it.
	 *
	 * @note Message length cannot exceed thumbnail area width.
	 * @param message String messsage to display.
	 */
	void draw_thumbnail_message(const char *message);
	/** Clear thumbnail area with UI background color. */
	void clear_thumbnail_area();

	/** XOR charaters in video memory (graphics or text mode).
	 *
	 * @param row Zero-based row character coordinate.
	 * @param col Zero-based column character coordinate.
	 * @param count Number of characters to XOR.
	 */
	void xor_chars(uint8_t row, uint8_t col, uint16_t count);

	/** When true Text-based UI will be used, graphical one otherwise. */
	bool_t m_text_mode;

	/** UI panel recteangle. */
	vga::text_rect_t m_panel_rect;
	/** UI panel characters attributes. */
	uint8_t m_panel_attrs;

	/** UI blank characters attributes (used when scroll list/info views). */
	uint8_t m_blank_attrs;

	/** Array of list entries. */
	list *m_list;
	/** Array of pointers to list entries to filter the ones that matches. */
	list_entry **m_filtered;
	/** Number of matching list entries. */
	int m_filtered_count;
	/** Search string to search in list entries. */
	char m_filtered_string[UI_FILTER_STR_SIZE];
	/** List offset as index of first visible entry. */
	int m_offset;
	/** Index of selected list entry. */
	int m_selected_entry;
	/** List displaying rectangle. */
	vga::text_rect_t m_list_rect;

	/** Flag to track if info file is displayed. */
	bool_t m_info_displayed;
	/** Info offset as index of first visible line. */
	int m_info_offset;
	/** Total number of lines of current displayed info file. */
	int m_info_lines;
	/** Array of offsets of info file lines from the beginning of file. */
	long *m_info_lines_offset;
	/** Info file pointer. */
	FILE *m_info_file;
	/** Info file displaying rectangle. */
	vga::text_rect_t m_info_rect;

	/** Path of current thumbnail. */
	char m_thumbnail_path[UI_THUMBNAIL_PATH_SIZE];
	/** Thumbnail loop index. */
	int m_thumbnail_loop;

	/** Encode bitmap for planar VGA modes. */
	bitmap_encoder *m_encoder;
};

#endif
