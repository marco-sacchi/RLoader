#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "types.hpp"
#include "isr.hpp"

/** Virtual shifts bits definitions. */

#define VS_LSHIFT	(1 << 0)
#define VS_RSHIFT	(1 << 1)
#define VS_SHIFT	(VS_RSHIFT | VS_LSHIFT)
#define VS_LCTRL	(1 << 2)
#define VS_RCTRL	(1 << 3)
#define VS_CTRL		(VS_RCTRL | VS_LCTRL)
#define VS_LALT		(1 << 4)
#define VS_RALT		(1 << 5)
#define VS_ALT		(VS_RALT | VS_LALT)
#define VS_META		(1 << 6)
#define VS_MENU		(1 << 7)
#define VS_PAUSE	(1 << 8)
#define VS_SCRLLOCK	(1 << 9)
#define VS_NUMLOCK	(1 << 10)
#define VS_CAPSLOCK	(1 << 11)
#define VS_INSERT	(1 << 12)

/** Virual keys definitions.
 *
 * @note If the virtual key codes are modified it si necessary to modify
 *       also the map in the cpp file (@see vk_to_char_map).
 */

#define VK_ESC		0x01
#define VK_1		0x02
#define VK_2		0x03
#define VK_3		0x04
#define VK_4		0x05
#define VK_5		0x06
#define VK_6		0x07
#define VK_7		0x08
#define VK_8		0x09
#define VK_9		0x0a
#define VK_0		0x0b
#define VK_BKSPC	0x0e

#define VK_TAB		0x0f
#define VK_Q		0x10
#define VK_W		0x11
#define VK_E		0x12
#define VK_R		0x13
#define VK_T		0x14
#define VK_Y		0x15
#define VK_U		0x16
#define VK_I		0x17
#define VK_O		0x18
#define VK_P		0x19
#define VK_ENTER	0x1c

#define VK_A		0x1e
#define VK_S		0x1f
#define VK_D		0x20
#define VK_F		0x21
#define VK_G		0x22
#define VK_H		0x23
#define VK_J		0x24
#define VK_K		0x25
#define VK_L		0x26

#define VK_Z		0x2c
#define VK_X		0x2d
#define VK_C		0x2e
#define VK_V		0x2f
#define VK_B		0x30
#define VK_N		0x31
#define VK_M		0x32

#define VK_SPACE	0x39

#define VK_F1		0x3b
#define VK_F2		0x3c
#define VK_F3		0x3d
#define VK_F4		0x3e
#define VK_F5		0x3f
#define VK_F6		0x40
#define VK_F7		0x41
#define VK_F8		0x42
#define VK_F9		0x43
#define VK_F10		0x44
#define VK_F11		0x45
#define VK_F12		0x46

#define VK_HOME		0x47
#define VK_UP		0x48
#define VK_PGUP		0x49
#define VK_LEFT     0x4b
#define VK_RIGHT	0x4d
#define VK_END		0x4f
#define VK_DOWN		0x50
#define VK_PGDN		0x51
#define VK_INS		0x52
#define VK_DEL		0x53

// From here virtual key codes do not matches scan codes.

#define VK_NUM0		0xb0
#define VK_NUM1		0xb1
#define VK_NUM2		0xb2
#define VK_NUM3		0xb3
#define VK_NUM4		0xb4
#define VK_NUM5		0xb5
#define VK_NUM6		0xb6
#define VK_NUM7		0xb7
#define VK_NUM8		0xb8
#define VK_NUM9		0xb9
#define VK_NUMDOT	0xba
#define VK_NUMDIV	0xbb
#define VK_NUMMUL	0xbc
#define VK_NUMSUB	0xbd
#define VK_NUMADD	0xbe

#define VK_LSHIFT	0xd0
#define VK_RSHIFT	0xd1
#define VK_LCTRL	0xd2
#define VK_RCTRL	0xd3
#define VK_LALT		0xd4
#define VK_RALT		0xd5
#define VK_META		0xd6
#define VK_MENU		0xd7
#define VK_SCRLLOCK	0xd8
#define VK_NUMLOCK	0xd9
#define VK_CAPSLOCK	0xda

#pragma pack(push, 1);

/** Keyboard handling. */
class keyboard {
public:
	/** Keyboard events. */
	enum key_events {
		/** Key press. */
		KEV_MAKE = 1,
		/** Key typematic repeat. */
		KEV_PRESS,
		/** Key depress. */
		KEV_BREAK
	};

	/** Structure filled with data read from keyboard. */
	typedef struct {
		/** Event type. */
		key_events type;
		/** ASCII value. */
		uint8_t ascii;
		/** Raw scan code. */
		uint8_t scan_code;
		/** Virtual key code (@see VK_* defines). */
		uint8_t virt_key;
		/** Virtual shifts pressed states (@see VS_* defines). */
		uint16_t shifts;
		/** Virtual shifts locked states (@see VS_* defines). */
		uint16_t locked_shifts;
	} read_data_t;

	/** Initialize keyboard handling chaining old ISR with new one.
	 *
	 * @return TRUE on success, FALSE otherwise.
	 */
	static bool_t init();
	/** Release keyboard resources restoring old ISR.
	 *
	 * @return TRUE on success, FALSE otherwise.
	 */
	static bool_t release();

	/** Read an event from the keyboard.
	 *
	 * @param data Filled with info about keyboard event.
	 * @return TRUE if an event is ready, FALSE otherwise.
	 */
	static bool_t read(read_data_t *data);

	/** Check if a key has changed its state from not pressed to pressed.
	 *
	 * @return TRUE if a key has changed its state to pressed, FALSE otherwise.
	 */
	static bool_t is_key_make_avail();
	/** Check if a key has changed its state from pressed to not pressed.
	 *
	 * @return TRUE if a key has changed its state to not pressed,
	 *         FALSE otherwise.
	 */
	static bool_t is_key_break_avail();
	/** Read data about key not pressed/pressed state change.
	 *
	 * @param data Filled with info about key that has changed state.
	 */
	static void read_make_break(read_data_t *data);

	/** Check if key is available to be read (pressed or typematic repeat).
	 *
	 * @return TRUE if key is available, FALSE otherwise.
	 */
	static bool_t is_key_avail();
	/** Read data about key pressed/repeated.
	 *
	 * @param data Filled with info about key that has pressed/repeated.
	 */
	static void read_press(read_data_t *data);

	/** Convert virtual key code to uppercase ascii character.
	 *
	 * @return Uppercase ascii character, zero if not virtual key is not
	 *         representable as a non-extended ascii character.
	 */
	static char virtual_key_to_char(uint8_t virt_key);

private:
	/** Normalize read data adding virtual keys values. */
	static void normalize(read_data_t *data);
	/** Update bits of keyboard shift keys states. */
	static void update_shifts_state();

	/** ISR routine that read data from the keyboard and call old isr. */
	static void interrupt isr_handler(__CPPARGS);
};

#pragma pack(pop);

#endif
