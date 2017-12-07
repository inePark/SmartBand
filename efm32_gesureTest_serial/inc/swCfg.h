#ifndef _SW_CFG_H_
#define _SW_CFG_H_

// 9axis-sensor2 enable flag, enable = 1 / disable = 0
#define SENSOR2_ENABLE	0


#define READING_CYCLE_MS_9AXIS_SENSOR1	50
#define TRANSMISSION_CYCLE_MS_TO_UART	1000


#define SENSOR_VALUE_COUNTER	20
#define SAMPLE_NUM SENSOR_VALUE_COUNTER
#define READING_DELAY_MS 50
#define AXIS_NUM	6

enum GESTURES	{
	RIGHT,
	LEFT,
	FRONT,
	BACK,
	UP,
	DOWN,

	CLOCK,	//6
	ANTI_CLOCK,
	LOW_CLOCK,
	LOW_ANTI,
	SIDE_CLOCK,
	SIDE_ANTI,

	UP_R,	//12
	UP_L,
	DOWN_R,
	DOWN_L,
	RIGHT_U,
	RIGHT_D,
	LEFT_U,
	LEFT_D,

	FRONT_R,	//20
	FRONT_L,
	BACK_R,
	BACK_L,
	RIGHT_F,
	RIGHT_B,
	LEFT_F,
	LEFT_B,

	UP_F,	//28
	UP_B,
	DOWN_F,
	DOWN_B,
	FRONT_U,
	FRONT_D,
	BACK_U,
	BACK_D,


	NONE,
	SIDE,
	PASS

};


#endif
