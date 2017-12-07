#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "user_usart.h"
#include "bglib_txrx.h"
#include "key_input.h"

bool bDfuFlag = false;
bt_hid_t btHid;

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

#if DEBUG
    printf("<< on_message_send() msg_Data:%d, msg_len%d, data_len:%d, data:%d >>\n", msg_data, msg_len, data_len, data);
#endif /* DEBUG */

	//printf("msg snd, %d\n", msg_len);

	ret = USART1_SEND(msg_data, msg_len);
	if (ret < 0)
	{
		return;
	}

//	printf("data snd, %d\n", data_len);
	if (data_len && data)
	{
		ret = USART1_SEND(data, data_len);
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

	pck = BGLIB_MSG(buffer);

	switch (BGLIB_MSG_ID(buffer)) {

	case dumo_rsp_system_hello_id:
		LOG_DBG("dumo_rsp_system_hello_id\n");
		dumo_cmd_system_reset(0);
		break;
	case dumo_evt_system_boot_id:
		LOG_DBG("dumo_evt_system_boot_id()\n");
		btHid.hid_endpoint = -1;
		btHid.hid_connected = false;
		memset(btHid.report, 0x00, sizeof(btHid.report));
		break;

	case dumo_evt_system_initialized_id:
		LOG_DBG("dumo_evt_system_initialized_id()\n");
		dumo_cmd_system_set_local_name(strlen(KETI_HID_KEYBOARD_NAME),
				KETI_HID_KEYBOARD_NAME)
		;
		break;

	case dumo_rsp_system_set_local_name_id:
		LOG_DBG("dumo_rsp_system_set_local_name_id, result=%04x\n",
				pck->rsp_system_set_local_name.result);
		dumo_cmd_system_set_class_of_device(DEVICE_CLASS_HID_KEYBOARD)
		;
		break;

	case dumo_rsp_system_set_class_of_device_id:
		LOG_DBG("dumo_rsp_system_set_class_of_device_id, result=%04x\n",
				pck->rsp_system_set_class_of_device.result);
		dumo_cmd_sm_configure(MITM_REQUIRED_FLAG_NOT_REQUIRED,
				IO_CAPABILITY_NOINPUT_NOOUTPUT)
		;
		break;

	case dumo_rsp_sm_configure_id:
		LOG_DBG("dumo_rsp_sm_configure_id, result=%04x\n",
				pck->rsp_sm_configure.result);
		dumo_cmd_sm_set_bondable_mode(BONDABLE_MODE_ALLOWED)
		;
		break;

	case dumo_rsp_sm_set_bondable_mode_id:
		LOG_DBG("dumo_rsp_sm_set_bondable_mode_id, result=%04x\n",
				pck->rsp_sm_set_bondable_mode.result);
		dumo_cmd_bt_hid_start_server(SDP_ID_HID_KEYBOARD,
				DEFAULT_STREAMING_DESTINATION)
		;
		break;

	case dumo_rsp_bt_hid_start_server_id:
		LOG_DBG("dumo_rsp_bt_hid_start_server_id, result=%04x\n",
				pck->rsp_bt_hid_start_server.result);
		dumo_cmd_bt_gap_set_mode(GAP_CONNECTABLE, GAP_DISCOVERABLE, GAP_DISCOVERMODE_GENERIC);
		break;

	case dumo_rsp_bt_gap_set_mode_id:
		LOG_DBG("dumo_rsp_bt_gap_set_mode_id, result=%04x\n", pck->rsp_bt_gap_set_mode.result);
		break;

	case dumo_rsp_hardware_set_soft_timer_id :
		LOG_DBG("dumo_rsp_hardware_set_soft_timer_id, result=%04x\n", pck->rsp_hardware_set_soft_timer.result);
		break;

	case dumo_evt_endpoint_status_id:
		LOG_DBG(
				"dumo_evt_endpoint_status_id, ep = %02x, type=%08x, dep=%02x, flag=%02x\n",
				pck->evt_endpoint_status.endpoint,
				pck->evt_endpoint_status.type,
				pck->evt_endpoint_status.destination_endpoint,
				pck->evt_endpoint_status.flags);

		if (endpoint_hid
				== pck->evt_endpoint_status.type&& pck->evt_endpoint_status.flags & ENDPOINT_FLAG_ACTIVE) {
			btHid.hid_endpoint = pck->evt_endpoint_status.endpoint;
			btHid.hid_connected = true;
			dumo_cmd_hardware_set_soft_timer(TIMER_VALUE_MS_GET_SENSOR_DATA, TIMER_ID_GET_SENSOR_DATA, TIMER_MODE_REPEATING);
		}
		break;

	case dumo_evt_bt_hid_opened_id:
		LOG_DBG("dumo_evt_bt_hid_opened_id, ep = %02x, addr=",
				pck->evt_bt_hid_opened.endpoint);
		print_address(pck->evt_bt_hid_opened.address);
		LOG_NOR("\n");
		memcpy(btHid.address, pck->evt_bt_hid_opened.address.addr,
				sizeof(btHid.address));
		break;

	case dumo_evt_endpoint_closing_id:
		LOG_DBG("dumo_evt_endpoint_closing_id, ep=%02x, reason=%04x\n",
				pck->evt_endpoint_closing.endpoint,
				pck->evt_endpoint_closing.reason);
		dumo_cmd_endpoint_close(pck->evt_endpoint_closing.endpoint);
		if(pck->evt_endpoint_closing.endpoint == btHid.hid_endpoint) {
			dumo_cmd_hardware_set_soft_timer(0, TIMER_ID_GET_SENSOR_DATA, TIMER_MODE_SINGLESHOT);
		}
		break;

	case dumo_rsp_endpoint_close_id:
		LOG_DBG("dumo_rsp_endpoint_close_id, ep=%02x, result=%04x\n",
				pck->rsp_endpoint_close.endpoint,
				pck->rsp_endpoint_close.result);
		break;

	case dumo_evt_bt_hid_state_changed_id:
		LOG_DBG("dumo_evt_bt_hid_state_changed_id, ep=%02x, state =%02x\n",
				pck->evt_bt_hid_state_changed.endpoint,
				pck->evt_bt_hid_state_changed.state);
		if (btHid.hid_endpoint == pck->evt_bt_hid_state_changed.endpoint
				&& bt_hid_state_virtual_cable_unplug
						== pck->evt_bt_hid_state_changed.state) {
			dumo_cmd_endpoint_close(pck->evt_endpoint_closing.endpoint);
			dumo_cmd_hardware_set_soft_timer(0, TIMER_ID_GET_SENSOR_DATA, TIMER_MODE_SINGLESHOT);
			btHid.hid_endpoint = -1;
			btHid.hid_connected = false;
		}
		break;

	case dumo_evt_sm_confirm_passkey_id:
		LOG_DBG(
				"dumo_evt_sm_confirm_passkey_id, connection=%02x, passkey=%08x\n",
				pck->evt_sm_confirm_passkey.connection,
				pck->evt_sm_confirm_passkey.passkey);
		dumo_cmd_sm_passkey_confirm(pck->evt_sm_confirm_passkey.connection,
				SM_CONFIRM_ACCEPT)
		;
		break;

	case dumo_rsp_sm_passkey_confirm_id:
		LOG_DBG("dumo_rsp_sm_passkey_confirm_id, result=%04x\n",
				pck->rsp_sm_passkey_confirm.result);
		break;

	case dumo_evt_sm_passkey_display_id:
		LOG_DBG(
				"dumo_evt_sm_passkey_display_id, connection=%02x, passkey=%08x\n",
				pck->evt_sm_passkey_display.connection,
				pck->evt_sm_passkey_display.passkey);
		break;

	case dumo_evt_bt_connection_opened_id:
		LOG_DBG("dumo_evt_bt_connection_opened_id, addr=");
		print_address(pck->evt_bt_connection_opened.address);
		LOG_DBG(", master=%02x, connection=%02x, bonding=%02x\n",
				pck->evt_bt_connection_opened.master,
				pck->evt_bt_connection_opened.connection,
				pck->evt_bt_connection_opened.bonding);
		break;

	case dumo_evt_sm_bonded_id:
		LOG_DBG("dumo_evt_sm_bonded_id, connection=%02x, bonding=%02x\n",
				pck->evt_sm_bonded.connection, pck->evt_sm_bonded.bonding);
		break;

	case dumo_evt_dfu_boot_id:
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
			switch (pck->evt_hardware_soft_timer.timer_id) {
			case TIMER_ID_GET_SENSOR_DATA:
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
				LOG_DBG("dumo_rsp_bt_hid_send_input_report_id, result=%04x\n",
								pck->rsp_bt_hid_send_input_report.result);
				break;

	}
	return 0;
}

void print_address(bd_addr address)
{
	int i;
	for (i = 5; i >= 0; i--)
	{
		printf("%02x", address.addr[i]);
		if (i > 0)
			printf(":");
	}

}
