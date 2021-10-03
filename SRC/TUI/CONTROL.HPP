#ifndef TUI_CONTROL_HPP
#define TUI_CONTROL_HPP

#include "types.hpp"
#include "vga.hpp"

#include "tui/tui.hpp"

/** Control base class. */
class uicontrol {
public:
	/** Control constructor.
	 *
	 * @param id Control unique identifier.
	 */
	uicontrol(uint16_t id);
	/** Control destructor.
	 *
	 * @note This destructor also deallocate resources of all
	 *       descendant controls.
	 */
	virtual ~uicontrol();

	/** Get control unique identifier.
	 *
	 * @return Control unique identifier.
	 */
	uint16_t get_id() { return m_id; }

	/** Get control accelerator character triggered by ALT+[char].
	 *
	 * @return The uppercase accelerator ASCII character.
	 */
	char get_accelerator() { return m_accelerator; }
	/** Set control accelerator character.
	 *
	 * @see get_accelerator
	 * @param accelerator Set the accelerator character as uppercase ASCII
	 *                    character.
	 */
	void set_accelerator(char accelerator);

	/** Set control text/foreground color.
	 *
	 * @param color Color index.
	 */
	void set_fore_color(uint8_t color);
	/** Set control background color.
	 *
	 * @param color Color index.
	 */
	void set_back_color(uint8_t color);

	/** Check if control can got focus.
	 *
	 * @return TRUE if control can got focus, FALSE otherwise.
	 */
	bool_t is_focusable() { return m_focusable; }

	/** Get parent control.
	 *
	 * @return Parent control pointer, NULL if not defined.
	 */
	uicontrol *get_parent() { return m_parent; }
	/** Get next sibling control.
	 *
	 * @return Next control, NULL if not defined.
	 */
	uicontrol *get_next() { return m_next; }
	/** Get previous sibling control.
	 *
	 * @return Previous control, NULL if not defined.
	 */
	uicontrol *get_previous() { return m_previous; }
	/** Get first children control.
	 *
	 * @return First children control, NULL if not defined.
	 */
	uicontrol *get_first_child() { return m_first_child; }
	/** Get last children control.
	 *
	 * @return Last children control, NULL if not defined.
	 */
	uicontrol *get_last_child();

	/** Set next control updating internal double linked list.
	 *
	 * @param ctrl Control to insert after this one.
	 */
	void set_next(uicontrol *ctrl);
	/** Set previous control updating internal double linked list.
	 *
	 * @param ctrl Control to insert before this one.
	 */
	void set_previous(uicontrol *ctrl);

	/** Append control to the children list.
	 *
	 * @param ctrl Control to append.
	 * @return TRUE on success, FALSE if control is already children
	 *         of this one.
	 */
	bool_t add_control(uicontrol *ctrl);
	/** Remove control to the children list.
	 *
	 * @param ctrl Control to remove.
	 * @return TRUE on success, FALSE if control is not children of this one.
	 */
	bool_t remove_control(uicontrol *ctrl);
	/** Check if control is children of this one.
	 *
	 * @param ctrl Control to test.
	 * @return TRUE if control is children of this one, FALSE otherwise.
	 */
	bool_t is_child_control(uicontrol *ctrl);

	/** Called when control got focus. */
	virtual void got_focus();
	/** Called when control lose focus. */
	virtual void lose_focus();
	/** Fully repaint the control and all descendants.
	 *
	 * @note uicontrol::repaint must be called on overloaded method.
	 */
	virtual void repaint();

	/** Called when accelerator for this control is triggered.
	 *
	 * @param action Filled with action data to return to main UI task.
	 */
	virtual void on_accelerator(tui::action_t *action) { }
	/** Called when the control receives a message.
	 *
	 * @param msg Received message.
	 * @param action Filled with action data to return to main UI task.
	 */
	virtual void on_message(tui::message_t *msg, tui::action_t *action) { }

protected:
	/** Compute parent control rectangle in absolute coordinate system.
	 *
	 * @param rect Filled with computed rectangle.
	 */
	void get_parent_rect(vga::text_rect_t *rect);
	/** Compute this control paint rectangle in absolute coordinate system.
	 *
	 * @param rect Filled with computed rectangle.
	 */
	void get_paint_rect(vga::text_rect_t *rect);

	/** Unique identifier. */
	uint16_t m_id;
	/** Accelerator as uppercase ASCII character. */
	char m_accelerator;
	/** Control drawing area in local coordinate respect to the parent. */
	vga::text_rect_t m_rect;
	/** Focused flag. */
	bool_t m_focused;
	/** Flag to track if this control is focusable. */
	bool_t m_focusable;

	/** Control foreground and background colors paint attributes. */
	uint8_t m_paint_attrs;
	/** Control blank characters attributes. */
	uint8_t m_blank_attrs;

private:
	/** Parent control. */
	uicontrol *m_parent;
	/** Next sibling control. */
	uicontrol *m_next;
	/** Parent sibling control. */
	uicontrol *m_previous;
	/** First children control. */
	uicontrol *m_first_child;
};

#endif