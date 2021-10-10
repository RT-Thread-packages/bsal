/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-09-10     WaterFishJ   the first version
 */

#ifndef __BSAL_SRV_HRS_H__
#define __BSAL_SRV_HRS_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdbool.h>
#include "bsal.h"
#define GATT_UUID_HEART_RATE                       BSAL_GATT_SERVICE_HEART_RATE
#define GATT_UUID_HRS_MEASUREMENT                  BSAL_UUID_CHAR_HEART_RATE_MEASUREMENT
#define GATT_UUID_CHAR_BODY_SENSOR_LOCATION        BSAL_UUID_CHAR_BODY_SENSOR_LOCATION
#define GATT_UUID_SERVICE_DEVICE_INFORMATION       BSAL_GATT_SERVICE_DEVICE_INFORMATION
#define GATT_UUID_CHAR_MANUFACTURER_NAME_STRING    BSAL_UUID_CHAR_MANUFACTURER_NAME_STRING
#define GATT_UUID_CHAR_MODEL_NUMBER_STRING         BSAL_UUID_CHAR_MODEL_NUMBER_STRING


#define GATT_SVC_BODY_SENSOR_LOCATION_READ_INDEX    5
#define GATT_SVC_HRS_MEASUREMENT_CHAR_INDEX         2
#define GATT_SVC_HRS_MEASUREMENT_CHAR_CCCD_INDEX    3

#define BODY_SENSOR_LOCATION_CHEST                  0x01

void bsal_le_hrs_svr_init(void *stack_ptr, void *app_callback);

void bsal_hrs_send_notify_level(void *stack_ptr, uint16_t conn_id,  uint8_t *hr_measurement);

#endif



