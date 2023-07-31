#ifndef BLE_GAP_H
#define BLE_GAP_H


void ble_gap_init_gap_ble_ctrl(bool enable_security);

void ble_gap_init_cb_gap_ctrl();

void ble_gap_start_advertise();

#endif /* BLE_GAP_H*/