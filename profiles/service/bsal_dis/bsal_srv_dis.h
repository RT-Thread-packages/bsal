/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-09-27     WaterFishJ   the first version
 */

#ifndef __BSAL_SRV_DIS_H__
#define __BSAL_SRV_DIS_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"

#define GATT_UUID_DEVICE_INFORMATION               BSAL_GATT_SERVICE_DEVICE_INFORMATION
#define GATT_UUID_CHAR_MANUFACTURER_NAME_STRING    BSAL_UUID_CHAR_MANUFACTURER_NAME_STRING
#define GATT_UUID_CHAR_PNP_ID                      BSAL_UUID_CHAR_PNP_ID

#define GATT_SVC_DIS_CHAR_MNS_INDEX                2
#define GATT_SVC_DIS_CHAR_PNPID_INDEX              4


void bsal_le_dis_svr_init(void *stack_ptr, void *app_callback);
    

#ifdef __cplusplus
}
#endif

#endif

