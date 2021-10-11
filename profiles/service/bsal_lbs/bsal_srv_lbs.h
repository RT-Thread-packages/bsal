/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-09-27     WaterFishJ   the first version
 */

#ifndef __BSAL_SRV_LBS_H__
#define __BSAL_SRV_LBS_H__


#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdbool.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "bsal.h"

#define BUTTON_PIN                                      11
#define LED_PIN                                         15

#define GATT_SVC_LBS_BUTTON_INDEX                       2
#define GATT_SVC_LBS_BUTTON_CCCD_INDEX                  3
#define GATT_SVC_LBS_LED_INDEX                          5

void bsal_le_lbs_svr_init(void *stack_ptr, void *app_callback, uint8_t *button_flag);

void bsal_lbs_send_notify_button(void *stack_ptr, uint16_t conn_id, uint8_t flag);

#endif




