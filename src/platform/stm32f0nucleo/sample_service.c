/**
  ******************************************************************************
  * @file    sample_service.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    04-July-2014
  * @brief   Add a sample service using a vendor specific profile.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "sample_service.h"
#include "connection_config.h"

/* Private variables ---------------------------------------------------------*/
volatile int connected = FALSE;
volatile uint8_t set_connectable = 1;
volatile uint16_t connection_handle = 0;
volatile uint8_t notification_enabled = FALSE;
uint16_t sampleServHandle, TXCharHandle, RXCharHandle;

extern BLE_RoleTypeDef BLE_Role;

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
  do {\
  	uuid_struct.uuid128[0] = uuid_0; uuid_struct.uuid128[1] = uuid_1; uuid_struct.uuid128[2] = uuid_2; uuid_struct.uuid128[3] = uuid_3; \
	uuid_struct.uuid128[4] = uuid_4; uuid_struct.uuid128[5] = uuid_5; uuid_struct.uuid128[6] = uuid_6; uuid_struct.uuid128[7] = uuid_7; \
	uuid_struct.uuid128[8] = uuid_8; uuid_struct.uuid128[9] = uuid_9; uuid_struct.uuid128[10] = uuid_10; uuid_struct.uuid128[11] = uuid_11; \
	uuid_struct.uuid128[12] = uuid_12; uuid_struct.uuid128[13] = uuid_13; uuid_struct.uuid128[14] = uuid_14; uuid_struct.uuid128[15] = uuid_15; \
	}while(0)

/**
 * @brief  Add a sample service using a vendor specific profile
 * @param  None
 * @retval Status
 */
tBleStatus Add_Sample_Service(void)
{
	tBleStatus ret;

	/*
	   UUIDs:
	   D973F2E0-B19E-11E2-9E96-0800200C9A66
	   D973F2E1-B19E-11E2-9E96-0800200C9A66
	   D973F2E2-B19E-11E2-9E96-0800200C9A66
	 */

	const uint8_t service_uuid[16] =
	    { 0x66, 0x9a, 0x0c, 0x20, 0x00, 0x08, 0x96, 0x9e, 0xe2, 0x11, 0x9e,
		0xb1, 0xe0, 0xf2, 0x73, 0xd9
	};
	const uint8_t charUuidTX[16] =
	    { 0x66, 0x9a, 0x0c, 0x20, 0x00, 0x08, 0x96, 0x9e, 0xe2, 0x11, 0x9e,
		0xb1, 0xe1, 0xf2, 0x73, 0xd9
	};
	const uint8_t charUuidRX[16] =
	    { 0x66, 0x9a, 0x0c, 0x20, 0x00, 0x08, 0x96, 0x9e, 0xe2, 0x11, 0x9e,
		0xb1, 0xe2, 0xf2, 0x73, 0xd9
	};

	ret = aci_gatt_add_serv(UUID_TYPE_128, service_uuid, PRIMARY_SERVICE, 7, &sampleServHandle);	/* original is 9?? */
	if (ret != BLE_STATUS_SUCCESS)
		goto fail;

	ret =
	    aci_gatt_add_char(sampleServHandle, UUID_TYPE_128, charUuidTX, 20,
			      CHAR_PROP_NOTIFY, ATTR_PERMISSION_NONE, 0, 16, 1,
			      &TXCharHandle);
	if (ret != BLE_STATUS_SUCCESS)
		goto fail;

	ret =
	    aci_gatt_add_char(sampleServHandle, UUID_TYPE_128, charUuidRX, 20,
			      CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RESP,
			      ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE,
			      16, 1, &RXCharHandle);
	if (ret != BLE_STATUS_SUCCESS)
		goto fail;

	PRINTF
	    ("Sample Service added.\nTX Char Handle %04X, RX Char Handle %04X\n",
	     TXCharHandle, RXCharHandle);
	return BLE_STATUS_SUCCESS;

fail:
	PRINTF("Error while adding Sample Service.\n");
	return BLE_STATUS_ERROR;
}

/**
 * @brief  Make the device connectable
 * @param  None 
 * @retval None
 */
void Make_Connection(void)
{
	tBleStatus ret;

	if (BLE_Role == CLIENT) {
		tBDAddr bdaddr = { 0xaa, 0x00, 0x00, 0xE1, 0x80, 0x02 };
		//tBDAddr bdaddr = {0xfd, 0x00, 0x25, 0xec, 0x02, 0x04}; //[SO] BT address for HRM test

		// Intial value for connection interval 9
		/*
		   Scan_Interval, Scan_Window, Peer_Address_Type, Peer_Address, Own_Address_Type, Conn_Interval_Min, 
		   Conn_Interval_Max, Conn_Latency, Supervision_Timeout, Conn_Len_Min, Conn_Len_Max    
		 */
		ret =
		    aci_gap_create_connection(SCAN_P, SCAN_L, PUBLIC_ADDR,
					      bdaddr, PUBLIC_ADDR, CONN_P1,
					      CONN_P2, 0, SUPERV_TIMEOUT,
					      CONN_L1, CONN_L2);

		if (ret != 0) {
			PRINTF("Error while starting connection.\n");
			Clock_Wait(100);
		}

	} else {

		const char local_name[] =
		    { AD_TYPE_COMPLETE_LOCAL_NAME, 'B', 'l', 'u', 'e', 'N', 'R',
			'G', '_', 'C', 'h', 'a', 't'
		};

		/* disable scan response */
		hci_le_set_scan_resp_data(0, NULL);

		PRINTF("General Discoverable Mode ");
		/*
		   Advertising_Event_Type, Adv_Interval_Min, Adv_Interval_Max, Address_Type, Adv_Filter_Policy,
		   Local_Name_Length, Local_Name, Service_Uuid_Length, Service_Uuid_List, Slave_Conn_Interval_Min,
		   Slave_Conn_Interval_Max
		 */
		ret =
		    aci_gap_set_discoverable(ADV_IND, 0, 0, PUBLIC_ADDR,
					     NO_WHITE_LIST_USE, 13, local_name,
					     0, NULL, 0, 0);
		PRINTF("%d\n", ret);
	}
}

/**
 * @brief  This function is used to receive data related to the sample service
 *         (received over the air from the remote board).
 * @param  data_buffer : pointer to store in received data
 * @param  Nb_bytes : number of bytes to be received
 * @retval None
 */
void receiveData(uint8_t * data_buffer, uint8_t Nb_bytes)
{
	BSP_LED_Toggle(LED2);

	for (int i = 0; i < Nb_bytes; i++) {
		PRINTF("%c", data_buffer[i]);
	}
}

/**
 * @brief  This function is used to send data related to the sample service
 *         (to be sent over the air to the remote board).
 * @param  data_buffer : pointer to data to be sent
 * @param  Nb_bytes : number of bytes to send
 * @retval None
 */
void sendData(uint8_t * data_buffer, uint8_t Nb_bytes)
{
	if (BLE_Role == SERVER) {
		aci_gatt_update_char_value(sampleServHandle, TXCharHandle, 0,
					   Nb_bytes, data_buffer);
	} else {
		aci_gatt_write_without_response(connection_handle,
						RX_HANDLE + 1, Nb_bytes,
						data_buffer);
	}
}

/**
 * @brief  Enable notification
 * @param  None 
 * @retval None
 */
void enableNotification(void)
{
	uint8_t client_char_conf_data[] = { 0x01, 0x00 };	// Enable notifications
	struct timer t;
	Timer_Set(&t, CLOCK_SECOND * 10);

	while (aci_gatt_write_charac_descriptor
	       (connection_handle, TX_HANDLE + 2, 2,
		client_char_conf_data) == BLE_STATUS_NOT_ALLOWED) {
		/* Radio is busy */
		if (Timer_Expired(&t))
			break;
	}
	notification_enabled = TRUE;
}

/**
 * @brief  This function is called when an attribute gets modified
 * @param  handle : handle of the attribute
 * @param  data_length : size of the modified attribute data
 * @param  att_data : pointer to the modified attribute data
 * @retval None
 */
void Attribute_Modified_CB(uint16_t handle, uint8_t data_length,
			   uint8_t * att_data)
{
	if (handle == RXCharHandle + 1) {
		receiveData(att_data, data_length);
	} else if (handle == TXCharHandle + 2) {
		if (att_data[0] == 0x01)
			notification_enabled = TRUE;
	}
}

/**
 * @brief  This function is called when there is a LE Connection Complete event.
 * @param  addr : Address of peer device
 * @param  handle : Connection handle
 * @retval None
 */
void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle)
{
	connected = TRUE;
	connection_handle = handle;

	PRINTF("Connected to device:");
	for (int i = 5; i > 0; i--) {
		PRINTF("%02X-", addr[i]);
	}
	PRINTF("%02X\n", addr[0]);
}

/**
 * @brief  This function is called when the peer device get disconnected.
 * @param  None 
 * @retval None
 */
void GAP_DisconnectionComplete_CB(void)
{
	connected = FALSE;

	PRINTF("Disconnected\n");
	/* Make the device connectable again. */
	set_connectable = TRUE;
	notification_enabled = FALSE;
}

/**
 * @brief  This function is called when there is a notification from the sever.
 * @param  attr_handle Handle of the attribute
 * @param  attr_len    Length of attribute value in the notification
 * @param  attr_value  Attribute value in the notification
 * @retval None
 */
void GATT_Notification_CB(uint16_t attr_handle, uint8_t attr_len,
			  uint8_t * attr_value)
{
	if (attr_handle == TX_HANDLE + 1) {
		receiveData(attr_value, attr_len);
	}
}

/**
 * @brief  This function is called whenever there is an ACI event to be processed.
 * @note   Inside this function each event must be identified and correctly
 *         parsed.
 * @param  pckt  Pointer to the ACI packet
 * @retval None
 */
void HCI_Event_CB(void *pckt)
{
	hci_uart_pckt *hci_pckt = pckt;
	hci_event_pckt *event_pckt = (hci_event_pckt *) hci_pckt->data;

	if (hci_pckt->type != HCI_EVENT_PKT)
		return;

	switch (event_pckt->evt) {

	case EVT_DISCONN_COMPLETE:
		{
			GAP_DisconnectionComplete_CB();
		}
		break;

	case EVT_LE_META_EVENT:
		{
			evt_le_meta_event *evt = (void *)event_pckt->data;

			switch (evt->subevent) {
			case EVT_LE_CONN_COMPLETE:
				{
					evt_le_connection_complete *cc =
					    (void *)evt->data;
					GAP_ConnectionComplete_CB
					    (cc->peer_bdaddr, cc->handle);
				}
				break;
			}
		}
		break;

	case EVT_VENDOR:
		{
			evt_blue_aci *blue_evt = (void *)event_pckt->data;
			switch (blue_evt->ecode) {

			case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:
				{
					evt_gatt_attr_modified *evt =
					    (evt_gatt_attr_modified *)
					    blue_evt->data;

					Attribute_Modified_CB(evt->attr_handle,
							      evt->data_length,
							      evt->att_data);
				}
				break;
			case EVT_BLUE_GATT_NOTIFICATION:
				{
					evt_gatt_attr_notification *evt =
					    (evt_gatt_attr_notification *)
					    blue_evt->data;
					GATT_Notification_CB(evt->attr_handle,
							     evt->data_length -
							     2,
							     evt->attr_value);
				}
				break;
			}
		}
		break;
	}
}

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
