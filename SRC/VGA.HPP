#ifndef VGA_HPP
#define VGA_HPP

#include "types.hpp"

/** Address/data register. */
#define VGA_ATTR_ADDR	0x3c0
/** Data read register. */
#define VGA_ATTR_READ	0x3c1

/** Palette address source. */
#define VGA_ATTR_PAS	(1<<5)
/** Palette bits 5-4 select. */
#define VGA_ATTR_P54S	(1<<7)
/** 8-bit color enable. */
#define VGA_ATTR_8BIT	(1<<6)
/** Pixel panning mode. */
#define VGA_ATTR_PPM	(1<<5)
/** Blink enable. */
#define VGA_ATTR_BLINK	(1<<3)
/** Line graphics enable. */
#define VGA_ATTR_LGE	(1<<2)
/* Monochrome emulation. */
#define VGA_ATTR_MONO	(1<<1)
/* Attribute controller graphics enable. */
#define VGA_ATTR_ATGE	(1<<0)

/** Sequencer address register. */
#define VGA_SEQ_ADDR	0x3c4
/** Sequencer data register. */
#define VGA_SEQ_DATA	0x3c5

/** DAC state register. */
#define VGA_DAC_STATE	0x3c7
/** DAC read register. */
#define VGA_DAC_READ	0x3c7
/** DAC write address register. */
#define VGA_DAC_WRITE	0x3c8
/** DAC write data register. */
#define VGA_DAC_DATA	0x3c9

/** DAC ready to accept reads. */
#define VGA_DAC_SREAD	0x00
/** DAC ready to accept writes. */
#define VGA_DAC_SWRITE	0x03

/** Graphics address register. */
#define VGA_GFX_ADDR	0x3ce
/** Graphics data register. */
#define VGA_GFX_DATA	0x3cf

/** CRT controller address register. */
#define VGA_CRTC_ADDR	0x3d4
/** CRT controller data register. */
#define VGA_CRTC_DATA	0x3d5

/** Input status #1 register. */
#define VGA_REG_STAT1	0x3da

/** VGA utility methods. */
class vga {

public:
	/** Video mode types. */
	enum vmodes_types {
	  /** Graphics mode. */
	  ALL_POINTS_ADDRESSABLE = 0,
	  /** Text mode. */
	  ALPHANUMERIC = 1
	};

	/** Mode register read modes. */
	enum gfx_rmodes {
		/** Read 1-bit for 8 pixels on plane specified by Read map
		 * select register.
		 */
		PER_PLANE = 0,
		/** Return 0, or 1 if match value loaded on Color compare register,
		 * for 8 pixels on planes selected by Color don't care register.
		 */
		COLOR_COMPARE = 1,
	};

	/** Mode register write modes. */
	enum gfx_wmodes {
		/** Default, direct write. */
		DIRECT = 0,
		/** Used for fast 32-bit memory-to-memory moves. */
		FAST_MOV = 1,
		/** Color plane filled with respective bit on Bit mask register. */
		PLANE_FILL = 2,
		/** Used for write data ANDed using Bit mask as a bit mask. */
		DATA_ROTATE = 3
	};

	/** Write string working modes. */
	enum ws_modes {
		/** Use specified attributes; no cursor position update. */
		USE_ATTRS_NO_UPDATE_CURSOR = 0x00,
		/** Use specified attributes; cursor position update. */
		USE_ATTRS_UPDATE_CURSOR = 0x01,
		/** Use attributes in string; no cursor position update. */
		USE_STR_NO_UPDATE_CURSOR = 0x02,
		/** Use attributes in string; cursor position update. */
		USE_STR_UPDATE_CURSOR = 0x03
	};

	/** Data rotate modes. */
	enum rot_modes {
		/** Data written unmodified. */
		UNMODIFIED = 0x00,
		/** Data ANDed with latched data. */
		AND_WITH_LATCH = 0x01,
		/** Data ORed with latched data. */
		OR_WITH_LATCH = 0x02,
		/** Data XORed with latched data. */
		XOR_WITH_LATCH = 0x03
	};

	/** Video state. */
	typedef struct {
		/** Mode number. */
		uint8_t mode;
		/** Charactes columns. */
		uint8_t columns;
		/** Charactes rows. */
		uint8_t rows;
		/** Current video page. */
		uint8_t current_page;
		/** Mode type (graphics or text). */
		vmodes_types type;
		/** Maximum number of colors. */
		int colors;
		/** True when N colors is N shades of gray. */
		bool_t is_grayscale;
	} state_t;

	/** Text cursor position and size. */
	typedef struct {
		/** Position as zero-based row and column. */
		uint8_t row, column;
		/** Cursor type as start/end line. */
		uint8_t start_line, end_line;
	} cursor_info_t;

	/** Characters grid rectagle. */
	typedef struct {
		/** Zero-based top-left corner in characters cells coordinates. */
		uint8_t top, left;
		/** Zero-based bottom-right corner in characters cells coordinates. */
		uint8_t bottom, right;
	} text_rect_t;

	/** RGB triplet to set color palette entry. */
	typedef uint8_t color_t[3];

	/** Initialize VGA library.
	 *
	 * NOTE: this function must be called before any other.
	 */
	static void init();

	/** Check for monochrome display adapter.
	 *
	 * @return True if on monochrome display adapter, false otherwise.
	 */
	static bool_t is_monochrome();

	/** Set graphics mode.
	 *
	 * @param mode Requested mode.
	 */
	static void set_mode(uint8_t mode);
	/** Read video state.
	 *
	 * @param state Valorized with video state.
	 */
	static void read_state(state_t *state);
	/** Get current video state.
	 *
	 * @return Current state.
	 */
	static const vga::state_t *get_current_state();

	/** Toggle cursor visibility.
	 *
	 * @param enabled TRUE to show cursor, FALSE otherwise.
	 */
	static void enable_cursor(bool_t enabled);

	/** Set the cursor shape/type.
	 *
	 * @param start Start scan line.
	 * @param end End scan line.
	 */
	static void set_cursor_type(uint8_t start, uint8_t end);
	/** Set cursor position.
	 *
	 * @param row Zero-based row in characters cells coordinate.
	 * @param column Zero-based column in characters cells coordinate.
	 */
	static void set_cursor_pos(uint8_t row, uint8_t column);
	/** Get cursor info.
	 *
	 * @param info Valorized with cursor informations.
	 */
	static void get_cursor(cursor_info_t *info);

	/** Encode character attributes.
	 *
	 * @see set_blinking
	 * @param back_color Background color.
	 * @param fore_color Foreground color.
	 * @param blink Blinking text bit.
	 */
	static uint8_t encode_attrs(uint8_t back_color, uint8_t fore_color,
								uint8_t blink);

	/** Scroll rectangle up using BIOS.
	 *
	 * @see encode_attrs
	 * @param rows Number of rows to scroll up.
	 * @param attrs Attributes to use for new rows added at bottom.
	 * @param rect Rectangle to scroll.
	 */
	static void scroll_page_up(uint8_t rows, uint8_t attrs,
							   const text_rect_t *rect);

	/** Scroll rectangle down using BIOS.
	 *
	 * @see encode_attrs
	 * @param rows Number of rows to scroll down.
	 * @param attrs Attributes to use for new rows added at top.
	 * @param rect Rectangle to scroll.
	 */
	static void scroll_page_down(uint8_t rows, uint8_t attrs,
								 const text_rect_t *rect);

	/** Write character to screen at current cursor position using BIOS.
	 *
	 * @see encode_attrs
	 * @param ch Charater.
	 * @param attrs Attributes to use.
	 * @param count Number of characters to write.
	 */
	static void write_char(uint8_t ch, uint8_t attrs, uint16_t count);
	/** Change attributes preserving characters using video memory access.
	 *
	 * @param row Row.
	 * @param col Start column.
	 * @param new_attrs New attributes.
	 * @param count Number of cells for which change attrs.
	 */
	static void write_attrs(uint8_t row, uint8_t col, uint8_t new_attrs,
							uint16_t count);
	/** XOR attributes preserving characters using video memory access.
	 *
	 * @param row Row.
	 * @param col Start column.
	 * @param xor_attrs Value XORed with readed attributes.
	 * @param count Number of cells for which change attrs.
	 */
	static void xor_attrs(uint8_t row, uint8_t col, uint8_t xor_attrs,
						  uint16_t count);
	/** Write string to screen at current corsor position using BIOS.
	 *
	 * @see encode_attrs
	 * @see ws_modes
	 * @param str String to write.
	 * @param attrs Attributes to use for all characters.
	 * @param mode Writing mode.
	 */
	static void write_string(const char *str, uint8_t attrs, ws_modes mode);

	/** Toggle text blink using bit 3 of background color.
	 *
	 * @param blink TRUE to enable blinking, FALSE otherwise.
	 */
	static void set_blinking(bool_t blink);

	/** Set bitplanes access.
	 *
	 * @param linear TRUE for linear access, FALSE for Chain-4 planar mode.
	 * @param graphics TRUE for graphics mode, FALSE for text mode.
	 * @param mem_limit TURE to access > 64 kb of memory, FALSE otherwise.
	 */
	static void set_memory_mode_reg(bool_t linear, bool_t graphics,
									bool_t mem_limit);
	/** Select map to write into.
	 *
	 * @param planes Each bit in range 0-3 enables planes write.
	 */
	static void set_map_mask_reg(uint8_t planes);
	/** Set Enable set/reset register.
	 *
	 * @see set_setreset_reg
	 * @param enable Each bit in range 0-3 enables planes to be modified.
	 */
	static void set_enable_reg(uint8_t enable);
	/** Set Set/reset register.
	 *
	 * @see set_enable_reg
	 * @param setreset Each bit in range 0-3 enables planes to be modified.
	 */
	static void set_setreset_reg(uint8_t setreset);
	/** Data rotate.
	 *
	 * @see rot_modes
	 * @param rotate_count Data written from host to video memory will be
	 *                     rotated to the right the number of bits specified.
	 * @param operation Operation applied between written and latched data.
	 */
	static void set_data_rotate_reg(uint8_t rotate_count, vga::rot_modes operation);
	/** Set map from which read data (if in planar mode).
	 *
	 * @param map_index Zero-based index of map to select.
	 */
	static void set_read_map_reg(uint8_t map_index);
	/** Set masked bits on display memory.
	 *
	 * @param mask Each bit in range 0-7 set to 0 will be masked and remain
	 *             unmodified.
	 */
	static void set_bit_mask(uint8_t mask);

	/** Set graphics read mode.
	 *
	 * @see gfx_rmodes
	 * @param mode New read mode.
	 */
	static void set_read_mode(vga::gfx_rmodes mode);
	/** Set graphics write mode.
	 *
	 * @see gfx_wmodes
	 * @param mode New write mode.
	 */
	static void set_write_mode(vga::gfx_wmodes mode);

	/** Set EGA lookup for which first 16 colors points to lookup 0..15. */
	static void set_ega_colors_lookup();
	/** Set text/EGA color palette at specified index.
	 *
	 * @param index Index of entry to set.
	 * @param rgb Color as rgb triplet.
	 */
	static void set_ega_color(uint8_t index, const vga::color_t rgb);
	/** Set VGA color palette at specified index.
	 *
	 * @param index Index of entry to set.
	 * @param rgb Color as rgb triplet.
	 */
	static void set_color(uint8_t index, const vga::color_t rgb);
};

#endif

