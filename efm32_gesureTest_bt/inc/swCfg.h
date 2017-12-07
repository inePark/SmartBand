#ifndef _SW_CFG_H_
#define _SW_CFG_H_

// 9axis-sensor2 enable flag, enable = 1 / disable = 0
//#define SENSOR2_ENABLE	1


//#define READING_CYCLE_MS_9AXIS_SENSOR1	50
//#define READING_CYCLE_MS_9AXIS_SENSOR2	50
//#define TRANSMISSION_CYCLE_MS_TO_UART	1000


////// ORIGIN
//#define READING_CYCLE_MS					200
//#define NUM_OF_SENSEOR_DATA_SET			5

////// Possible
//#define READING_CYCLE_MS					25
//#define NUM_OF_SENSEOR_DATA_SET				40

////// We will use
#define READING_CYCLE_MS					50
#define NUM_OF_SENSEOR_DATA_SET				20
#define SAMPLE_NUM	 NUM_OF_SENSEOR_DATA_SET
#define AXIS_NUM	6

#define START_ID_HB							0xFE
#define START_ID_LB							0xDC
#define STOP_ID_HB							0xAB
#define STOP_ID_LB							0xCD
#define DATA_HEADER_LEN						4
#define DATA_TAIL_LEN						2
#define S1_ACCEL_X_OFFSET					DATA_HEADER_LEN
#define S1_ACCEL_Y_OFFSET					((1*NUM_OF_SENSEOR_DATA_SET*2) + S1_ACCEL_X_OFFSET)
#define S1_ACCEL_Z_OFFSET					((2*NUM_OF_SENSEOR_DATA_SET*2) + S1_ACCEL_X_OFFSET)
#define S1_GYRO_X_OFFSET					((3*NUM_OF_SENSEOR_DATA_SET*2) + S1_ACCEL_X_OFFSET)
#define S1_GYRO_Y_OFFSET					((4*NUM_OF_SENSEOR_DATA_SET*2) + S1_ACCEL_X_OFFSET)
#define S1_GYRO_Z_OFFSET					((5*NUM_OF_SENSEOR_DATA_SET*2) + S1_ACCEL_X_OFFSET)

enum GESTURES	{
	NONE,

	RIGHT,
	LEFT,
	FRONT,
	BACK,
	UP,
	DOWN,

	CLOCK,	//7
	ANTI_CLOCK,
	LOW_CLOCK,
	LOW_ANTI,
	SIDE_CLOCK,
	SIDE_ANTI,

	UP_R,	//13
	UP_L,
	DOWN_R,
	DOWN_L,
	RIGHT_U,
	RIGHT_D,
	LEFT_U,
	LEFT_D,

	FRONT_R,	//21
	FRONT_L,
	BACK_R,
	BACK_L,
	RIGHT_F,
	RIGHT_B,
	LEFT_F,
	LEFT_B,

	UP_F,	//29
	UP_B,
	DOWN_F,
	DOWN_B,
	FRONT_U,
	FRONT_D,
	BACK_U,
	BACK_D,


	SIDE,

};

typedef struct sensor_data_st {
	unsigned char accel_x[2];
	unsigned char accel_y[2];
	unsigned char accel_z[2];
	unsigned char gyro_x[2];
	unsigned char gyro_y[2];
	unsigned char gyro_z[2];
} sensor_data_t;

#endif
