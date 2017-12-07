/*
 * sensor.h
 *
 *  Created on: 2017. 3. 28.
 *      Author: netbugger
 */

#ifndef SRC_SENSOR_H_
#define SRC_SENSOR_H_

#include "em_i2c.h"


/* FSR configurations */

/* Sensitivity configurations */
#define ACC_SENSITIVITY (int32_t) ( ((cfg_acc_fsr/1000) * (1 << 16)) / INT16_MAX)
#define GYR_SENSITIVITY (int32_t) ( (cfg_gyr_fsr * (1 << 16) / INT16_MAX) )


#define ICM_I2C_ADDR     (0x68 << 1) /* I2C slave address for ICM20602 */
static const uint8_t EXPECTED_WHOAMI[] = { 0x12, 0x11 };  /* WHOAMI value for ICM20602 or derivative */
#define DATA_ACCURACY_MASK  ((uint32_t)0x7)

#define DEFAULT_SENSING_PERIOD		50


void init_sensor();
int inv_icm20602_soft_reset();
void do_sensing(int16_t *pAx, int16_t *pAy, int16_t *pAz, int16_t *pGx, int16_t *pGy, int16_t *pGz  );
I2C_TransferReturn_TypeDef i2c_write_reg8(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, int len);
I2C_TransferReturn_TypeDef i2c_read_reg8(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, int len);
static void check_rc(int rc, const char * msg_context);
int inv_icm20602_check_drdy(uint8_t int_status);

#endif /* SRC_SENSOR_H_ */
