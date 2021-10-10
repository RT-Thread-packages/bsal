/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-09-010     WaterFishJ   the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>

#include <stdio.h>
#include <stdint.h>
#include "bsal.h"
#include "bsal_osif.h"
#include "bsal_srv_hrs.h"

static uint16_t hrs_hrm_handle;

/* Sensor location, set to "Chest" */
static uint8_t body_sens_loc = BODY_SENSOR_LOCATION_CHEST;

static P_SRV_GENERAL_CB pfn_bas_cb = NULL;
static void hrs_profile_callback(void *p)
{
    bsal_callbak_data_t *p_param = (bsal_callbak_data_t *)p;
    bool is_app_cb = false;

    if (p_param->msg_type == BSAL_CALLBACK_TYPE_READ_CHAR_VALUE)
    {
        if (GATT_SVC_BODY_SENSOR_LOCATION_READ_INDEX == p_param->off_handle)
        {
            is_app_cb = true;
            bsal_srv_write_data(p_param->stack_ptr, p_param->start_handle, p_param->off_handle, sizeof(uint8_t), &body_sens_loc);
        }
    }
    else if (p_param->msg_type == BSAL_CALLBACK_TYPE_WRITE_CHAR_VALUE)
    {
        is_app_cb = true;
    }
    else if (p_param->msg_type == BSAL_CALLBACK_TYPE_INDIFICATION_NOTIFICATION)
    {
        if (GATT_SVC_HRS_MEASUREMENT_CHAR_CCCD_INDEX == p_param->off_handle)
        {
            if (p_param->length == 2)
            {
                is_app_cb = true;
            }
        }
    }
    if (is_app_cb && (pfn_bas_cb != NULL))
    {
        pfn_bas_cb(p_param);
    }
}

void bsal_le_hrs_svr_init(void *stack_ptr, void *app_callback)
{
    struct bsal_gatt_app_srv_def ble_svc_hrs_defs[] =
    {
        {
            /*** Heart Rate Service. */
            .type = BSAL_GATT_UUID_PRIMARY_SERVICE,
            .uuid = BSAL_UUID16_DECLARE(GATT_UUID_HEART_RATE),
            .characteristics = (bsal_gatt_chr_def_t[])
            {
                {
                    /*** Heart Rate Measurement characteristic */
                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_HRS_MEASUREMENT),
                    .properties = BSAL_ATT_P_NOTIFY,
                    .val_handle = &hrs_hrm_handle,
                    .permission = BSAL_GATT_PERM_READ_NONE,
                    .value_length = 1,
                },
                {
                    /*** Body Sensor Location characteristic */
                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_BODY_SENSOR_LOCATION),
                    .properties = BSAL_ATT_P_READ,
                    .val_handle = &hrs_hrm_handle,
                    .permission = BSAL_GATT_PERM_READ_NONE,
                    .value_length = 1,
                },
                {
                    0, /* No more characteristics in this service. */
                }
            },
        },
        {
            0, /* No more services. */
        },
    };
    bsal_stack_le_srv_reg_func(stack_ptr, &ble_svc_hrs_defs, (P_SRV_GENERAL_CB *)hrs_profile_callback);
    pfn_bas_cb = (P_SRV_GENERAL_CB)app_callback;
}

void bsal_hrs_send_notify_level(void *stack_ptr, uint16_t conn_id,  uint8_t *hr_measurement)
{
    bsal_uuid_any_t uuid_srv;
    uuid_srv.u_type = 16;
    uuid_srv.u16.value = GATT_UUID_HEART_RATE;
    uint16_t start_handle = bsal_srv_get_start_handle(stack_ptr, uuid_srv);
    bsal_srv_send_notify_data(stack_ptr, conn_id, start_handle, GATT_SVC_HRS_MEASUREMENT_CHAR_INDEX, sizeof(hr_measurement), hr_measurement);
}



