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

#define KEYBOARD_7		0x24
#define KEYBOARD_1		0x1E
#define KEYBOARD_2		0x1F
#define KEYBOARD_3		0x20

#define SENSOR_VALUE_COUNTER	20	//# per 1 sec
#define SAMPLING_DELAY			1	//ms
//#define AXIS_NUM				6
#define AXIS_NUM				3
#define BACKUP_SAMPLE 4
#define BRAKE_TERM	10
#define SAMPLE_NUM SENSOR_VALUE_COUNTER
int biggestValue [22];

#define X_INIT	-500
#define Y_INIT	1600
#define Z_INIT	-15000

int16_t X_CALM;
int16_t Y_CALM;
int16_t Z_CALM;
#define MOVE_THRES	3000
#define GAP_THRES	2000

int corr_int [AXIS_NUM][AXIS_NUM];


enum GESTURES	{
	RIGHT,
	LEFT,
	FRONT,
	BACK,
	UP,
	DOWN,
	CLOCK,
	ANTI_CLOCK,
	LOW_CLOCK,
	LOW_ANTI,
	SIDE_CLOCK,
	SIDE_ANTI,
	FRONT_R,
	FRONT_L,
	BACK_R,
	BACK_L,
	RIGHT_F,
	RIGHT_B,
	LEFT_F,
	LEFT_B,
	NONE,
	SIDE,

};

#endif /* SRC_KEY_INPUT_H_ */
