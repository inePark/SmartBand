/*
 * user_delay.c
 *
 *  Created on: 2017. 1. 18.
 *      Author: netbugger
 */

#include "em_cmu.h"
#include "user_delay.h"

#if 1
volatile uint32_t msTicks; /* counts 1ms timeTicks */

void Delay(uint32_t dlyTicks)
{
	uint32_t curTicks;

	curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks);
}

void SysTick_Handler(void)
{
	msTicks++; /* increment counter necessary in Delay()*/
}

void init_user_delay()
{
	/* Setup SysTick Timer for 1 msec interrupts  */
	if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1) ;
}
#endif
