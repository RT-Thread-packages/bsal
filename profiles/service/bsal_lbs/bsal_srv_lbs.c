/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-09-27     WaterFishJ   the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>

#include <stdio.h>
#include <stdint.h>
#include "bsal.h"
#include "bsal_osif.h"
#include "bsal_srv_lbs.h"

/* {00001523-1212-efde-1523-785feabcd123} */
static const struct bsal_uuid128 gatt_svr_svc_lbs_uuid =
    BSAL_UUID128_INIT(0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15,
                      0xDE, 0xEF, 0x12, 0x12, 0x23, 0x15, 0x00, 0x00);

/* {00001524-1212-efde-1523-785feabcd123} */
static const struct bsal_uuid128 gatt_svr_chr_button_uuid =
    BSAL_UUID128_INIT(0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15,
                      0xDE, 0xEF, 0x12, 0x12, 0x24, 0x15, 0x00, 0x00);


/* {00001525-1212-efde-1523-785feabcd123} */
static const struct bsal_uuid128 gatt_svr_chr_led_uuid =
    BSAL_UUID128_INIT(0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15,
                      0xDE, 0xEF, 0x12, 0x12, 0x25, 0x15, 0x00, 0x00);

static P_SRV_GENERAL_CB pfn_bas_cb = NULL;
uint8_t *button_cccd_flag;
uint8_t led_flag = 0;
static void lbs_profile_callback(void *p)
{
    bsal_callbak_data_t *p_param = (bsal_callbak_data_t *)p;
    bool is_app_cb = false;

    if (p_param->msg_type == BSAL_CALLBACK_TYPE_READ_CHAR_VALUE)
    {
        if (GATT_SVC_LBS_BUTTON_INDEX == p_param->off_handle)
        {
            is_app_cb = true;
            uint8_t button = !rt_pin_read(BUTTON_PIN);
            bsal_srv_write_data(p_param->stack_ptr, p_param->start_handle, p_param->off_handle, 1, &button);
        }
        else if (GATT_SVC_LBS_LED_INDEX == p_param->off_handle)
        {
            bsal_srv_write_data(p_param->stack_ptr, p_param->start_handle, p_param->off_handle, 1, &led_flag);
        }
    }
    else if (p_param->msg_type == BSAL_CALLBACK_TYPE_WRITE_CHAR_VALUE)
    {
        if (GATT_SVC_LBS_LED_INDEX == p_param->off_handle)
        {
            is_app_cb = true;
            led_flag = *(p_param->data);
            rt_pin_write(LED_PIN, !led_flag);
        }
    }
    else if (p_param->msg_type == BSAL_CALLBACK_TYPE_INDIFICATION_NOTIFICATION)
    {
        if (GATT_SVC_LBS_BUTTON_CCCD_INDEX == p_param->off_handle)
        {
            if (p_param->length == 2)
            {
                uint16_t  cccbits = p_param->value;
                if (cccbits & BSAL_GATT_CCC_NOTIFY)
                {
                    *button_cccd_flag = 1;
                }
                else *button_cccd_flag = 0;
                is_app_cb = true;
            }
        }
    }
    if (is_app_cb && (pfn_bas_cb != NULL))
    {
        pfn_bas_cb(p_param);
    }
}

void bsal_le_lbs_svr_init(void *stack_ptr, void *app_callback, uint8_t *button_flag)
{
    struct bsal_gatt_app_srv_def ble_svc_lbs_defs[] =
    {
        {
            /*** LBS Service. */
            .type = BSAL_GATT_UUID_PRIMARY_SERVICE,
            .uuid = BSAL_UUID128_DECLARE(0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15,
                                         0xDE, 0xEF, 0x12, 0x12, 0x23, 0x15, 0x00, 0x00),
            .characteristics = (bsal_gatt_chr_def_t[])
            {
                {
                    /*** Button characteristic */
                    .uuid = BSAL_UUID128_DECLARE(0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15,
                                                 0xDE, 0xEF, 0x12, 0x12, 0x24, 0x15, 0x00, 0x00),
                    .properties = BSAL_ATT_P_READ | BSAL_ATT_P_NOTIFY,
                    .permission = BSAL_GATT_PERM_READ_NONE,
                    .value_length = 1,
                },
                {
                    /*** LED characteristic */
                    .uuid = BSAL_UUID128_DECLARE(0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15,
                                                 0xDE, 0xEF, 0x12, 0x12, 0x25, 0x15, 0x00, 0x00),
                    .properties = BSAL_ATT_P_READ | BSAL_ATT_P_WRITE,
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
    bsal_stack_le_srv_reg_func(stack_ptr, &ble_svc_lbs_defs, (P_SRV_GENERAL_CB *)lbs_profile_callback);
    pfn_bas_cb = (P_SRV_GENERAL_CB)app_callback;
    button_cccd_flag = button_flag;
}


void bsal_lbs_send_notify_button(void *stack_ptr, uint16_t conn_id,  uint8_t flag)
{
    bsal_uuid_any_t uuid_srv;
    uuid_srv.u_type = 128;
    rt_memcpy(uuid_srv.u128.value, gatt_svr_svc_lbs_uuid.value, 16);
    uint16_t start_handle = bsal_srv_get_start_handle(stack_ptr, uuid_srv);
    bsal_srv_send_notify_data(stack_ptr, conn_id, start_handle, GATT_SVC_LBS_BUTTON_INDEX, sizeof(uint8_t), &flag);
}



