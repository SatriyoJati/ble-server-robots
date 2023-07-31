/*
    setup gatts layer, initialize gap, set dynamic config
    for gap

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
#include "ble-gatts.h"
#include <string.h>

/* data types*/
typedef struct gatts_profile {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
}gatts_profile;




/* constants*/


/* macro definitions*/
#define PROFILE_NUM 1
#define PROFILE_A_APP_ID 0
#define PROFILE_B_APP_ID 1


/* static data declarations*/

static uint8_t char_demo[] = {0x05};

esp_attr_value_t gatts_demo_char_val = 
{
    .attr_max_len = 0x40,
    .attr_len = sizeof(char_demo),
    .attr_value = char_demo,
};
/* private functions prototypes*/
static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event_t, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t * param);

static void setup_primary_service_id(gatts_profile* gatts_profile_inst_param , uint8_t index);

static void setup_characterisctic_id(gatts_profile* gatts_profile_inst_param, uint8_t index, esp_ble_gatts_cb_param_t *param);

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);



static gatts_profile gatts_profile_inst[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gatts_cb = gatts_profile_a_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,
    },
};


/* public functions bodies*/

void ble_gatts_init_gatts_cb()
{   
    esp_err_t ret;
    printf("init gatts : registering app and gatts event handler..\n");
    esp_ble_gatts_register_callback(gatts_event_handler);
    ret = esp_ble_gatts_app_register(PROFILE_A_APP_ID);
    printf("register status : %d\n", ret);
}

/* private functions bodies*/

static void setup_primary_service_id(gatts_profile* gatts_profile_inst_param , uint8_t index)
{
    gatts_profile_inst_param[index].service_id.is_primary = true;
    gatts_profile_inst_param[index].service_id.id.inst_id = 0x00;
    gatts_profile_inst_param[index].service_id.id.uuid.len = ESP_UUID_LEN_16;
    gatts_profile_inst_param[index].service_id.id.uuid.uuid.uuid16 = 0x00FF;
}

static void setup_characterisctic_id(gatts_profile* gatts_profile_inst_param, uint8_t index , esp_ble_gatts_cb_param_t *param)
{
    esp_gatt_char_prop_t a_property = 0;
    gatts_profile_inst_param[index].service_handle = param->create.service_handle;
    gatts_profile_inst_param[index].char_uuid.len = ESP_UUID_LEN_16;
    gatts_profile_inst_param[index].char_uuid.uuid.uuid16 = 0xFF01;

    esp_ble_gatts_start_service(gatts_profile_inst_param[index].service_handle);

    a_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
    esp_err_t ret = esp_ble_gatts_add_char(gatts_profile_inst_param[index].service_handle,
                                            &gatts_profile_inst_param[index].char_uuid,
                                            ESP_GATT_PERM_READ,
                                            a_property,
                                            &gatts_demo_char_val,
                                            NULL);
    printf("add char status : %d", ret);

}


static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gatts_profile_inst[param->reg.app_id].gatts_if = gatts_if;
        } else {
            return;
        }
    }

    gatts_profile_inst[0].gatts_cb(event, gatts_if, param);
}

static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event , esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t * param){
    switch (event){
        case ESP_GATTS_REG_EVT:
            printf("gatt service registered\n");
            setup_primary_service_id(&gatts_profile_inst[0] , 0);
            esp_ble_gap_set_device_name("PACHINKO");
            ble_gap_init_gap_ble_ctrl(false);

            esp_ble_gatts_create_service(gatts_if, &gatts_profile_inst[0].service_id, 4);
            break;

        
        case ESP_GATTS_CREATE_EVT:
            printf("gatt service created\n");
            gatts_profile_inst[0].service_handle = param->create.service_handle;
            setup_characterisctic_id(gatts_profile_inst, 0, param );
            break;

        case ESP_GATTS_ADD_CHAR_EVT:
            printf("characteristic added\n");

            gatts_profile_inst[PROFILE_A_APP_ID].char_handle = param->add_char.attr_handle;
            gatts_profile_inst[PROFILE_A_APP_ID].descr_uuid.len = ESP_UUID_LEN_16;
            gatts_profile_inst[PROFILE_A_APP_ID].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
            esp_err_t add_descr_ret = esp_ble_gatts_add_char_descr(
                gatts_profile_inst[0].service_handle,
                &gatts_profile_inst[0].char_uuid,
                ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                NULL,
                NULL
            );
            printf("adding descriptor result : %d\n", add_descr_ret);
            break;

        case ESP_GATTS_CONNECT_EVT :
            printf("connected!\n") ;
            printf("central device connected : %s\n", param->connect.remote_bda);
            esp_ble_conn_update_params_t conn_params = {0};
            memcpy(conn_params.bda , param->connect.remote_bda, sizeof(esp_bd_addr_t));

            conn_params.latency = 0;
            conn_params.max_int = 0x30;
            conn_params.min_int = 0x10;
            conn_params.timeout = 400;
            gatts_profile_inst[0].conn_id = param->connect.conn_id;
            esp_ble_gap_update_conn_params(&conn_params);
            break;
        
        case ESP_GATTS_READ_EVT:
            printf("GATT_READ_EVT, conn_id %d, trans_id %" PRIu32 ", handle %d", param->read.conn_id, param->read.trans_id, param->read.handle);
            esp_gatt_rsp_t rsp;
            memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
            rsp.attr_value.handle = param->read.handle;
            rsp.attr_value.len = 4;
            rsp.attr_value.value[0] = 0xde;
            rsp.attr_value.value[1] = 0xed;
            rsp.attr_value.value[2] = 0xbe;
            rsp.attr_value.value[3] = 0xef;
            esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                        ESP_GATT_OK, &rsp);
            break;
        


        case ESP_GATTS_DISCONNECT_EVT:
            ble_gap_start_advertise();
            printf("disconnected!\n");
            break;
        case ESP_GATTS_OPEN_EVT:
        case ESP_GATTS_CANCEL_OPEN_EVT:
        case ESP_GATTS_CLOSE_EVT:
        case ESP_GATTS_LISTEN_EVT:
        case ESP_GATTS_CONGEST_EVT:
        default:
            break;
        }  
}


