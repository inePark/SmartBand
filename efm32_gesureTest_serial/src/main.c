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
#include <stdio.h>

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
#define BACKUP_SAMPLE	3


extern unsigned char gestureRecognition(int16_t ** sensors);

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/

static volatile uint32_t msTicks; /* counts 1ms timeTicks */
static void Delay(uint32_t dlyTicks);

void SysTick_Handler(void)
{
	msTicks++;       /* increment counter necessary in Delay()*/
}

static void Delay(uint32_t dlyTicks)
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

int main(void)
{


	int nLoop, nLoop2;
	uint32_t tickSensor1, tickSensor2, tickUart = 0;

	I2C_TransferReturn_TypeDef ret;
	uint8_t buf[6], maxFlag=0;
	int16_t ax, ay, az, gx, gy, gz;
	int16_t bax, bay, baz, bgx, bgy, bgz;
	int16_t sensors[6][SENSOR_VALUE_COUNTER];
	unsigned char gesture_result;

	/* Chip errata */
	CHIP_Init();
	//Enable VCOM
	//GPIO_PinModeSet(gpioPortC, 8, gpioModePushPull, 1);
	enter_DefaultMode_from_RESET();

	/* Setup SysTick Timer for 1 msec interrupts  */
	if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1) ;

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


	// Gyro range at +/-1000 ¡Æ/s: 0b00010000
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


	// Delay for Chip intialize

	Delay(1000);

	tickSensor1 = tickSensor2 = tickUart = msTicks;


	if( (msTicks - tickSensor1) > READING_CYCLE_MS_9AXIS_SENSOR1 ) {
		///////////////////////////// Read Accelerometer1 /////////////////////////////
		ret = i2c_read_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_ACCEL_X_H, buf, 6);
		if(ret != i2cTransferDone) {
			printf("i2cTransferFail[%x], ret =%d\n", DEV_ADDR_9AXIS, ret);
		}
		ax = ((int16_t)buf[0] << 8) | (int16_t)buf[1];
		ay = ((int16_t)buf[2] << 8) | (int16_t)buf[3];
		az = ((int16_t)buf[4] << 8) | (int16_t)buf[5];
		///////////////////////////// Read Gyroscope1 /////////////////////////////
		ret = i2c_read_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_GYRO_X_H, buf, 6);
		if(ret != i2cTransferDone) {
			printf("i2cTransferFail[%x], ret =%d\n", DEV_ADDR_9AXIS, ret);
		}
		gx = ((int16_t)buf[0] << 8) | (int16_t)buf[1];
		gy = ((int16_t)buf[2] << 8) | (int16_t)buf[3];
		gz = ((int16_t)buf[4] << 8) | (int16_t)buf[5];

		tickSensor1 = msTicks;
	}

	ax = ay = az = gx = gy = gz = 0;
	bax = bay = baz = bgx = bgy = bgz = 0;



	unsigned char counter = 0;
	maxFlag=0;
	printf ("START!\n");
		/* Infinite loop */
	while (1) {
		bax = ax;
		bay = ay;
		baz = az;
		bgx = gx;
		bgy = gy;
		bgz = gz;

		ax = ay = az = gx = gy = gz = 0;

		///////////////////////////// Read Accelerometer1 /////////////////////////////
		// X
		ret = i2c_read_with_regaddr(DEV_ADDR_9AXIS, REG_ADDR_9AXIS_ACCEL_X_H, buf, 1);
		//if(ret != i2cTransferDone) {
		//	printf("i2cTransferFail[%x], ret =%d\n", DEV_ADDR_9AXIS, ret);
		//}
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

			if (((ax > 6100) || (ax < -6000)) && ( abs(abs(ax) - abs(bax)) > 2000))	{
				maxFlag = 1;

			}
			else if (((ay > 5000) || (ay < -7000)) && (abs( abs(ay) - abs(bay)) > 2000))	{
				maxFlag = 1;
			}
			else if (((az > 23000) || (az < 11000)) && ( abs(abs(az) - abs(baz)) > 2000))	{
				maxFlag = 1;
			}
			/*else if ((gx > 5950) || (gx < -6050))	{
				maxFlag = 1;
			}
			else if ((gy > 6000) || (gy < -6000))	{
				maxFlag = 1;
			}
			else if ((gz > 6000) || (gz < -6000))	{
				maxFlag = 1;
			}*/
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
						sensors [nLoop][tempInt - nLoop2 - 1] = sensors [nLoop][SENSOR_VALUE_COUNTER -1 - nLoop2];

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



		if (counter == SENSOR_VALUE_COUNTER)	{

			counter = 0;
			if (maxFlag)	{

				maxFlag = 0;
				gesture_result = gestureRecognition((int16_t **)sensors);

				printf ("result: ");
				switch (gesture_result)	{
				case NONE:
					printf ("NOTHING\n\n\n");
					break;
				case RIGHT:
					printf (" RIGHT\n"); break;
				case LEFT:
					printf (" LEFT\n"); break;
				case FRONT:
					printf (" FRONT\n"); break;
				case BACK:
					printf (" BACK\n"); break;
				case UP:
					printf (" UP\n"); break;
				case DOWN:
					printf (" DOWN\n"); break;
				case CLOCK:
					printf (" CLOCK\n"); break;
				case ANTI_CLOCK:
					printf (" ANTI-CLOCK\n"); break;
				case LOW_CLOCK:
					printf (" LOW-CLOCK\n"); break;
				case LOW_ANTI:
					printf (" LOW-ANTI\n"); break;
				case SIDE_CLOCK:
					printf (" SIDE-CLOCK\n"); break;
				case SIDE_ANTI:
					printf (" SIDE-ANTI\n"); break;

				case UP_R:
					printf (" UP_R\n"); break;
				case UP_L:
					printf (" UP_L\n"); break;
				case DOWN_R:
					printf (" DOWN_R\n"); break;
				case DOWN_L:
					printf (" DOWN_L\n"); break;
				case RIGHT_U:
					printf (" RIGHT_U\n"); break;
				case RIGHT_D:
					printf (" RIGHT_D\n"); break;
				case LEFT_U:
					printf (" LEFT_U\n"); break;
				case LEFT_D:
					printf (" LEFT_D\n"); break;

				case FRONT_R:
					printf (" FRONT_R\n"); break;
				case FRONT_L:
					printf (" FRONT_L\n"); break;
				case BACK_R:
					printf (" BACK_R\n"); break;
				case BACK_L:
					printf (" BACK_L\n"); break;
				case RIGHT_F:
					printf (" RIGHT_F\n"); break;
				case RIGHT_B:
					printf (" RIGHT_B\n"); break;
				case LEFT_F:
					printf (" LEFT_F\n"); break;
				case LEFT_B:
					printf (" LEFT_B\n"); break;

				case UP_F:
					printf (" UP_F\n"); break;
				case UP_B:
					printf (" UP_B\n"); break;
				case DOWN_F:
					printf (" DOWN_F\n"); break;
				case DOWN_B:
					printf (" DOWN_B\n"); break;
				case FRONT_U:
					printf (" FRONT_U\n"); break;
				case FRONT_D:
					printf (" FRONT_D\n"); break;
				case BACK_U:
					printf (" BACK_U\n"); break;
				case BACK_D:
					printf (" BACK_D\n"); break;

				default:
					printf ("?? UNKNOWNN: %d\n\n\n", gesture_result);
					break;
				}
				printf ("\n");

			}

		}


		//function1();
		Delay(READING_DELAY_MS);



	}
}
