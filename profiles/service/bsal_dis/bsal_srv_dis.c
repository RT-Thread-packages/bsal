/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-09-27     WaterFishJ   the first version
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "bsal.h"
#include "bsal_osif.h"
#include "bsal_srv_dis.h"

unsigned char MANUFACTURER_NAME_STRING[] = "RT-THREAD";
unsigned char PnP_ID[] = {0x02, 0x19, 0x15, 0xee, 0xee, 0x00, 0x01};


static P_SRV_GENERAL_CB pfn_bas_cb = NULL;

static void dis_profile_callback(void *p)
{
    bsal_callbak_data_t *p_param = (bsal_callbak_data_t *)p;
    bool is_app_cb = false;

    if (p_param->msg_type == BSAL_CALLBACK_TYPE_READ_CHAR_VALUE)
    {
        if (GATT_SVC_DIS_CHAR_MNS_INDEX == p_param->off_handle)
        {
            bsal_srv_write_data(p_param->stack_ptr, p_param->start_handle, p_param->off_handle, sizeof(MANUFACTURER_NAME_STRING), MANUFACTURER_NAME_STRING);
        }
        else if (GATT_SVC_DIS_CHAR_PNPID_INDEX == p_param->off_handle)
        {
            bsal_srv_write_data(p_param->stack_ptr, p_param->start_handle, p_param->off_handle, sizeof(PnP_ID), PnP_ID);
        }
        is_app_cb = true;
    }
    if (is_app_cb && (pfn_bas_cb != NULL))
    {
        pfn_bas_cb(p_param);
    }
}

void bsal_le_dis_svr_init(void *stack_ptr, void *app_callback)
{
    struct bsal_gatt_app_srv_def ble_svc_hid_defs[] =
    {
        {
            /*** Device Information Service. */
            .type = BSAL_GATT_UUID_PRIMARY_SERVICE,
            .uuid = BSAL_UUID16_DECLARE(GATT_UUID_DEVICE_INFORMATION),
            .characteristics = (bsal_gatt_chr_def_t[])
            {
                {
                    /*** Manufacturer Name String characteristic */
                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_MANUFACTURER_NAME_STRING),
                    .properties = BSAL_ATT_P_READ,
                    .permission = BSAL_GATT_PERM_READ_NONE,
                    .value_length = 10,
                },
                {
                    /*** PnP ID characteristic */
                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_PNP_ID),
                    .properties = BSAL_ATT_P_READ,
                    .permission = BSAL_GATT_PERM_READ_NONE,
                    .value_length = 7,
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
    bsal_stack_le_srv_reg_func(stack_ptr, &ble_svc_hid_defs, (P_SRV_GENERAL_CB *)dis_profile_callback);
    pfn_bas_cb = (P_SRV_GENERAL_CB)app_callback;
}
