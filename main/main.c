#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"


#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "sdkconfig.h"

#include "../components/ble-controls/ble-gap/ble-gap.h"
#include "../components/ble-controls/ble-gatts/ble-gatts.h"

void app_main(void)
{
    esp_err_t ret;

    ret  = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    ret = esp_bt_controller_init(&bt_cfg);

    if (ret){
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);

    if (ret){
        return;
    }


    ret = esp_bluedroid_init();

    ret = esp_bluedroid_enable();

    ble_gatts_init_gatts_cb();
    ble_gap_init_cb_gap_ctrl();

}
