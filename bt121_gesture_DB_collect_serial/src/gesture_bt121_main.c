#include "em_device.h"
#include "em_chip.h"
#include "em_gpio.h"

#include "dumo_bglib.h"
#include "bglib_txrx.h"
#include "key_input.h"

#include "InitDevice.h"
#include "user_usart.h"

BGLIB_DEFINE();


#define LED_TOGGLE()	GPIO_PinOutToggle(PE12_PORT, PE12_PIN)
int main(void)
{
	/* Chip errata */
	CHIP_Init();

	/* Init */
	enter_DefaultMode_from_RESET();
	init_user_delay();
	/* Delay for SWD Flashing */
	Delay(3000);

    BGLIB_INITIALIZE(on_message_send);
	init_sensor();
	init_user_usart();

	X_CALM = X_INIT;
	Y_CALM = Y_INIT;
	Z_CALM = Z_INIT;

#if 0
	dumo_cmd_system_hello();


	/* Infinite loop */
	while (1) {
		handle_bt_rx();
		//Delay(10);
	}
#else
	/* Infinite loop */
	while (1) {
		int16_t ax=0, ay=0, az=0, gx=0, gy=0, gz =0;
		LED_TOGGLE();
		do_sensing(&ax, &ay, &az, &gx, &gy, &gz);

		//get_key_value();

		//USART1_SEND ("1234 5432", 9);

		get_key_value_from_gesture();


		Delay(50);
	}
#endif
}
