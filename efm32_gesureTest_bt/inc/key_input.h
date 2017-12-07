/*
 * key_input.h
 *
 *  Created on: 2017. 1. 18.
 *      Author: netbugger
 */

#ifndef SRC_KEY_INPUT_H_
#define SRC_KEY_INPUT_H_



void isr_btn2(uint8_t pin);
void isr_btn3(uint8_t pin);
void init_key_input();
uint8_t get_key_value();

#define true 1
#define false 0

#define JOYSTICK_NORMAL	40
#define JOYSTICK_LEFT 	24
#define JOYSTICK_UP		34
#define JOYSTICK_RIGHT	31
#define JOYSTICK_DOWN	20
#define JOYSTICK_PRESS	0

#define KEYBOARD_NONE	0x00
#define KEYBOARD_RETURN	0x28
#define KEYBOARD_ENTER	0x58
#define KEYBOARD_ESC	0x29
#define KEYBOARD_DEL	0x2A
#define KEYBOARD_RIGHT	0x4f
#define KEYBOARD_LEFT	0x50
#define KEYBOARD_DOWN	0x51
#define KEYBOARD_UP		0x52
#define KEYBOARD_TAB	0x2B
#define KEYBOARD_HOME	0x4A
#define KEYBOARD_END	0x4D
#define KEYBOARD_A		0x04
#define KEYBOARD_1		0x1E
#define KEYBOARD_2		0x1F
#define KEYBOARD_3		0x20
#define KEYBOARD_4		0x21
#define KEYBOARD_5		0x22
#define KEYBOARD_6		0x23
#define KEYBOARD_7		0x24
#define KEYBOARD_8		0x25
#define KEYBOARD_9		0x26
#define KEYBOARD_0		0x27

#define SENSOR_VALUE_COUNTER	20	//# per 1 sec
//#define SAMPLING_DELAY			50	//ms
#define AXIS_NUM				6
#define BACKUP_SAMPLE 5
#define SAMPLE_NUM SENSOR_VALUE_COUNTER


#endif /* SRC_KEY_INPUT_H_ */
