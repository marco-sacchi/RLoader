#ifndef THEME_HPP
#define THEME_HPP

#include "types.hpp"

class uitheme {
public:
	struct button {
		static uint8_t fore;
		static uint8_t back;
	};

	struct editbox {
		static uint8_t fore;
		static uint8_t back;
		static uint8_t selected_fore;
		static uint8_t selected_back;
	};

	struct label {
		static uint8_t fore;
	};

	struct listbox {
		static uint8_t fore;
		static uint8_t back;
		static uint8_t selected_fore;
		static uint8_t selected_back;
	};

	struct scroll_bar {
		static uint8_t fore;
		static uint8_t back;
	};

	struct window {
		static uint8_t fore;
		static uint8_t back;
		static uint8_t caption_fore;
		static uint8_t caption_back;
	};

	static void init(bool_t text_mode);
};

#endif
