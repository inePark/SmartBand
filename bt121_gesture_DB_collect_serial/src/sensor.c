/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2016-2016 InvenSense Inc. All rights reserved.
 *
 * This software, related documentation and any modifications thereto (collectively 諛챣ftware占�) is subject
 * to InvenSense and its licensors' intellectual property rights under U.S. and international copyright
 * and other intellectual property rights laws.
 *
 * InvenSense and its licensors retain all intellectual property and proprietary rights in and to the Software
 * and any use, reproduction, disclosure or distribution of the Software without an express license agreement
 * from InvenSense is strictly prohibited.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE SOFTWARE IS
 * PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * INVENSENSE BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 * ________________________________________________________________________________________________________
 */

#include <stdint.h>

/* board driver */
#include "em_gpio.h"
#include "em_i2c.h"

/* std */
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "user_usart.h"
#include "user_delay.h"
#include "sensor.h"
#include "Icm20602Defs.h"
#include "InvError.h"

void init_sensor()
{
	uint8_t data;
	int result;

	result |= inv_icm20602_soft_reset();

	/*
	 * Make Sleep State
	 */
#if 0
	data = CLK_SEL;
	result |= i2c_write_reg8(ICM_I2C_ADDR, MPUREG_PWR_MGMT_1, &data, 1);
	data = BIT_PWR_ACCEL_STBY | BIT_PWR_GYRO_STBY;
	result |= i2c_write_reg8(ICM_I2C_ADDR, MPUREG_PWR_MGMT_2, &data, 1);
#endif

	/*
	 * Configuration Reg
	 */
	data = BIT_FIFO_SNAPSHOT_MODE | 1; //DLPF CFG Goes to 1KHz Rate
	result |= i2c_write_reg8(ICM_I2C_ADDR, MPUREG_CONFIG, &data, 1);


	/* Setting Samaple rate : SMPLRT_DIV reg */
	/*
	 *  Internal Sample rate 20Hz = 1000/(49+1)
	 */
	data = 49;
	result |= i2c_write_reg8(ICM_I2C_ADDR, MPUREG_SMPLRT_DIV, &data, 1);





	/*
	 * Gyroscope Configuration, fullScale = 250dps
	 */
	data = (MPU_FS_250dps << BIT_POS_ACCEL_FS_SEL);
	result |= i2c_write_reg8(ICM_I2C_ADDR, MPUREG_GYRO_CONFIG, &data, 1);


	/*
	 * Accel Configuration, fullscale = +-2g
	 */
	data = (MPU_FS_2G << BIT_POS_ACCEL_FS_SEL);
	result |= i2c_write_reg8(ICM_I2C_ADDR, MPUREG_ACCEL_CONFIG, &data, 1);

	/*
	 * Accel Configuration2, Sample average
	 */
	data = BITS_ACCEL_FCHOICE_B;	//1sample
	result |= i2c_write_reg8(ICM_I2C_ADDR, MPUREG_ACCEL_CONFIG_2, &data, 1);



	/*
	 * FIFO EN
	 */
	data = 0; // FIFO DISABLE
	result |= i2c_write_reg8(ICM_I2C_ADDR, MPUREG_FIFO_EN, &data, 1);

	/* USER Control */
	data = 0; //FIFO Off,
	result |= i2c_write_reg8(ICM_I2C_ADDR, MPUREG_USER_CTRL, &data, 1);

	/*
	 * FSYNC
	 */

	/*
	 * INT
	 */

	/*
	 * Accelerometer OUTPUT Limit
	 */
	//data = 2; // 6 axis sensor on
	//result |= i2c_write_reg8(ICM_I2C_ADDR, MPUREG_ACCEL_INTEL_CTRL, &data, 1);

	/*
	 * Gyro Low Power Cfg
	 */
	data = BIT_GYRO_CYCLE;	//1sample low power en
	result |= i2c_write_reg8(ICM_I2C_ADDR, MPUREG_LP_CONFIG, &data, 1);

	/* Power MGMT2 */
	data = 0; // 6 axis sensor on
	result |= i2c_write_reg8(ICM_I2C_ADDR, MPUREG_PWR_MGMT_2, &data, 1);

	/* Power MGMT1 */
	data = CLK_SEL; // FIFO DISABLE
	//data = BIT_CYCLE | BIT_GYRO_STDBY |BIT_TEMP_DISABLE | CLK_SEL; // FIFO DISABLE
	result |= i2c_write_reg8(ICM_I2C_ADDR, MPUREG_PWR_MGMT_1, &data, 1);




}

int inv_icm20602_soft_reset()
{
	int result = 0;
	int timeout = 1000; /* 1s */
	uint8_t data;

	// Reset the internal registers and restores the default settings.
	// The bit automatically clears to 0 once the reset is done.
	data = BIT_DEVICE_RESET;
	result |= i2c_write_reg8(ICM_I2C_ADDR, MPUREG_PWR_MGMT_1, &data, 1);
	if(result) {
		return result;
	}

	Delay(50); // wait for 50ms after soft reset

	do {
		result = i2c_read_reg8(ICM_I2C_ADDR, MPUREG_PWR_MGMT_1, &data, 1);
		if(result) {
			return result;
		}

		Delay(1);
		timeout -= 1;

		if(timeout < 0) {
			return INV_ERROR_TIMEOUT;
		}

	} while (data != RST_VAL_PWR_MGMT_1); // this is the default expected value

	return 0;
}


void do_sensing(int16_t *pAx, int16_t *pAy, int16_t *pAz, int16_t *pGx, int16_t *pGy, int16_t *pGz  )
{
	int rc = 0;
	uint8_t ddry = 0;
	uint8_t int_status;
	uint8_t raw_acc[6] = {0,}, raw_gyro[6] = {0,}, sRtemp_data[2];


#if 0
	// check INT
	/*
	 *  Ensure data ready status
	 */
	if ((rc = i2c_read_reg8(ICM_I2C_ADDR, &REG_INT_STATUS, &int_status, 1)) == i2cTransferDone) {
		ddry = inv_icm20602_check_drdy(int_status);
	}

	if (ddry) {
		/* Read Data */


	}
#endif
	/* Read Raw Data */
	rc |= i2c_read_reg8(ICM_I2C_ADDR, MPUREG_ACCEL_XOUT_H, raw_acc, 2);
	rc |= i2c_read_reg8(ICM_I2C_ADDR, MPUREG_ACCEL_YOUT_H, raw_acc +2, 2);
	rc |= i2c_read_reg8(ICM_I2C_ADDR, MPUREG_ACCEL_ZOUT_H, raw_acc +4, 2);
	rc |= i2c_read_reg8(ICM_I2C_ADDR, MPUREG_GYRO_XOUT_H, raw_gyro, 2);
	rc |= i2c_read_reg8(ICM_I2C_ADDR, MPUREG_GYRO_YOUT_H, raw_gyro +2, 2);
	rc |= i2c_read_reg8(ICM_I2C_ADDR, MPUREG_GYRO_ZOUT_H, raw_gyro +4, 2);

	*pAx = (raw_acc[0] << 8) | raw_acc[1];
	*pAy = (raw_acc[2] << 8) | raw_acc[3];
	*pAz = (raw_acc[4] << 8) | raw_acc[5];
	*pGx = (raw_gyro[0] << 8) | raw_gyro[1];
	*pGy = (raw_gyro[2] << 8) | raw_gyro[3];
	*pGz = (raw_gyro[4] << 8) | raw_gyro[5];
}

I2C_TransferReturn_TypeDef i2c_write_reg8(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, int len)
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

I2C_TransferReturn_TypeDef i2c_read_reg8(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, int len)
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


/*
 * Helper function to check RC value and block programm exectution
 */
static void check_rc(int rc, const char * msg_context)
{
	if(rc < 0) {
		LOG_ERR("%s: error %d (%s)\n", msg_context, rc, inv_error_str(rc));
		while(1);
	}
}

int inv_icm20602_check_drdy(uint8_t int_status)
{
	if(int_status & BIT_DATA_RDY_INT)
		return 1;

	return 0;
}


