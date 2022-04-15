#include "tui/theme.hpp"

uint8_t uitheme::button::fore = 0x0b;
uint8_t uitheme::button::back = 0x03;

uint8_t uitheme::editbox::fore = 0x00;
uint8_t uitheme::editbox::back = 0x07;
uint8_t uitheme::editbox::selected_fore = 0x0b;
uint8_t uitheme::editbox::selected_back = 0x03;

uint8_t uitheme::label::fore = 0x07;

uint8_t uitheme::listbox::fore = 0x00;
uint8_t uitheme::listbox::back = 0x07;
uint8_t uitheme::listbox::selected_fore = 0x0b;
uint8_t uitheme::listbox::selected_back = 0x03;

uint8_t uitheme::scroll_bar::fore = 0x07;
uint8_t uitheme::scroll_bar::back = 0x00;

uint8_t uitheme::window::fore = 0x07;
uint8_t uitheme::window::back = 0x08;
uint8_t uitheme::window::caption_fore = 0x0f;
uint8_t uitheme::window::caption_back = 0x07;

uint8_t uitheme::shadow::fore = 0x08;
uint8_t uitheme::shadow::back = 0x00;

void uitheme::init(bool_t text_mode) {
	// Already initialized values for text-mode UI.
	if (text_mode == TRUE)
		return;

	// 1-bit per pixel theme for graphics UI.
	uint8_t back = 0x00, fore = 0x07;
	uitheme::button::fore = fore;
	uitheme::button::back = back;

	uitheme::editbox::fore = fore;
	uitheme::editbox::back = back;
	uitheme::editbox::selected_fore = back;
	uitheme::editbox::selected_back = fore;

	uitheme::label::fore = fore;

	uitheme::listbox::fore = fore;
	uitheme::listbox::back = back;
	uitheme::listbox::selected_fore = back;
	uitheme::listbox::selected_back = fore;

	uitheme::scroll_bar::fore = fore;
	uitheme::scroll_bar::back = back;

	uitheme::window::fore = fore;
	uitheme::window::back = back;
	uitheme::window::caption_fore = back;
	uitheme::window::caption_back = fore;
}
