#include <mem.h>

#include "tui/control.hpp"
#include "tui/view.hpp"

#include "math.hpp"

uicontrol::uicontrol(uint16_t id) {
	m_id = id;
	m_focused = FALSE;
	m_focusable = TRUE;
	m_accelerator = NULL;
	memset(&m_rect, 0x00, sizeof(vga::text_rect_t));
	m_parent = m_previous = m_next = m_first_child = NULL;
	m_view = NULL;
}

uicontrol::~uicontrol() {
	// Unlink the control from siblings double linked list.
	if (m_previous != NULL)
		m_previous->m_next = m_next;

	if (m_next != NULL)
		m_next->m_previous = m_previous;

	// Deallocate all children controls.
	if (m_first_child != NULL) {
		uicontrol *ctrl = m_first_child, *next;
		while (ctrl != NULL) {
			next = ctrl->m_next;
			delete ctrl;
			ctrl = next;
		}
	}

	m_parent = m_next = m_previous = m_first_child = NULL;
	m_view = NULL;
	m_accelerator = NULL;
	memset(&m_rect, 0x00, sizeof(vga::text_rect_t));
}

void uicontrol::set_accelerator(char accelerator) {
	m_accelerator = accelerator;
}

uint8_t uicontrol::get_fore_color() {
	uint8_t back_color, fore_color, blink;
	vga::decode_attrs(m_paint_attrs, &back_color, &fore_color, &blink);
	return fore_color;
}

void uicontrol::set_fore_color(uint8_t color) {
	uint8_t back_color, fore_color, blink;
	vga::decode_attrs(m_paint_attrs, &back_color, &fore_color, &blink);
	m_paint_attrs = vga::encode_attrs(back_color, color, 0);

	this->repaint();
}

uint8_t uicontrol::get_back_color() {
	uint8_t back_color, fore_color, blink;
	vga::decode_attrs(m_paint_attrs, &back_color, &fore_color, &blink);
	return back_color;
}

void uicontrol::set_back_color(uint8_t color) {
	uint8_t back_color, fore_color, blink;
	vga::decode_attrs(m_paint_attrs, &back_color, &fore_color, &blink);
	m_paint_attrs = vga::encode_attrs(color, fore_color, 0);
	m_blank_attrs = vga::encode_attrs(color, color, 0);

	this->repaint();
}

bool_t uicontrol::get_transparency() {
	return m_transparent;
}

void uicontrol::set_transparency(bool_t enabled) {
	m_transparent = enabled;
}

void uicontrol::update_transparency_attrs() {
	if (m_transparent == FALSE)
		return;

	uicontrol *control = this;
	do {
		control = control->m_parent;
	} while (control != NULL && control->m_transparent == TRUE);

	if (control == NULL)
		return;

	uint8_t back_color, fore_color, blink;
	vga::decode_attrs(m_paint_attrs, &back_color, &fore_color, &blink);

	back_color = control->get_back_color();

	m_paint_attrs = vga::encode_attrs(back_color, fore_color, 0);
	m_blank_attrs = vga::encode_attrs(back_color, back_color, 0);
}

uicontrol *uicontrol::get_last_child() {
	uicontrol *children = m_first_child;
	while (children->m_next != NULL)
		children = children->m_next;

	return children;
}

void uicontrol::set_next(uicontrol *ctrl) {
	if (ctrl != NULL)
		ctrl->m_next = this->m_next;

	this->m_next = ctrl;
}

void uicontrol::set_previous(uicontrol *ctrl) {
	if (ctrl != NULL)
		ctrl->m_previous = this->m_previous;

	this->m_previous = ctrl;
}

bool_t uicontrol::add_control(uicontrol *ctrl) {
	if (m_first_child == NULL) {
		ctrl->m_parent = this;
		ctrl->m_previous = NULL;
		ctrl->m_next = NULL;
		m_first_child = ctrl;

		ctrl->repaint();
		return TRUE;
	}

	if (m_first_child == ctrl)
		return FALSE;

	uicontrol *children = m_first_child;
	while (children->m_next != NULL) {
		if (children->m_next == ctrl)
			return FALSE;

		children = children->m_next;
	}

	ctrl->m_parent = this;
	ctrl->m_previous = children;
	ctrl->m_next = NULL;
	children->m_next = ctrl;

	ctrl->repaint();
	return TRUE;
}

bool_t uicontrol::remove_control(uicontrol *ctrl) {
	if (m_first_child == ctrl) {
		m_first_child = m_first_child->m_next;
		m_first_child->m_previous = NULL;

		ctrl->m_parent = NULL;
		ctrl->m_next = ctrl->m_previous = NULL;
		return TRUE;
	}

	uicontrol *children = m_first_child;
	while (children->m_next != NULL) {
		if (children->m_next == ctrl) {
			children->m_next = children->m_next->m_next;
			children->m_next->m_previous = children;

			ctrl->m_parent = NULL;
			ctrl->m_next = ctrl->m_previous = NULL;
			return TRUE;
		}
	}

	return FALSE;
}

bool_t uicontrol::is_child_control(uicontrol *ctrl) {
	uicontrol *children = this->m_first_child;
	do {
		if (children == ctrl)
			return TRUE;
	} while ((children = children->m_next) != NULL);

	return FALSE;
}

void uicontrol::got_focus() {
	m_focused = TRUE;
	this->repaint();
}

void uicontrol::lose_focus() {
	m_focused = FALSE;
	this->repaint();
}

void uicontrol::get_parent_rect(vga::text_rect_t *rect) {
	if (m_parent == NULL) {
		const vga::state_t *state = vga::get_current_state();
		rect->top = rect->left = 0;
		rect->right = state->columns - 1;
		rect->bottom = state->rows - 1;
		return;
	}

	uicontrol *parent = m_parent;
	int height = parent->m_rect.bottom - parent->m_rect.top + 1;
	int width = parent->m_rect.right - parent->m_rect.left + 1;

	memcpy(rect, &(m_parent->m_rect), sizeof(vga::text_rect_t));

	while (parent->m_parent != NULL) {
		parent = parent->m_parent;

		rect->top += parent->m_rect.top;
		rect->left += parent->m_rect.left;
	}

	rect->right = rect->left + width - 1;
	rect->bottom = rect->top + height - 1;
}

bool_t uicontrol::can_repaint() {
	uicontrol *control = this;
	while (control->m_parent != NULL)
		control = control->m_parent;

	if (control->m_view != NULL &&
		control->m_view->get_state() == uiview::STATE_OPENED)
		return TRUE;
	else
		return FALSE;
}

void uicontrol::get_paint_rect(vga::text_rect_t *rect) {
	vga::text_rect_t parent;
	this->get_parent_rect(&parent);

	rect->top = parent.top + m_rect.top;
	rect->left = parent.left + m_rect.left;

	rect->right = min(parent.right, (uint8_t)(parent.left + m_rect.right));
	rect->bottom = min(parent.bottom, (uint8_t)(parent.top + m_rect.bottom));
}

void uicontrol::repaint() {
	if (this->can_repaint() == FALSE)
		return;

	uicontrol *children = m_first_child;
	while (children != NULL) {
		children->repaint();
		children = children->m_next;
	}
}
