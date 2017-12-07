



///////////////////////////////////////////////////////////////////////////////
//
//		Gestre detection with NO SYNCHONIZED DURATION 20161130 			//
//		FOR DEMO SET of Smart key 과제
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////////


/**************************************************************************//**
 * @file
 * @brief Empty Project
 * @author Energy Micro AS
 * @version 3.20.2
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silicon Labs Software License Agreement. See 
 * "http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt"  
 * for details. Before using this software for any purpose, you must agree to the 
 * terms of that agreement.
 *
 ******************************************************************************/
#include "em_device.h"
#include "em_chip.h"
#include "em_usart.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_i2c.h"
#include "swCfg.h"
#include "bsp.h"
#include "bglib_txrx.h"
#include "key_input.h"

#include <stdio.h>
//---add
#include <stdint.h>
#define GESTURE_INBOARD

#define DEV_ADDR_9AXIS					0xD0
#define DEV_ADDR_9AXIS2					0xD2
#define DEV_ADDR_9AXIS_MAGNETO			0x18
#define REG_ADDR_9AXIS_SMPRT_DIV		0x19    // Gyro sampling rate divider
#define REG_ADDR_9AXIS_CONFIG			0x1A
#define CONFIG_DLPF_2MS_DELAY			0x01
#define REG_ADDR_GYRO_CONFIG			0x1B
#define CONFIG_GYRO_VAR_1000			0x10
#define REG_ADDR_ACCEL_CONFIG			0x1C
#define CONFIG_ACCEL_VAR_2				0x00
#define REG_ADDR_FIFO_EN				0x23
#define REG_ADDR_INT_PIN_CFG			0x37
#define CONFIG_INT_PIN_I2C_BYPASS		0x02
#define REG_ADDR_INT_ENABLE				0x38
#define REG_ADDR_USER_CTRL				0x6A

#define REG_ADDR_9AXIS_PWR_MGMT_1		0x6B	// R/W
#define REG_ADDR_9AXIS_MAGNETO_CNTL		0x0A	// R/W
#define REG_ADDR_9AXIS_ACCEL_X_H		0x3B	// R
#define REG_ADDR_9AXIS_ACCEL_X_L		0x3C   // R
#define REG_ADDR_9AXIS_ACCEL_Y_H		0x3D   // R
#define REG_ADDR_9AXIS_ACCEL_Y_L		0x3E   // R
#define REG_ADDR_9AXIS_ACCEL_Z_H		0x3F   // R
#define REG_ADDR_9AXIS_ACCEL_Z_L		0x40   // R
#define REG_ADDR_9AXIS_GYRO_X_H			0x43   // R
#define REG_ADDR_9AXIS_GYRO_X_L			0x44   // R
#define REG_ADDR_9AXIS_GYRO_Y_H			0x45   // R
#define REG_ADDR_9AXIS_GYRO_Y_L			0x46   // R
#define REG_ADDR_9AXIS_GYRO_Z_H			0x47   // R
#define REG_ADDR_9AXIS_GYRO_Z_L			0x48   // R
#define REG_ADDR_9AXIS_MAGNETO_X_H		0x03  // R
#define REG_ADDR_9AXIS_MAGNETO_X_L		0x04   // R
#define REG_ADDR_9AXIS_MAGNETO_Y_H		0x05   // R
#define REG_ADDR_9AXIS_MAGNETO_Y_L		0x06   // R
#define REG_ADDR_9AXIS_MAGNETO_Z_H		0x07   // R
#define REG_ADDR_9AXIS_MAGNETO_Z_L		0x08   // R
#define BACKUP_SAMPLE	5


uint32_t INT_LockCnt;
extern unsigned char gestureRecognition (int16_t** sensor);

 /**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/

static volatile uint32_t msTicks; /* counts 1ms timeTicks */
 void Delay(uint32_t dlyTicks);

void SysTick_Handler(void)
{
	msTicks++;       /* increment counter necessary in Delay()*/
}

 void Delay(uint32_t dlyTicks)
{
	uint32_t curTicks;

	curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks) ;
}


I2C_TransferReturn_TypeDef i2c_write_with_regaddr(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, int len)
{
	I2C_TransferReturn_TypeDef ret;
	I2C_TransferSeq_TypeDef	i2cSeq;

	i2cSeq.addr = dev_addr;
	i2cSeq.flags = I2C_FLAG_WRITE_WRITE;
	i2cSeq.buf[0].data = &reg_addr;
	i2cSeq.buf[0].len = 1;
	i2cSeq.buf[1].data = (uint8_t *)(data);
	i2cSeq.buf[1].len = len;
	ret = I2C_TransferInit(I2C0, &i2cSeq);
	while (ret == i2cTransferInProgress)
	{
		ret = I2C_Transfer(I2C0);
	}

	return ret;
}

I2C_TransferReturn_TypeDef i2c_read_with_regaddr(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, int len)
{
	I2C_TransferReturn_TypeDef ret;
	I2C_TransferSeq_TypeDef	i2cSeq;

	i2cSeq.addr = dev_addr;
	i2cSeq.flags = I2C_FLAG_WRITE_READ;
	i2cSeq.buf[0].data = &reg_addr;
	i2cSeq.buf[0].len = 1;
	i2cSeq.buf[1].data = (uint8_t *)(data);
	i2cSeq.buf[1].len = len;
	ret = I2C_TransferInit(I2C0, &i2cSeq);
	while (ret == i2cTransferInProgress)
	{
		ret = I2C_Transfer(I2C0);
	}

	return ret;
}

void LED_blink (int led, int count)	{
	int nLoop;

	for (nLoop=0; nLoop < count; nLoop ++)	{
		BSP_LedsSet (led);
		Delay(200);
		BSP_LedsSet (0);
		Delay(200);

	}
}/*
void LED1_blink (int count)	{
	int nLoop=0;

	for (nLoop; nLoop < count; nLoop ++)	{
		BSP_LedsSet (2);
		Delay(500);
		BSP_LedsSet (0);

	}
}
*/



int counter = 0, nLoop, nLoop2;
//sensor_data_t ds1[NUM_OF_SENSEOR_DATA_SET], ds2[NUM_OF_SENSEOR_DATA_SET];

I2C_TransferReturn_TypeDef ret;
uint8_t buf[6], maxFlag;
//uint8_t buffer[(12*NUM_OF_SENSEOR_DATA_SET) + DATA_HEADER_LEN + DATA_TAIL_LEN];
uint8_t buffer [9];
int16_t ax, ay, az, gx, gy, gz;
int16_t sensors[AXIS_NUM][NUM_OF_SENSEOR_DATA_SET];
unsigned char gesture_result;
BGLIB_DEFINE();

int main(void)
{
	/* Chip errata */
	CHIP_Init();

	//LED0_blink(1);

	//Enable VCOM
	I2C_TransferReturn_TypeDef ret;

	enter_DefaultMode_from_RESET();
	/* Setup SysTick Timer for 1 msec interrupts  */
	if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1) ;

	/* 여기 위로는 코드 작성하지 마세요 */

  /* Initialize LED driver */
	BSP_LedsInit();
	init_user_usart();
	BGLIB_INITIALIZE(on_message_send);





	// 1Khz sampling rate
	buf[0] = 0x00;
	ret = i2c_write_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_SMPRT_DIV, buf, 1);
	if(ret != i2cTransferDone) {
		printf("i2cTransferFail[%x], ret =%d\n", DEV_ADDR_9AXIS, ret);
	}

	// No ext sync, DLPF at 184Hz for the accel and 188Hz for the gyro: 0b00000001
	buf[0] = CONFIG_DLPF_2MS_DELAY;
	ret = i2c_write_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_CONFIG, buf, 1);
	if(ret != i2cTransferDone) {
		printf("i2cTransferFail[%x], ret =%d\n", DEV_ADDR_9AXIS, ret);
	}

	// Gyro range at +/-1000 °/s: 0b00010000
	buf[0] = CONFIG_GYRO_VAR_1000;
	ret = i2c_write_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_GYRO_CONFIG, buf, 1);
	if(ret != i2cTransferDone) {
		printf("i2cTransferFail[%x], ret =%d\n", DEV_ADDR_9AXIS, ret);
	}

	// Accel range at +/-2g: 0b00000000
	buf[0] = CONFIG_ACCEL_VAR_2;
	ret = i2c_write_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_ACCEL_CONFIG, buf, 1);
	if(ret != i2cTransferDone) {
		printf("i2cTransferFail[%x], ret =%d\n", DEV_ADDR_9AXIS, ret);
	}

	// Disable all FIFOs
	buf[0] = 0x00;
	ret = i2c_write_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_FIFO_EN, buf, 1);
	if(ret != i2cTransferDone) {
		printf("i2cTransferFail[%x], ret =%d\n", DEV_ADDR_9AXIS, ret);
	}

	// Bypass mode enabled: 0b00000010
	buf[0] = CONFIG_INT_PIN_I2C_BYPASS;
	ret = i2c_write_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_INT_PIN_CFG, buf, 1);
	if(ret != i2cTransferDone) {
		printf("i2cTransferFail[%x], ret =%d\n", DEV_ADDR_9AXIS, ret);
	}

	// Disable all interrupts: 0b00000000
	buf[0] = 0x00;
	ret = i2c_write_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_INT_ENABLE, buf, 1);
	if(ret != i2cTransferDone) {
		printf("i2cTransferFail[%x], ret =%d\n", DEV_ADDR_9AXIS, ret);
	}

	// No FIFO and no I2C slaves: 0b00000000
	buf[0] = 0x00;
	ret = i2c_write_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_USER_CTRL, buf, 1);
	if(ret != i2cTransferDone) {
		printf("i2cTransferFail[%x], ret =%d\n", DEV_ADDR_9AXIS, ret);
	}

	// Enable 9Axis Sensor
	buf[0] = 0;
	ret = i2c_write_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_PWR_MGMT_1, buf, 1);
	if(ret != i2cTransferDone) {
		printf("i2cTransferFail[%x], ret =%d\n", DEV_ADDR_9AXIS, ret);
	}

	//dumo_cmd_system_hello();



	// Delay for Chip intialize
	Delay(1000);

	maxFlag = 0;


	//dumo_cmd_system_hello();

//	int temp=2;
	while (1) {
//		LED_blink (3, temp++);

		handle_bt_rx();



	}
}



uint8_t get_key_value ()	{

	buffer[5] = gesture_result = 0;
	ax = ay = az = gx = gy = gz = 0;

	///////////////////////////// Read Accelerometer1 /////////////////////////////
	// X
	ret = i2c_read_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_ACCEL_X_H, buf, 1);
	ax = ((int16_t)buf[0] << 8);
	ret = i2c_read_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_ACCEL_X_L, buf, 1);
	ax = ax | (int16_t)buf[0];
	// Y
	ret = i2c_read_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_ACCEL_Y_H, buf, 1);
	ay = (buf[0] << 8);
	ret = i2c_read_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_ACCEL_Y_L, buf, 1);
	ay = ay | buf[0];
	// Z
	ret = i2c_read_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_ACCEL_Z_H, buf, 1);
	az = (buf[0] << 8);
	ret = i2c_read_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_ACCEL_Z_L, buf, 1);
	az = az | buf[0];



	///////////////////////////// Read Gyroscope  /////////////////////////////
	// X
	ret = i2c_read_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_GYRO_X_H, buf, 1);
	gx = (buf[0] << 8);
	ret = i2c_read_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_GYRO_X_L, buf, 1);
	gx = gx | buf[0];
	// Y
	ret = i2c_read_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_GYRO_Y_H, buf, 1);
	gy = (buf[0] << 8);
	ret = i2c_read_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_GYRO_Y_L, buf, 1);
	gy = gy | buf[0];
	// Z
	ret = i2c_read_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_GYRO_Z_H, buf, 1);
	gz = (buf[0] << 8);
	ret = i2c_read_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_GYRO_Z_L, buf, 1);
	gz = gz | buf[0];


	if (!maxFlag)	{	//checking branch

		if ((ax > 3100) || (ax < -3000))	{
			maxFlag = 1;
		}
		else if ((ay > 2000) || (ay < -4000))	{
			maxFlag = 1;
		}
		else if ((az > 20000) || (az < 14000))	{
			maxFlag = 1;
		}
		else if ((gx > 2950) || (gx < -3050))	{
			maxFlag = 1;
		}
		else if ((gy > 3000) || (gy < -3000))	{
			maxFlag = 1;
		}
		else if ((gz > 3000) || (gz < -3000))	{
			maxFlag = 1;
		}
	}



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
					sensors [nLoop][tempInt - nLoop2 - 1] = sensors [nLoop][NUM_OF_SENSEOR_DATA_SET -1 - nLoop2];

				} //for nLoop2

			} //for nLoop
		} //else if

		counter  = BACKUP_SAMPLE;
		maxFlag = 2;
	} //if

	sensors [0][counter] = ax;
	sensors [1][counter] = ay;
	sensors [2][counter] = az;
	sensors [3][counter] = gx;
	sensors [4][counter] = gy;
	sensors [5][counter ++] = gz;


	if (counter == NUM_OF_SENSEOR_DATA_SET)	{
		counter = 0;
		if (maxFlag)	{

			maxFlag = 0;
			gesture_result = gestureRecognition((int16_t **)sensors);

			switch (gesture_result)	{
			case NONE:
				gesture_result = KEYBOARD_NONE;
				break;
			default:
				gesture_result = KEYBOARD_NONE;
				break;

			case FRONT:
				gesture_result = KEYBOARD_RETURN;
				break;
			case BACK:
				gesture_result = KEYBOARD_DEL;
				break;
			case RIGHT:
				gesture_result=KEYBOARD_RIGHT;
				break;
			case LEFT:
				gesture_result=KEYBOARD_LEFT;
				break;
			case UP:
				gesture_result = KEYBOARD_UP;
				break;
			case DOWN:
				gesture_result = KEYBOARD_DOWN;
				break;

			case CLOCK:
				gesture_result = KEYBOARD_HOME;
				break;
			case ANTI_CLOCK:
				gesture_result = KEYBOARD_END;
				break;

			case RIGHT_F:
				gesture_result = KEYBOARD_0;
				break;
			case RIGHT_B:
				gesture_result = KEYBOARD_1;
				break;
			case RIGHT_U:
				gesture_result = KEYBOARD_2;
				break;
			case RIGHT_D:
				gesture_result = KEYBOARD_3;
				break;

			case FRONT_R:
				gesture_result = KEYBOARD_4;
				break;
			case FRONT_L:
				gesture_result = KEYBOARD_5;
				break;
			case FRONT_U:
				gesture_result = KEYBOARD_6;
				break;
			case FRONT_D:
				gesture_result = KEYBOARD_7;
				break;

			case UP_R:
				gesture_result = KEYBOARD_8;
				break;
			case UP_L:
				gesture_result = KEYBOARD_9;
				break;




			} //switch
		} //if
	} //if
	return gesture_result;
}




