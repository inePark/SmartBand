/*
 * key_input.c
 *
 *  Created on: 2017. 1. 18.
 *      Author: netbugger
 */

#include <stdint.h>
#include "em_gpio.h"
#include "em_usart.h"
#include "em_cmu.h"
#include "InitDevice.h"
#include "user_usart.h"
#include "key_input.h"
#include "sensor.h"

extern unsigned char gestureRecognition(int16_t ** sensors);


uint8_t btn_input_status = KEYBOARD_NONE;
uint16_t joystickValue = JOYSTICK_NORMAL, lastJoystickValue = JOYSTICK_NORMAL;

uint8_t get_key_value_from_gesture();

uint8_t get_key_value()
{
	return  get_key_value_from_gesture();
}

uint8_t get_key_value_from_gesture()
{
	static int16_t bax=X_INIT, bay=Y_INIT, baz=Z_INIT;
	static int cnt = 0, brk_term = 0;
	uint8_t gesture_result=KEYBOARD_NONE;
	static int16_t ax=X_INIT,ay=Y_INIT,az=Z_INIT, gx=0, gy=0, gz=0;
	static int counter = 0, maxFlag = 0;
	static int16_t sensors[AXIS_NUM][SENSOR_VALUE_COUNTER];
	uint16_t	nLoop, nLoop2;
	cnt++;

	if(cnt == SAMPLING_DELAY) {		//logging every 500ms,



		bax = ax;
		bay = ay;
		baz = az;
		cnt = 0;
		do_sensing(&ax, &ay, &az, &gx, &gy, &gz);
		/*
		 * TODO
		 * insert into gesture buffer for analyzing
		 */

		if (brk_term)	{

			brk_term --;
			return KEYBOARD_NONE;
		}

#if 0
		// --------------------------------
		// for Kalman filter TEST
		//---------------------------------
		if (!maxFlag)	{	//checking branch
			maxFlag = 1;

		}

#else
		//printf ("RAW : %d %d %d \t", ax, ay, az);

		if (!maxFlag)	{	//checking branch

			if (((ax > X_CALM + MOVE_THRES) || (ax <  X_CALM - MOVE_THRES)) && ( abs(abs(ax) - abs(bax)) > GAP_THRES))	{
				maxFlag = 1;
				USART1_SEND (" hit-x ", 7);

				//printf ("[x]: %d, %d = %d\n", abs(ax), abs(bax),  abs(abs(ax) - abs(bax)));
			}
			else if (((ay >  Y_CALM + MOVE_THRES) || (ay < Y_CALM - MOVE_THRES)) && (abs( abs(ay) - abs(bay)) > GAP_THRES))	{
				maxFlag = 1;
				USART1_SEND (" hit-y ", 7);
				//printf ("[y]: %d, %d = %d\n", abs(ay), abs(bay),  abs(abs(ay) - abs(bay)));
			}
			else if (((az >  Z_CALM + MOVE_THRES) || (az < Z_CALM - MOVE_THRES)) && (abs( abs(az) - abs(baz)) > GAP_THRES))	{
				maxFlag = 1;
				USART1_SEND (" hit-z ", 7);
				//printf ("[z]: %d, %d = %d\n", abs(az), abs(baz),  abs(abs(az) - abs(baz)));
			}
			/*else {


				KalmanFilter();

			}*/

		}

#endif

		if (maxFlag == 1)	{	//block copy(5 factors) from array to array

			if (counter > BACKUP_SAMPLE)	{ 	//bigger than 5th sample
				for (nLoop = 0; nLoop < AXIS_NUM; nLoop ++)	{
					for (nLoop2 = 0; nLoop2 < BACKUP_SAMPLE; nLoop2 ++)	{
						sensors [nLoop][nLoop2] = sensors [nLoop][counter - BACKUP_SAMPLE + nLoop2];

					} //for nLoop2
				} //for nLoop
			} //else if

			else if (counter < BACKUP_SAMPLE)	{	//smaller than 5th sample

				for (nLoop = 0; nLoop < AXIS_NUM; nLoop ++)	{

					for (nLoop2 = 1; nLoop2 <= counter; nLoop2 ++)	{
						sensors [nLoop][BACKUP_SAMPLE - nLoop2] = sensors [nLoop][counter - nLoop2];
					} //for nLoop2
					int tempInt = BACKUP_SAMPLE - counter;
					for (nLoop2 = 0; nLoop2 < tempInt; nLoop2 ++)	{
						sensors [nLoop][tempInt - nLoop2 - 1] = sensors [nLoop][SENSOR_VALUE_COUNTER -1 - nLoop2];

					} //for nLoop2

				} //for nLoop
			} //else if

			counter  = BACKUP_SAMPLE;
			maxFlag = 2;
		} //if

		sensors [0][counter] = ax;
		sensors [1][counter] = ay;
		sensors [2][counter ++] = az;
	/*	sensors [3][counter] = gx;
		sensors [4][counter] = gy;
		sensors [5][counter ++] = gz;

*/
		if (counter == SENSOR_VALUE_COUNTER)	{

			counter = 0;
			if (maxFlag)	{


				maxFlag = 0;
				gesture_result = gestureRecognition((int16_t **)sensors);

#if 0
				printf ("result: ");
				switch (gesture_result)	{
				case NONE:
					LOG_NOR ("NOTHING\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case RIGHT:
					LOG_NOR (" RIGHT\n");
					gesture_result=KEYBOARD_RIGHT;
					break;
				case LEFT:
					LOG_NOR (" LEFT\n");
					gesture_result = KEYBOARD_LEFT;
					break;
				case FRONT:
					gesture_result = KEYBOARD_RETURN;
					LOG_NOR (" FRONT\n");
					break;
				case BACK:
					LOG_NOR (" BACK\n");
					gesture_result = KEYBOARD_DEL;
					break;
				case UP:
					LOG_NOR (" UP\n");
					gesture_result = KEYBOARD_UP;
					break;
				case DOWN:
					LOG_NOR (" DOWN\n");
					gesture_result = KEYBOARD_DOWN;
					break;
				case CLOCK:
					LOG_NOR (" CLOCK\n");
					gesture_result = KEYBOARD_HOME;
					break;
				case ANTI_CLOCK:
					LOG_NOR (" ANTI-CLOCK\n");
					gesture_result = KEYBOARD_END;
					break;
				case LOW_CLOCK:
					LOG_NOR (" LOW-CLOCK\n");
					gesture_result = KEYBOARD_ESC;
					break;
				case LOW_ANTI:
					LOG_NOR (" LOW-ANTI\n");
					gesture_result = KEYBOARD_TAB;
					break;
				case SIDE_CLOCK:
					LOG_NOR (" SIDE-CLOCK\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case SIDE_ANTI:
					LOG_NOR (" SIDE-ANTI\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case FRONT_R:
					LOG_NOR (" FRONT_R\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case FRONT_L:
					LOG_NOR (" FRONT_L\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case BACK_R:
					LOG_NOR (" BACK_R\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case BACK_L:
					LOG_NOR (" BACK_L\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case RIGHT_F:
					LOG_NOR (" RIGHT_F\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case RIGHT_B:
					LOG_NOR (" RIGHT_B\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case LEFT_F:
					LOG_NOR (" LEFT_F\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case LEFT_B:
					LOG_NOR (" LEFT_B\n");
					gesture_result = KEYBOARD_NONE;
					break;


				default:
					LOG_NOR ("?? UNKNOWNN: %d\n", gesture_result);
					gesture_result = KEYBOARD_NONE;
					break;
				}
				LOG_NOR ("\n");



#else


				printf ("result: ");
				switch (gesture_result)	{
				case NONE:
					USART1_SEND (" NOTHING ", 9);
					gesture_result = KEYBOARD_NONE;
					break;
				case RIGHT:
					USART1_SEND (" RIGHT ", 7);
					gesture_result=KEYBOARD_RIGHT;
					break;
				case LEFT:
					USART1_SEND (" LEFT ", 6);
					gesture_result = KEYBOARD_LEFT;
					break;
				case FRONT:
					USART1_SEND (" FRONT ", 7);
					gesture_result = KEYBOARD_RETURN;
					break;
				case BACK:
					USART1_SEND (" BACK ", 6);
					gesture_result = KEYBOARD_DEL;
					break;
				case UP:
					USART1_SEND (" UP ", 4);
					gesture_result = KEYBOARD_UP;
					break;
				case DOWN:
					USART1_SEND (" DOWN ", 6);
					gesture_result = KEYBOARD_DOWN;
					break;
				case CLOCK:
					USART1_SEND (" CLOCK ", 7);
					gesture_result = KEYBOARD_HOME;
					break;
				case ANTI_CLOCK:
					USART1_SEND (" ANTI-CLOCK ", 12);
					gesture_result = KEYBOARD_END;
					break;
				case LOW_CLOCK:
					USART1_SEND (" LOW-CLOCK ", 11);
					gesture_result = KEYBOARD_ESC;
					break;
				case LOW_ANTI:
					USART1_SEND (" ANTI-LOW ", 10);
					gesture_result = KEYBOARD_TAB;
					break;
				case SIDE_CLOCK:
					LOG_NOR (" SIDE-CLOCK\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case SIDE_ANTI:
					LOG_NOR (" SIDE-ANTI\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case FRONT_R:
					LOG_NOR (" FRONT_R\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case FRONT_L:
					LOG_NOR (" FRONT_L\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case BACK_R:
					LOG_NOR (" BACK_R\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case BACK_L:
					LOG_NOR (" BACK_L\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case RIGHT_F:
					LOG_NOR (" RIGHT_F\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case RIGHT_B:
					LOG_NOR (" RIGHT_B\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case LEFT_F:
					LOG_NOR (" LEFT_F\n");
					gesture_result = KEYBOARD_NONE;
					break;
				case LEFT_B:
					LOG_NOR (" LEFT_B\n");
					gesture_result = KEYBOARD_NONE;
					break;


				default:
					LOG_NOR ("?? UNKNOWNN: %d\n", gesture_result);
					gesture_result = KEYBOARD_NONE;
					break;
				}
				LOG_NOR ("\n");
#endif
				brk_term = BRAKE_TERM;
			}

			bax = X_CALM;
			bay = Y_CALM;
			baz = Z_CALM;
		}

/*
#if GESTURE_LOG
		LOG_NOR("%d, %d, %d\n", x, y, z);
#endif
*/
	}


	return gesture_result;
}


void KalmanFilter ()	{

	static float X_ESTt = X_INIT;
	static float Y_ESTt = Y_INIT;
	static float Z_ESTt = Z_INIT;

	static float X_Eest = 300;
	static float Y_Eest = 300;
	static float Z_Eest = 300;

	//Error in measurement = 200
	int Emea = 200;
	float KG = 0;

	KG = X_Eest / (X_Eest + Emea);
	X_ESTt = X_ESTt + KG;
	printf ("KG: %d(%d),", (int)(KG*100000), (int)((X_Eest / (X_Eest + 200))*10000));
	X_Eest = (1-KG) * X_Eest;

	KG = Y_Eest / (Y_Eest + Emea);
	Y_ESTt = Y_ESTt + KG;
	printf ("%d(%d),", (int)(KG*100000), (int)((Y_Eest / (Y_Eest + 200))*10000));

	Y_Eest = (1-KG) * Y_Eest;


	KG = Z_Eest / (Z_Eest + Emea);
	Z_ESTt = Z_ESTt + KG;
	printf ("%d(%d)\t", (int)(KG*100000), (int)((Z_Eest / (Z_Eest + 200))*10000));

	Z_Eest = (1-KG) * Z_Eest;


	X_CALM = (int)(X_ESTt * 100);
	Y_CALM = (int)(Y_ESTt * 100);
	Z_CALM = (int)(Z_ESTt * 100);

	printf ("KF: %d %d %d\n", X_CALM, Y_CALM, Z_CALM);

}
