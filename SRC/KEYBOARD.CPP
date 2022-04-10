#include <mem.h>
#include <stdio.h>

#include "keyboard.hpp"
#include "isr.hpp"

#define BREAK_BIT	0x80

/** Macro to change bits on pressed and locked shifts bit-field. */
#define CHANGE_BIT(test, var, bit) \
			((test) ? ((var) | (bit)) : ((var) & ~(bit)))

#define KC_RSHIFT	(1 << 0)
#define KC_LSHIFT	(1 << 1)
#define KC_CTRL		(1 << 2)
#define KC_ALT		(1 << 3)
#define KC_SCRLLOCK	(1 << 4)
#define KC_NUMLOCK	(1 << 5)
#define KC_CAPSLOCK	(1 << 6)
#define KC_INSLOCK	(1 << 7)

#define KC_LCTRL	(1 << 8)
#define KC_LALT		(1 << 9)
#define KC_SYSREQ	(1 << 10)
#define KC_PAUSE	(1 << 11)
#define KC_SCROLL	(1 << 12)
#define KC_NUM		(1 << 13)
#define KC_CAPS		(1 << 14)
#define KC_INSERT	(1 << 15)

#define KS_LAST_E1	(1 << 0)
#define KS_LAST_E0	(1 << 1)
#define KS_RCTRL	(1 << 2)
#define KS_RALT		(1 << 3)
#define KS_101KEYB	(1 << 4)
#define KS_NUMPAD	(1 << 5)
#define KS_LAST_ID	(1 << 6)
#define KS_READING	(1 << 7)

#define KS_LED_SCRL	(1 << 8)
#define KS_LED_NUM	(1 << 9)
#define KS_LED_CAPS	(1 << 10)
#define KS_RESERVED	(1 << 11)
#define KS_ACK_RX	(1 << 12)
#define KS_RTX_RX	(1 << 13)
#define KS_MODE		(1 << 14)
#define KS_TX_ERR	(1 << 15)

// Define DEBUG_KEYB on compiler option to enable debugging (for TASM also).

isr::isrptr_t keyboard::old_isr = NULL;
#ifdef DEBUG_KEYB
volatile uint8_t keys_pressed[0x80];
#endif
volatile uint8_t key_make_break = 0;
static volatile uint8_t old_key_make_break = 0;
static volatile uint16_t far *keyb_control = (uint16_t far *)0x00400017L;
static volatile uint16_t far *keyb_state = (uint16_t far *)0x00400096L;
static volatile uint16_t pressed_shifts = 0, locked_shifts = 0;

bool_t keyboard::init() {
#ifdef DEBUG_KEYB
	memset((void *)keys_pressed, 0x01, 0x80 * sizeof(uint8_t));
#endif
	keyboard::old_isr = isr::set(0x09, keyboard::isr_handler);

	return TRUE;
}

bool_t keyboard::release() {
	if (keyboard::old_isr == NULL)
		return FALSE;

	isr::set(0x09, keyboard::old_isr);
	return TRUE;
}

bool_t keyboard::read(keyboard::read_data_t *data) {
	if (keyboard::is_key_make_avail() == TRUE) {
		keyboard::read_make_break(data);
		return TRUE;
	}

	if (keyboard::is_key_avail() == TRUE) {
		keyboard::read_press(data);
		return TRUE;
	}

	if (keyboard::is_key_break_avail() == TRUE) {
		keyboard::read_make_break(data);
		return TRUE;
	}

	return FALSE;
}

bool_t keyboard::is_key_make_avail() {
	if (old_key_make_break == key_make_break ||
		(key_make_break & BREAK_BIT))
		return FALSE;

	return TRUE;
}

bool_t keyboard::is_key_break_avail() {
	if (old_key_make_break == key_make_break ||
		!(key_make_break & BREAK_BIT))
		return FALSE;

	return TRUE;
}

void keyboard::read_make_break(keyboard::read_data_t *data) {
	while (old_key_make_break == key_make_break);

	bool_t is_make = (key_make_break & BREAK_BIT) ? FALSE : TRUE;

	data->ascii = !(key_make_break & BREAK_BIT);
	data->scan_code = key_make_break & ~BREAK_BIT;

	// Update shifts state based on break bit to have consistent results
	// on data->virt_key.
	if (is_make) {
		keyboard::update_shifts_state();
		keyboard::normalize(data);
		data->type = keyboard::KEV_MAKE;
	} else {
		keyboard::normalize(data);
		keyboard::update_shifts_state();
		data->type = keyboard::KEV_BREAK;
	}

	old_key_make_break = key_make_break;
}

bool_t keyboard::is_key_avail() {
	asm {
		mov		ah, 0x01
		int		0x16
		lahf
		and		ah, 0x40
		jz		new_key
	}

	return FALSE;

new_key:
	// Ensure make/break available flag is got before standard key flag.
	if (keyboard::is_key_make_avail() == TRUE)
		return FALSE;

	return TRUE;
}

void keyboard::read_press(keyboard::read_data_t *data) {
	uint8_t scan, ascii;
	asm {
		mov		ah, 0x00
		int		0x16
		mov		ascii, al
		mov		scan, ah
	}

	data->ascii = ascii;
	data->scan_code = scan;

	keyboard::normalize(data);
	data->type = keyboard::KEV_PRESS;
}

void keyboard::normalize(keyboard::read_data_t *data) {
	uint8_t code = data->scan_code;

	if (code >= (0x3b + 0x19) && code <= (0x44 + 0x19))
		code -= 0x19;
	else if (code >= (0x3b + 0x23) && code <= (0x44 + 0x23))
		code -= 0x23;
	else if (code >= (0x3b + 0x2d) && code <= (0x44 + 0x2d))
		code -= 0x2d;
	else if (code >= (0x02 + 0x76) && code <= (0x0d + 0x76))
		code -= 0x76;
	else if (code == (0x47 + 0x30))
		code -= 0x30;
	else if (code == (0x49 + 0x3b))
		code -= 0x3b;
	else if (code == (0x4b + 0x28))
		code -= 0x28;
	else if (code == (0x4d + 0x27))
		code -= 0x27;
	else if (code == (0x4f + 0x26))
		code -= 0x26;
	else if (code == (0x51 + 0x25))
		code -= 0x25;
	else if (code >= 0x47 && code <= 0x53) {
		if (data->ascii >= '0' && data->ascii <= '9')
			code = VK_NUM0 + data->ascii - '0';
		else if (data->ascii == '.')
			code = VK_NUMDOT;
		else if (data->ascii == '-')
			code = VK_NUMSUB;
		else if (data->ascii == '+')
			code = VK_NUMADD;
	} else if (code == 0x35 && data->ascii == '/')
		code = VK_NUMDIV;
	else if (code == 0x37 && data->ascii == '*')
		code = VK_NUMMUL;
	else if (code == 0x2a)
		code = VK_LSHIFT;
	else if (code == 0x36)
		code = VK_RSHIFT;
	else if (code == 0x1d) {
		if (!(pressed_shifts & VS_RCTRL))
			code = VK_LCTRL;
		else
			code = VK_RCTRL;
	} else if (code == 0x38) {
		if (!(pressed_shifts & VS_RALT))
			code = VK_LALT;
		else
			code = VK_RALT;
	}

	data->virt_key = code;

	data->shifts = pressed_shifts;
	data->locked_shifts = locked_shifts;
}

void keyboard::update_shifts_state() {
	uint16_t kc = *keyb_control;
	uint16_t ks = *keyb_state;

	uint16_t lctrl = kc & KC_LCTRL, rctrl = ks & KS_RCTRL;
	uint16_t lalt = kc & KC_LALT, ralt = ks & KS_RALT;

	pressed_shifts = CHANGE_BIT(kc & KC_LSHIFT, pressed_shifts, VS_LSHIFT);
	pressed_shifts = CHANGE_BIT(kc & KC_RSHIFT, pressed_shifts, VS_RSHIFT);
	pressed_shifts = CHANGE_BIT(lctrl && !rctrl, pressed_shifts, VS_LCTRL);
	pressed_shifts = CHANGE_BIT(lalt && !ralt, pressed_shifts, VS_LALT);
	pressed_shifts = CHANGE_BIT(rctrl && lctrl, pressed_shifts, VS_RCTRL);
	pressed_shifts = CHANGE_BIT(ralt && lalt, pressed_shifts, VS_RALT);
	pressed_shifts = CHANGE_BIT(kc & KC_PAUSE, pressed_shifts, VS_PAUSE);
	pressed_shifts = CHANGE_BIT(kc & KC_SCROLL, pressed_shifts, VS_SCRLLOCK);
	pressed_shifts = CHANGE_BIT(kc & KC_NUM, pressed_shifts, VS_NUMLOCK);
	pressed_shifts = CHANGE_BIT(kc & KC_CAPS, pressed_shifts, VS_CAPSLOCK);
	pressed_shifts = CHANGE_BIT(kc & KC_INSERT, pressed_shifts, VS_INSERT);

	locked_shifts = CHANGE_BIT(kc & KC_SCRLLOCK, locked_shifts, VS_SCRLLOCK);
	locked_shifts = CHANGE_BIT(kc & KC_NUMLOCK, locked_shifts, VS_NUMLOCK);
	locked_shifts = CHANGE_BIT(kc & KC_CAPSLOCK, locked_shifts, VS_CAPSLOCK);
	locked_shifts = CHANGE_BIT(kc & KC_INSLOCK, locked_shifts, VS_INSERT);
}

/** Map to convert virtual key codes (the array indices) into ASCII
 * characters (the array values).
 *
 * @note This map is an array so gaps on virtual key codes must be
 *       filled with placeholder NULL characters.
 */
static char vk_to_char_map[] = {
/* <NONE>	0x00 */ '\0',

/* VK_ESC	0x01 */ '\0',
/* VK_1		0x02 */ '1',
/* VK_2		0x03 */ '2',
/* VK_3		0x04 */ '3',
/* VK_4		0x05 */ '4',
/* VK_5		0x06 */ '5',
/* VK_6		0x07 */ '6',
/* VK_7		0x08 */ '7',
/* VK_8		0x09 */ '8',
/* VK_9		0x0a */ '9',
/* VK_0		0x0b */ '0',

/* <NONE>	0x0c */ '\0',
/* <NONE>	0x0d */ '\0',

/* VK_BKSPC	0x0e */ '\0',

/* VK_TAB	0x0f */ '\t',
/* VK_Q		0x10 */ 'Q',
/* VK_W		0x11 */ 'W',
/* VK_E		0x12 */ 'E',
/* VK_R		0x13 */ 'R',
/* VK_T		0x14 */ 'T',
/* VK_Y		0x15 */ 'Y',
/* VK_U		0x16 */ 'U',
/* VK_I		0x17 */ 'I',
/* VK_O		0x18 */ 'O',
/* VK_P		0x19 */ 'P',

/* <NONE>	0x1a */ '\0',
/* <NONE>	0x1b */ '\0',

/* VK_ENTER	0x1c */ '\n',

/* <NONE>	0x1d */ '\0',

/* VK_A		0x1e */ 'A',
/* VK_S		0x1f */ 'S',
/* VK_D		0x20 */ 'D',
/* VK_F		0x21 */ 'F',
/* VK_G		0x22 */ 'G',
/* VK_H		0x23 */ 'H',
/* VK_J		0x24 */ 'J',
/* VK_K		0x25 */ 'K',
/* VK_L		0x26 */ 'L',

/* <NONE>	0x27 */ '\0',
/* <NONE>	0x28 */ '\0',
/* <NONE>	0x29 */ '\0',
/* <NONE>	0x2a */ '\0',
/* <NONE>	0x2b */ '\0',

/* VK_Z		0x2c */ 'Z',
/* VK_X		0x2d */ 'X',
/* VK_C		0x2e */ 'C',
/* VK_V		0x2f */ 'V',
/* VK_B		0x30 */ 'B',
/* VK_N		0x31 */ 'N',
/* VK_M		0x32 */ 'M',

/* <NONE>	0x33 */ '\0',
/* <NONE>	0x34 */ '\0',
/* <NONE>	0x35 */ '\0',
/* <NONE>	0x36 */ '\0',
/* <NONE>	0x37 */ '\0',
/* <NONE>	0x38 */ '\0',

/* VK_SPACE	0x39 */ ' '
};

char keyboard::virtual_key_to_char(uint8_t virt_key) {
	if (virt_key > VK_SPACE)
		return '\0';

	return vk_to_char_map[virt_key];
}
