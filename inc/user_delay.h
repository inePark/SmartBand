/*
 * user_delay.h
 *
 *  Created on: 2017. 1. 23.
 *      Author: netbugger
 */

#ifndef SRC_USER_DELAY_H_
#define SRC_USER_DELAY_H_

#include <stdint.h>

void Delay(uint32_t dlyTicks);
void SysTick_Handler(void);
void init_user_delay();

#endif /* SRC_USER_DELAY_H_ */
