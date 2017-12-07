#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "bglib_txrx.h"
#include "key_input.h"
#include "user_usart.h"
#include "em_int.h"

bool bDfuFlag = false;
bt_hid_t btHid;


extern struct dumo_cmd_packet bglib_temp_msg;
extern void (*bglib_output)(uint8 len1,uint8* data1,uint16 len2,uint8* data2);
/**
 * Function called when a message needs to be written to the serial port.
 * @param msg_len Length of the message.
 * @param msg_data Message data, including the header.
 * @param data_len Optional variable data length.
 * @param data Optional variable data.
 */
void on_message_send(uint8 msg_len, uint8* msg_data, uint16 data_len, uint8* data)
{
    /** Variable for storing function return values. */
    int ret;


	ret = USART0_SEND(msg_data, msg_len);
	if (ret < 0)
	{
		return;
	}

	if (data_len && data)
	{
		ret = USART0_SEND(data, data_len);
		if (ret < 0)
		{
			return;
		}
	}
}

int handle_bt_rx(void)
{
	struct dumo_cmd_packet* pck;
	int ret;
	static int led_count=1, led=1;
	/** Buffer for storing data from the serial port. */
	/** Length of message payload data. */
	uint16_t msg_length;
	static unsigned char buffer[BGLIB_MSG_MAXLEN];
	//unsigned char fwbuffer[250];
	uint8_t keyVal = KEYBOARD_NONE;
	uint8_t key_buffer[HID_KEY_BUFFER_LEN] = {0,};

	/*
	* Read enough data from serial port for the BGAPI message header.
	*/
	ret = get_usart_rx_data_blocking(buffer, BGLIB_MSG_HEADER_LEN);
	if (ret < 0)
	{
		//LOG_ERR("rx , ret: %d\n", ret);
		return -1;
	}
		//printf("%02x%02x%02x%02x\n", buffer[0], buffer[1], buffer[2], buffer[3]);
	msg_length = BGLIB_MSG_LEN(buffer);

	/**
	 * Read the payload data if required and store it after the header.
	 */
	if (msg_length) {
		ret = get_usart_rx_data_blocking(&buffer[BGLIB_MSG_HEADER_LEN], msg_length);
		if (ret < 0) {
			//LOG_ERR("rx, ret: %d\n", ret);
			return -1;
		}
	}

	static char triger = 0;
	pck = BGLIB_MSG(buffer);

	switch (BGLIB_MSG_ID(buffer)) {

	case dumo_rsp_system_hello_id:
		//led_count = 1; led=1; 	LED_blink (led, led_count);
		if (!triger)
		{
			LOG_DBG("dumo_rsp_system_hello_id\n");
			dumo_cmd_system_reset(0);
			triger = 1;
		}

		break;
	case dumo_evt_system_boot_id:
		//led_count = 3;led=1;		LED_blink (led, led_count);

		LOG_DBG("dumo_evt_system_boot_id()\n");
		btHid.hid_endpoint = -1;
		btHid.hid_connected = false;
		memset(btHid.report, 0x00, sizeof(btHid.report));

		break;

	case dumo_evt_system_initialized_id:
		//led_count = 4;led=1;	LED_blink (led, led_count);
		LOG_DBG("dumo_evt_system_initialized_id()\n");
		dumo_cmd_system_set_local_name(strlen(KETI_HID_KEYBOARD_NAME),KETI_HID_KEYBOARD_NAME);

		break;

	case dumo_rsp_system_set_local_name_id:
//		led_count = 5;led=1;	LED_blink (led, led_count);
		LOG_DBG("dumo_rsp_system_set_local_name_id, result=%04x\n",
				pck->rsp_system_set_local_name.result);
		dumo_cmd_system_set_class_of_device(DEVICE_CLASS_HID_KEYBOARD);

		break;

	case dumo_rsp_system_set_class_of_device_id:
	//	led_count = 1;led=2;	LED_blink (led, led_count);
		LOG_DBG("dumo_rsp_system_set_class_of_device_id, result=%04x\n", pck->rsp_system_set_class_of_device.result);
		dumo_cmd_sm_configure(MITM_REQUIRED_FLAG_NOT_REQUIRED,IO_CAPABILITY_NOINPUT_NOOUTPUT);

		break;

	case dumo_rsp_sm_configure_id:
		//led_count = 2;led=2;	LED_blink (led, led_count);
		LOG_DBG("dumo_rsp_sm_configure_id, result=%04x\n",
				pck->rsp_sm_configure.result);
		dumo_cmd_sm_set_bondable_mode(BONDABLE_MODE_ALLOWED);
		break;

	case dumo_rsp_sm_set_bondable_mode_id:
		//led_count = 3;led=2;	LED_blink (led, led_count);
		LOG_DBG("dumo_rsp_sm_set_bondable_mode_id, result=%04x\n",
				pck->rsp_sm_set_bondable_mode.result);
		dumo_cmd_bt_hid_start_server(SDP_ID_HID_KEYBOARD,DEFAULT_STREAMING_DESTINATION);
		break;

	case dumo_rsp_bt_hid_start_server_id:
//		led_count = 4;led=2;	LED_blink (led, led_count);
		LOG_DBG("dumo_rsp_bt_hid_start_server_id, result=%04x\n",
				pck->rsp_bt_hid_start_server.result);
		dumo_cmd_bt_gap_set_mode(GAP_CONNECTABLE, GAP_DISCOVERABLE, GAP_DISCOVERMODE_GENERIC);
		break;

	case dumo_rsp_bt_gap_set_mode_id:
	//	led_count = 5;led=2;	LED_blink (led, led_count);
		LOG_DBG("dumo_rsp_bt_gap_set_mode_id, result=%04x\n", pck->rsp_bt_gap_set_mode.result);
		break;

	case dumo_rsp_hardware_set_soft_timer_id :
	//	led_count = 1;led=2;	LED_blink (led, led_count);
		LOG_DBG("dumo_rsp_hardware_set_soft_timer_id, result=%04x\n", pck->rsp_hardware_set_soft_timer.result);
		break;

	case dumo_evt_endpoint_status_id:
	//	led_count = 1; led=3;	LED_blink (led, led_count);
		LOG_DBG(
				"dumo_evt_endpoint_status_id, ep = %02x, type=%08x, dep=%02x, flag=%02x\n",
				pck->evt_endpoint_status.endpoint,
				pck->evt_endpoint_status.type,
				pck->evt_endpoint_status.destination_endpoint,
				pck->evt_endpoint_status.flags);

		if (endpoint_hid
				== pck->evt_endpoint_status.type&& pck->evt_endpoint_status.flags & ENDPOINT_FLAG_ACTIVE) {
			led_count = 2; led=1;	LED_blink (led, led_count);

			btHid.hid_endpoint = pck->evt_endpoint_status.endpoint;
			btHid.hid_connected = true;
			dumo_cmd_hardware_set_soft_timer(TIMER_VALUE_MS_GET_SENSOR_DATA, TIMER_ID_GET_SENSOR_DATA, TIMER_MODE_REPEATING);
		}

		break;

	case dumo_evt_bt_hid_opened_id:
	//	led_count = 3; led=3;	LED_blink (led, led_count);
		LOG_DBG("dumo_evt_bt_hid_opened_id, ep = %02x, addr=",
				pck->evt_bt_hid_opened.endpoint);
		memcpy(btHid.address, pck->evt_bt_hid_opened.address.addr, sizeof(btHid.address));
		break;

	case dumo_evt_endpoint_closing_id:
	//	led_count = 4; led=3;	LED_blink (led, led_count);
		LOG_DBG("dumo_evt_endpoint_closing_id, ep=%02x, reason=%04x\n",
				pck->evt_endpoint_closing.endpoint,
				pck->evt_endpoint_closing.reason);
		dumo_cmd_endpoint_close(pck->evt_endpoint_closing.endpoint);
		if(pck->evt_endpoint_closing.endpoint == btHid.hid_endpoint) {
			dumo_cmd_hardware_set_soft_timer(0, TIMER_ID_GET_SENSOR_DATA, TIMER_MODE_SINGLESHOT);
		}


		break;

	case dumo_rsp_endpoint_close_id:
	//	led_count = 5; led=3;	LED_blink (led, led_count);
		LOG_DBG("dumo_rsp_endpoint_close_id, ep=%02x, result=%04x\n",
				pck->rsp_endpoint_close.endpoint,
				pck->rsp_endpoint_close.result);
		break;

	case dumo_evt_bt_hid_state_changed_id:
		//led_count = 6; led=3;	LED_blink (led, led_count);


		if (btHid.hid_endpoint == pck->evt_bt_hid_state_changed.endpoint &&
				bt_hid_state_virtual_cable_unplug == pck->evt_bt_hid_state_changed.state) {
			btHid.hid_endpoint = -1;
			btHid.hid_connected = false;
		}

		break;

	case dumo_evt_sm_confirm_passkey_id:
		//led_count = 7; led=3;	LED_blink (led, led_count);


		LOG_DBG(
				"dumo_evt_sm_confirm_passkey_id, connection=%02x, passkey=%08x\n",
				pck->evt_sm_confirm_passkey.connection,
				pck->evt_sm_confirm_passkey.passkey);
		dumo_cmd_sm_passkey_confirm(pck->evt_sm_confirm_passkey.connection,
				SM_CONFIRM_ACCEPT)
		;

		break;

	case dumo_rsp_sm_passkey_confirm_id:
		//led_count = 8; led=3;	LED_blink (led, led_count);
		LOG_DBG("dumo_rsp_sm_passkey_confirm_id, result=%04x\n",
				pck->rsp_sm_passkey_confirm.result);

		break;

	case dumo_evt_sm_passkey_display_id:
		//led_count = 6; led=1;	LED_blink (led, led_count);
		LOG_DBG(
				"dumo_evt_sm_passkey_display_id, connection=%02x, passkey=%08x\n",
				pck->evt_sm_passkey_display.connection,
				pck->evt_sm_passkey_display.passkey);

		break;

	case dumo_evt_bt_connection_opened_id:
		//led_count = 7; led=1;	LED_blink (led, led_count);
		LOG_DBG("dumo_evt_bt_connection_opened_id, addr=");
		//print_address(pck->evt_bt_connection_opened.address);
		LOG_DBG(", master=%02x, connection=%02x, bonding=%02x\n",
				pck->evt_bt_connection_opened.master,
				pck->evt_bt_connection_opened.connection,
				pck->evt_bt_connection_opened.bonding);


		break;

	case dumo_evt_sm_bonded_id:
		//led_count = 8; led=1;	LED_blink (led, led_count);
		LOG_DBG("dumo_evt_sm_bonded_id, connection=%02x, bonding=%02x\n",
				pck->evt_sm_bonded.connection, pck->evt_sm_bonded.bonding);
		break;

	case dumo_evt_dfu_boot_id:
		//led_count = 6; led=2;	LED_blink (led, led_count);
		LOG_DBG("DFU version = %d\n", pck->evt_dfu_boot.version);
		if (bDfuFlag) {
			dumo_cmd_dfu_flash_set_address(0);
		} else {
			dumo_cmd_system_reset(0);
		}
		break;

#if 0
		/*
		 * DFU Function comming soon
		 */
		case dumo_rsp_dfu_flash_set_address_id :
		printf("set addr result = %04x\n", pck->rsp_dfu_flash_set_address.result);
		if (fopen_s(&fp, "test.bin", "rb") != 0) {
			printf("fopen fail, err=%d(%s)\n", errno, strerror(errno));
		}
		printf("File Open\n");
		size = fread(fwbuffer, sizeof(char), sizeof(fwbuffer), fp);
		if (size) {
			dumo_cmd_dfu_flash_upload(size, fwbuffer);
			cnt++;
			printf("File Uploaded, size=%d, cnt=%d\n", size, cnt);

		}
		break;

		case dumo_rsp_dfu_flash_upload_id :
		size = fread(fwbuffer, sizeof(char), sizeof(fwbuffer), fp);
		if (size) {
			dumo_cmd_dfu_flash_upload(size, fwbuffer);
			cnt++;
			//printf("File Uploaded, size=%d, cnt=%d\n", size, cnt);
			printf(".");

		}
		else {
			dumo_cmd_dfu_flash_upload_finish();
			bDfuFlag = false;
			fclose(fp);
			cnt = 0;
		}
		break;

		case dumo_rsp_dfu_flash_upload_finish_id :
		printf("\nflash upload finished, res=%04x\n", pck->rsp_dfu_flash_upload_finish.result);
		dumo_cmd_dfu_reset(0);
		break;
#endif
		case dumo_evt_hardware_soft_timer_id :
		//	led_count = 7; led=2;	LED_blink (led, led_count);
			switch (pck->evt_hardware_soft_timer.timer_id) {
			case TIMER_ID_GET_SENSOR_DATA:
				keyVal = KEYBOARD_NONE;
				keyVal = get_key_value();
				if (keyVal != KEYBOARD_NONE) {
					/* Key Press Event */
					key_buffer[HID_BUF_KEY_OFFSET] = keyVal;
					dumo_cmd_bt_hid_send_input_report(btHid.hid_endpoint, HID_REPORT_ID_KEYBOARD, HID_KEY_BUFFER_LEN, key_buffer);
					/* Key Release Event */
					key_buffer[HID_BUF_KEY_OFFSET] = KEYBOARD_NONE;
					dumo_cmd_bt_hid_send_input_report(btHid.hid_endpoint, HID_REPORT_ID_KEYBOARD, HID_KEY_BUFFER_LEN, key_buffer);
				}
				break;
			}
			break;
			case dumo_rsp_bt_hid_send_input_report_id :
			//	led_count = 8; led=2;	LED_blink (led, led_count);
				LOG_DBG("dumo_rsp_bt_hid_send_input_report_id, result=%04x\n", pck->rsp_bt_hid_send_input_report.result);


				break;

	}
	return 0;
}

