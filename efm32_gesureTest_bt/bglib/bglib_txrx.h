#ifndef _BGLIB_RX_H_
#define _BGLIB_RX_H_

#include "dumo_bglib.h"

#define KETI_HID_KEYBOARD_NAME			"KETI-Test"
#define DEVICE_CLASS_HID_KEYBOARD		0x540

#define MITM_REQUIRED_FLAG_NOT_REQUIRED	0
#define MITM_REQUIRED_FLAG_REQUIRED		1

#define IO_CAPABILITY_DISPLAY_ONLY		0
#define IO_CAPABILITY_DISPLAY_YESNO		1
#define IO_CAPABILITY_KEYBOARD_ONLY		2
#define IO_CAPABILITY_NOINPUT_NOOUTPUT	3
#define IO_CAPABILITY_KEYBOARD_DISPLAY	4

#define BONDABLE_MODE_NOT_ACCEPTED		0
#define BONDABLE_MODE_ALLOWED			1

#define SDP_ID_HID_KEYBOARD				5
#define DEFAULT_STREAMING_DESTINATION	0

#define	GAP_NOTCONNECTABLE				0
#define	GAP_CONNECTABLE					1
#define	GAP_NOT_DISCOVERABLE			0
#define	GAP_DISCOVERABLE				1
#define GAP_DISCOVERMODE_GENERIC		0
#define GAP_DISCOVERMODE_LIMITED		1

#define SM_CONFIRM_REJECT				0
#define SM_CONFIRM_ACCEPT				1

#define TIMER_ID_GET_SENSOR_DATA		0
#define TIMER_MODE_REPEATING			0
#define TIMER_MODE_SINGLESHOT			1
#define TIMER_VALUE_MS_GET_SENSOR_DATA	50

#define HID_KEY_BUFFER_LEN				8
#define HID_BUF_KEY_OFFSET				2
#define HID_REPORT_ID_KEYBOARD			1

typedef struct bt_hid_st {
	int8_t hid_endpoint;
	bool hid_connected;
	uint8_t report[8];
	uint8_t address[6];
}bt_hid_t;

void on_message_send(uint8 msg_len, uint8* msg_data, uint16 data_len, uint8* data);
int handle_bt_rx(void);
void print_address(bd_addr address);
#endif
