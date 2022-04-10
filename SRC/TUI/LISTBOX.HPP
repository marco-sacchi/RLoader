#ifndef TUI_LISTBOX_HPP
#define TUI_LISTBOX_HPP

#include "types.hpp"

#include "tui/control.hpp"

/** Maximum number of columns in which items are arranged. */
#define LISTBOX_MAX_COLS	10

/** Listbox control.
 */
class uilistbox : public uicontrol {
public:
	/** Items sorting modes. */
	enum sorting {
		/** No sorting. */
		SORT_NONE,
		/** Ascending sorting. */
		SORT_ASCENDING,
		/** Descending sorting. */
		SORT_DESCENDING
	};

	/** Listbox constructor.
	 *
	 * @param id Control unique identifier.
	 * @param top Zero-based vertical position in characters.
	 * @param left Zero-based horizontal position in characters.
	 * @param width List width in characters, scrollbar included.
	 * @param height List height in characters.
	 */
	uilistbox(uint16_t id, int top, int left, int width, int height);
	~uilistbox();

	/** Remove all items. */
	void clear();

	/** Add an item to the list.
	 *
	 * @param text Item text.
	 * @return True on success, false otherwise.
	 */
	bool_t add_item(const char *text);

	/** Select specified item by index.
	 *
	 * @param index Item zero-based index.
	 * @return True on item selection success, false otherwise.
	 */
	bool_t set_selected_item(int index);

	/** Select specified item by text.
	 *
	 * @param text Text to search for.
	 * @return True on item selection success, false otherwise.
	 */
	bool_t set_selected_item(const char *text);

	/** Get selected item index.
	 *
	 * @return Index of selected item, -1 if no item is selected.
	 */
	int get_selected_item();

	/** Get selected item text.
	 *
	 * @return Text of selected item, NULL if no item is selected.
	 */
	const char *get_selected_item_text();

	/** Get current number of columns in which items are arranged.
	 *
	 * @return Number of columns.
	 */
	int get_columns() { return m_columns; }

	/** Set number of columns in which items are arranged.
	 *
	 * @param columns Number of columns in range [1, LISTBOX_MAX_COLS].
	 */
	void set_columns(int columns);

	/** Get current sorting mode.
	 *
	 * @return Current sorting mode.
	 */
	sorting get_sort_mode();

	/** Set sorting mode.
	 *
	 * @param mode Sorting mode.
	 */
	void set_sort_mode(sorting mode);

	/** Set highlight string.
	 *
	 * @param string Text to be highlighted in all items.
	 */
	void set_highlight(const char *string);

	virtual void repaint();

	virtual void on_message(uiqueue::message_t *msg, tui::action_t *action);

protected:
	/** Find first item beginning with search string.
	 *
	 * @param search String to search for.
	 * @return First found item index, -1 if no match is found.
	 */
	int find_first_item(char *search);

	/** Repaint item.
	 *
	 * @param index Index of item to be repainted.
	 * @param rect Repaint rectangle.
	 */
	virtual void repaint_item(int index, vga::text_rect_t *rect);

	/** List offset as index of first visible item. */
	int m_offset;
	/** Index of selected list item. */
	int m_selected_item;
	/** List items count. */
	int m_items_count;

	/** Capacity change step when list items will be added/removed. */
	int m_items_capacity_step;
	/** Internal list capacity. */
	int m_items_capacity;

	/** Number of columns in which items are arranged, default 1. */
	int m_columns;
	/** Horizontal offset in local coordinates from control left edge. */
	int m_columns_offsets[LISTBOX_MAX_COLS + 1];

	/** Items sorting mode. */
	sorting m_sort_mode;

	/** Mnemonic search string. */
	char *m_mnemonic_string;
	/** Mnemonic current character pointer into search string. */
	char *m_mnemonic_ptr;
	/** Maximum mnemonic string length. */
	int m_mnemonic_maxlen;

	/** Highlight string. */
	char *m_highlight_string;
	/** Maximum highlight string length. */
	int m_highlight_maxlen;

	/** Array of items string pointers. */
	char **m_items;

private:
	/** Sort items in-place when sorting is not SORT_NONE. */
	void sort_items();

	/** Append character to the end of mnemonic string.
	 *
	 * @param c Character to append.
	 */
	void mnemonic_push(char c);
	/** Remove last character from the mnemonic string. */
	void mnemonic_pop();
	/** Reset mnemonic string to empty string. */
	void mnemonic_reset();

	/** Scroll list by specified offset.
	 *
	 * @param offset Scroll offset, negative value are allowed.
	 * @return True if selected item is changed, false otherwise.
	 */
	bool_t scroll(int offset);
	/** Jump to the beginning of the list.
	 *
	 * @return True if selected item is changed, false otherwise.
	 */
	bool_t jump_to_begin();
	/** Jump to the end of the list.
	 *
	 * @return True if selected item is changed, false otherwise.
	 */
	bool_t jump_to_end();

	/** Calculate list offset.
	 *
	 * @param selected_item Zero-based index of selected item.
	 * @param Zero based index of top item to be displayed.
	 */
	int calc_list_offset(int selected_item);
	/** Calculate list repaint rectangle.
	 *
	 * @param rect Filled with calculated values.
	 */
	void calc_list_rect(vga::text_rect_t *rect);

	/** Repaint scrollbar one column beyond the rectangle right side.
	 *
	 * @param rect Reference rectangle.
	 */
	void repaint_scroll_bar(vga::text_rect_t *rect);

	/** Track last sent action to handle item selecting/selected. */
	tui::action_types m_last_action;
};

#endif
