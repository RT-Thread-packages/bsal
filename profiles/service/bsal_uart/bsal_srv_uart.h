/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-09-09     WaterFishJ   the first version
 */

#ifndef __BSAL_SRV_UART_H__
#define __BSAL_SRV_UART_H__

#include <stdint.h>
#include <stdbool.h>
#include "bsal.h"


#define GATT_SVC_NUS_READ_INDEX				2
#define GATT_SVC_NUS_WRITE_INDEX			5
#define GATT_SVC_NUS_CHAR_CCCD_INDEX	3



int bsal_bleuart_init(void *stack_ptr, uint16_t conn_id);

void bsal_le_uart_svr_init(void *stack_ptr, void *app_callback);




#endif






