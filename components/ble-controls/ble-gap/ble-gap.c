/*
    setup config for gap layer. initialize gap, and
    set dynamic config for gap.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "ble-gap.h"

/* data types*/


/* constants*/
#define GATTS_TABLE_TAG "GATTS_TABLE_DEMO"

/* macro definitions*/


/* static data declarations*/
static const char* device_name = "ROBOT-BLE-BBX";

static uint8_t service_uuid[16] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

static esp_ble_adv_data_t robot_control_ble_adv_data = {
    .set_scan_rsp        = false,
    .include_name        = true,
    .include_txpower     = true,
    .min_interval        = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval        = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
    .appearance          = 0x00,
    .manufacturer_len    = 0,    //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //test_manufacturer,
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = sizeof(service_uuid),
    .p_service_uuid      = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};


esp_ble_adv_params_t robot_control_ble_adv_params = {
    .adv_int_min         = 0x20,
    .adv_int_max         = 0x40,
    .adv_type            = ADV_TYPE_IND,
    .own_addr_type       = BLE_ADDR_TYPE_PUBLIC,
    .channel_map         = ADV_CHNL_ALL,
    .adv_filter_policy   = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

/* private functions protoypes*/
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

/* public functions bodies*/
void ble_gap_init_gap_ble_ctrl(bool security)
{
    if (!security){
        printf("init advertisement data\n");
        esp_ble_gap_config_adv_data(&robot_control_ble_adv_data);
    }
    else {
        //TODO : enable security
    }
}


void ble_gap_init_cb_gap_ctrl()
{
    /*
    TODO : learn about mechanism
    */
    printf("init gap register cb\n");
    esp_ble_gap_register_callback(gap_event_handler);
}

void ble_gap_start_advertise()
{
    esp_ble_gap_start_advertising(&robot_control_ble_adv_params);
}

/* private functions bodies*/

static void start_advertising()
{
    printf("start advertising\n");
    esp_ble_gap_start_advertising(&robot_control_ble_adv_params);
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch(event){
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            start_advertising();
            break;

        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        //advertising start complete event to indicate advertising start successfully or failed
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(GATTS_TABLE_TAG, "Advertising start failed\n");
            }
            break;
        
        default:
            break;
    }
}

