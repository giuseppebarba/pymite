#include "stm32_bluenrg_ble.h"
#include "ble_status.h"
#include "bluenrg_hal_aci.h"
#include "gap.h"
#include "sm.h"
#include "hci.h"
#include "hci_const.h"
#include "osal.h"
#include "bluenrg_gatt_aci.h"
#include "bluenrg_gap_aci.h"
#include "bluenrg_aci_const.h"
#include "bluenrg_UART.h"
#include <string.h>

#define BTLE_MAX_TX_DATA	20
#define BDADDR_SIZE		6
#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11,\
			uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, \
			uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
	uuid_struct[0] = uuid_0; \
	uuid_struct[1] = uuid_1; \
	uuid_struct[2] = uuid_2; \
	uuid_struct[3] = uuid_3; \
	uuid_struct[4] = uuid_4; \
	uuid_struct[5] = uuid_5; \
	uuid_struct[6] = uuid_6; \
	uuid_struct[7] = uuid_7; \
	uuid_struct[8] = uuid_8; \
	uuid_struct[9] = uuid_9; \
	uuid_struct[10] = uuid_10; \
	uuid_struct[11] = uuid_11; \
	uuid_struct[12] = uuid_12; \
	uuid_struct[13] = uuid_13; \
	uuid_struct[14] = uuid_14; \
	uuid_struct[15] = uuid_15; \
}while(0)

#define COPY_UART_SERVICE_UUID(uuid_struct)	COPY_UUID_128(uuid_struct, 0x02, \
						0x36, 0x6e, 0x80, 0xcf, 0x3a, \
						0x11, 0xe1, 0x9a, 0xb4, 0x00, \
						0x02, 0xa5, 0xd5, 0xc5, 0x1b)

#define COPY_UART_TX_UUID(uuid_struct)		COPY_UUID_128(uuid_struct, 0xe2, \
						0x3e, 0x78, 0xa0, 0xcf, 0x4a, \
						0x11, 0xe1, 0x8f, 0xfc, 0x00, \
						0x02, 0xa5, 0xd5, 0xc5, 0x1b)

#define COPY_UART_RX_UUID(uuid_struct)		COPY_UUID_128(uuid_struct, 0x34, \
						0x0a, 0x1b, 0x80, 0xcf, 0x4b, \
						0x11, 0xe1, 0xac, 0x36, 0x00, \
						0x02, 0xa5, 0xd5, 0xc5, 0x1b)

uint16_t UARTServHandle, UART_TX_Handle, UART_RX_Handle;
volatile uint16_t connection_handle = 0;

/**
 * @brief  Add the UART service using a vendor specific profile.
 *
 * @param  None
 * @retval tBleStatus Status
 */
tBleStatus UART_service_add(void)
{
	int ret;
	uint8_t uuid[16];

	COPY_UART_SERVICE_UUID(uuid);
	ret = aci_gatt_add_serv(UUID_TYPE_128,  uuid, PRIMARY_SERVICE, 7,
                          &UARTServHandle);
	if (ret != BLE_STATUS_SUCCESS)
		return PM_RET_ERR;

	COPY_UART_TX_UUID(uuid);
	ret = aci_gatt_add_char(UARTServHandle, UUID_TYPE_128, uuid, 20,
				CHAR_PROP_NOTIFY,
				ATTR_PERMISSION_NONE,
				0,
				16, 0, &UART_TX_Handle);
	if (ret != BLE_STATUS_SUCCESS)
		return PM_RET_ERR;

	COPY_UART_RX_UUID(uuid);
	ret = aci_gatt_add_char(UARTServHandle, UUID_TYPE_128, uuid, 6,
				CHAR_PROP_NOTIFY|CHAR_PROP_READ,
				ATTR_PERMISSION_NONE,
				GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
				16, 0, &UART_RX_Handle);
	if (ret != BLE_STATUS_SUCCESS)
		return PM_RET_ERR;

	return PM_RET_OK;
}

PmReturn_t BTLE_UART_init(void)
{
	uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
	uint8_t SERVER_BDADDR[] = { 0xaa, 0x00, 0x00, 0xE1, 0x80, 0x02 };
	uint8_t bdaddr[BDADDR_SIZE];
	const char *name = "BlueNRG-UART";
	int ret;

		/* Initialize the BlueNRG SPI driver */
	BNRG_SPI_Init();

	/* Initialize the BlueNRG HCI */
	HCI_Init();

	/* Reset BlueNRG hardware */
	BlueNRG_RST();

	Osal_MemCpy(bdaddr, SERVER_BDADDR, sizeof(SERVER_BDADDR));

	ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
					CONFIG_DATA_PUBADDR_LEN, bdaddr);
	if (ret)
		return PM_RET_ERR;

	ret = aci_gatt_init();
	if (ret)
		return PM_RET_ERR;

	ret = aci_gap_init(GAP_PERIPHERAL_ROLE, &service_handle,
					&dev_name_char_handle,
					&appearance_char_handle);
	if (ret != BLE_STATUS_SUCCESS)
		return PM_RET_ERR;

	ret = aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0,
                                   strlen(name), (uint8_t *)name);
	if (ret != BLE_STATUS_SUCCESS)
		return PM_RET_ERR;

	ret = aci_gap_set_auth_requirement(MITM_PROTECTION_REQUIRED,
					OOB_AUTH_DATA_ABSENT,
					NULL,
					7,
					16,
					USE_FIXED_PIN_FOR_PAIRING,
					123456, BONDING);
	if (ret != BLE_STATUS_SUCCESS)
		return PM_RET_ERR;

	ret = UART_service_add();
	if (ret != BLE_STATUS_SUCCESS)
		return PM_RET_ERR;

	/* Set output power level */
	ret = aci_hal_set_tx_power_level(1,4);
	if (ret != BLE_STATUS_SUCCESS)
		return PM_RET_ERR;

	return PM_RET_OK;
}

/**
 * @brief  Send UART chars.
 *
 * @param  Structure containing acceleration value in mg
 * @retval Status
 */
PmReturn_t BTLE_UART_send(const uint8_t *data, uint8_t length)
{
	int len = length, ret, tx_data_len = 0;
	
	while(len > 0) {
		if (len > BTLE_MAX_TX_DATA)
			tx_data_len = BTLE_MAX_TX_DATA;
		else
			tx_data_len = len;

		ret = aci_gatt_update_char_value(UARTServHandle, UART_TX_Handle,
					0, tx_data_len, data);
		if ((ret != BLE_STATUS_SUCCESS) &&
				(ret != BLE_STATUS_INSUFFICIENT_RESOURCES))
			return PM_RET_ERR;

		if (ret != BLE_STATUS_INSUFFICIENT_RESOURCES) {
			len -= tx_data_len;
			if (len > 0)
				data += tx_data_len;
		}
		
	}

	return PM_RET_OK;	
}

/**
 * @brief  Read request callback.
 * @param  uint16_t Handle of the attribute
 * @retval None
 */
void Read_Request_CB(uint16_t handle)
{
	if (handle == (UART_TX_Handle + 1 )) {
	} else if(handle == (UART_RX_Handle + 1)) {
	}

	if(connection_handle != 0)
		aci_gatt_allow_read(connection_handle);
}

/**
 * @brief  This function is called when there is a LE Connection Complete event.
 * @param  uint8_t Address of peer device
 * @param  uint16_t Connection handle
 * @retval None
 */
void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle)
{
	int i;

	connection_handle = handle;

	PRINTF("Connected to device:");
	for(i = 5; i > 0; i--){
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
	PRINTF("Disconnected\n");
}

/**
 * @brief  Callback processing the ACI events.
 * @note   Inside this function each event must be identified and correctly
 *         parsed.
 * @param  void* Pointer to the ACI packet
 * @retval None
 */
void HCI_Event_CB(void *pckt)
{
	hci_uart_pckt *hci_pckt = pckt;
	/* obtain event packet */
	hci_event_pckt *event_pckt = (hci_event_pckt*)hci_pckt->data;
	evt_le_meta_event *evt;
	evt_le_connection_complete *cc;
	evt_blue_aci *blue_evt;

	if(hci_pckt->type != HCI_EVENT_PKT)
		return;

	switch(event_pckt->evt){
	case EVT_DISCONN_COMPLETE:
		GAP_DisconnectionComplete_CB();

		break;

	case EVT_LE_META_EVENT:
		evt = (void *)event_pckt->data;
		if(evt->subevent == EVT_LE_CONN_COMPLETE) {
			cc = (void *)evt->data;
			GAP_ConnectionComplete_CB(cc->peer_bdaddr, cc->handle);
		}

		break;

	case EVT_VENDOR:
		blue_evt = (void*)event_pckt->data;
		if(blue_evt->ecode == EVT_BLUE_GATT_READ_PERMIT_REQ){
			evt_gatt_read_permit_req *pr = (void*)blue_evt->data;
			Read_Request_CB(pr->attr_handle);
		}

		break;
	}
}
