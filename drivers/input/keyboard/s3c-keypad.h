/*
 *	
 * $Id: s3c-keypad.h,v 1.5 2008/05/23 06:22:53 dark0351 Exp $
 */

#ifndef _S3C_KEYPAD_H_
#define _S3C_KEYPAD_H_

#define KEYPAD_COLUMNS	5
#define KEYPAD_ROWS	6
#define MAX_KEYPAD_NR	26	/* 4*6 */
#define MAX_KEYMASK_NR	26	
#define KEYPAD_DELAY	80

int keypad_keycode[] = {
	KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6,
	KEY_7, KEY_8, KEY_9, KEY_F10, KEY_0, KEY_F11,
	KEY_END, KEY_SEND, KEY_ENTER, KEY_LEFT, KEY_RIGHT, KEY_UP, 
	KEY_BACKSPACE, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_DOWN,
	KEY_VOLUMEDOWN, KEY_VOLUMEUP
};

struct s3c_keypad {
	struct input_dev *dev;
	int nr_rows;	
	int no_cols;
	int total_keys; 
	int keycodes[MAX_KEYPAD_NR];

	struct work_struct work;
};

typedef enum {
	KEY_PRESS_STATE = 0,
	KEY_RELEASE_STATE,
} key_press_state;

#endif				/* _S3C_KEYIF_H_ */
