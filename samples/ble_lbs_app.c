/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-09-27     WaterFishJ   the first version
 */

#include "bsal.h"
#include <stdio.h>
#include <string.h>
#include "bsal_osif.h"
#include "bsal_srv_lbs.h"


#define BSAL_STACK_NAME PKG_BSAL_STACK_NAME

static void *bsal_stack_ptr = NULL;
static uint16_t bsal_app_conn_handle;
static rt_uint8_t gap_conn_state = BSAL_GAP_CONN_STATE_CONNECTED;
static rt_uint8_t button_cccd_flag;

static void bsa_app_set_adv_data(void *stack_ptr)
{
    uint8_t tmp_data[32] = {0} ; //must be zero
    bsal_le_adv_data_add_flag(tmp_data, BSAL_GAP_ADTYPE_FLAGS_LIMITED | BSAL_GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED);

    char *adv_name = (char *)bsal_get_device_name(stack_ptr);
    bsal_adv_data_add_name(tmp_data, strlen(adv_name), adv_name);
    //bsal_adv_data_add_uuid16(tmp_data, BSAL_GATT_SERVICE_BATTERY_SERVICE);
    bsal_set_le_adv_data_user(stack_ptr, tmp_data);
}

static void bsal_app_all_callback(void *stack_ptr, uint8_t cb_layer, uint16_t cb_sub_event, uint8_t value_length, void *value)
{
    T_BSAL_GAP_MSG_DATA  *bsal_gap_msg_data = (T_BSAL_GAP_MSG_DATA *)value;
    uint8_t bd_addr[6];
    switch (cb_layer)
    {
    case BSAL_CB_LAYER_GAP:
        switch (cb_sub_event)
        {
        case BSAL_CB_STACK_READY:
            //get mac address

            bsal_osif_printf_info("============stack ready===========\r\n");
            bsa_app_set_adv_data(stack_ptr);
            bsal_stack_start_adv(stack_ptr);
            break;
        case BSAL_CB_CONNECT_STATUS:
            bsal_osif_printf_info("============stack connect id %d===========\r\n", bsal_gap_msg_data->gap_conn_state_change.conn_id);
            if (bsal_gap_msg_data->gap_conn_state_change.new_state == BSAL_GAP_CONN_STATE_CONNECTED)
            {
                bsal_app_conn_handle = bsal_gap_msg_data->gap_conn_state_change.conn_id;
            }
            else if (bsal_gap_msg_data->gap_conn_state_change.new_state == BSAL_GAP_CONN_STATE_DISCONNECTED)
            {
                bsal_stack_start_adv(stack_ptr);
            }
            bsal_osif_printf_info("BSAL: conn_id %d old_state %d new_state %d, disc_cause 0x%x",
                                  bsal_gap_msg_data->gap_conn_state_change.conn_id, gap_conn_state, bsal_gap_msg_data->gap_conn_state_change.new_state, bsal_gap_msg_data->gap_conn_state_change.disc_cause);

            break;
        default:
            break;
        }

        if (cb_sub_event == BSAL_CB_STACK_READY)
        {
            //stack ready
        }

        break;
    case BSAL_CB_LAYER_GATT_PROFILE:
        switch (cb_sub_event)
        {
            //save the service start_handle
            //case uuid profile save start_handle
            //case SRV_CALLBACK66
            //save the identity
        }
        break;
    case BSAL_CB_LAYER_SM:
        break;
    case BSAL_CB_LAYER_COMMON:
        //connected save the connect id

        break;
    case BSAL_CB_LAYER_UNKNOWN:
        break;
    default:
        break;
    }

}

bool lbs_is_uuid(bsal_uuid_any_t *s, bsal_uuid_any_t *u)
{
    if (s->u_type == u->u_type)
    {
        switch (s->u_type)
        {
        case BSAL_UUID_TYPE_128BIT:
            for (rt_uint8_t i = 0; i < 16; i++)
            {
                if (s->u128.value[i] != u->u128.value[i])   return false;
            }
            return true;
        case BSAL_UUID_TYPE_16BIT:
            break;
        case BSAL_UUID_TYPE_32BIT:
            break;
        default:
            return false;
        }
    }
    else    return false;
}

static void bsal_app_profile_callback(void *p)
{
    bsal_callbak_data_t *bsal_param = (bsal_callbak_data_t *)p;

    if (bsal_param->msg_type == BSAL_CALLBACK_TYPE_READ_CHAR_VALUE)
    {
        bsal_osif_printf_info("========callback read from %x====%x=======\r\n", bsal_param->off_handle, bsal_param->srv_uuid.u16.value);
    }
    else if (bsal_param->msg_type == BSAL_CALLBACK_TYPE_INDIFICATION_NOTIFICATION)
    {
        uint16_t  cccbits = bsal_param->value;
        bsal_osif_printf_info("======callback notify from %x===data cccd %x====%x=====\r\n", bsal_param->off_handle, cccbits, bsal_param->srv_uuid.u16.value);
        if (lbs_is_uuid(&(bsal_param->srv_uuid), BSAL_UUID128_DECLARE(0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15,
                      0xDE, 0xEF, 0x12, 0x12, 0x23, 0x15, 0x00, 0x00)))//lbs_uuid
        {
            if (cccbits & BSAL_GATT_CCC_NOTIFY)
            {
                bsal_osif_printf_info("=========NOTIFY ENABLE from %x===data cccd %x====%x=====\r\n", bsal_param->off_handle, cccbits, bsal_param->srv_uuid.u16.value);
            }
            else
            {
                bsal_osif_printf_info("========NOTIFY DISABLE from %x===data cccd %x====%x=====\r\n", bsal_param->off_handle, cccbits, bsal_param->srv_uuid.u16.value);
            }
        }
    }
    else if (bsal_param->msg_type == BSAL_CALLBACK_TYPE_WRITE_CHAR_VALUE)
    {
        bsal_osif_printf_info("\r\n BSAL: THE DATA IS :%s\r\n", bsal_param->data);
    }
}

rt_sem_t button_sem = 0;
void button_irq(void *p)
{
    rt_sem_release(p);
}

void bsal_lbs_loop(void *p)
{
    button_sem = rt_sem_create("button", 0, RT_IPC_FLAG_FIFO);
    
    rt_pin_mode(BUTTON_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(BUTTON_PIN, PIN_IRQ_MODE_RISING_FALLING, button_irq, button_sem);
    rt_pin_irq_enable(BUTTON_PIN, PIN_IRQ_ENABLE);
    
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(LED_PIN, PIN_HIGH);
    
    uint8_t flag = 0;
    
    while (1)
    {
        rt_sem_take(button_sem, RT_WAITING_FOREVER);
        if (button_cccd_flag)
        {
            flag = !rt_pin_read(BUTTON_PIN);
            bsal_lbs_send_notify_button(p, bsal_app_conn_handle, flag);
        }
    }
}

int bsal_lbs_app(void)
{   
    void *bsal_test_app_task = RT_NULL;
    
    void *stack_ptr = bsal_find_stack_ptr(BSAL_STACK_NAME);
    if (stack_ptr == NULL)
    {
        //print error;
        return 1;
    }
    //set iocapability

    bsal_stack_ptr = stack_ptr;
    //1. init stack
    bsal_stack_init(stack_ptr, bsal_app_all_callback);  // init param not start stack
    // set device name
    char *device_name = "ble_rtt_lbs";
    bsal_set_device_name(stack_ptr, strlen(device_name), (uint8_t *)device_name);
    //2. bond type
    bsal_set_device_le_bond_type(stack_ptr, false, BSAL_NO_INPUT, BSAL_NO_OUTPUT, BSAL_GAP_AUTHEN_BIT_NO_BONDING, false);
    //set the bond flag:

    //3. service begin
    bsal_stack_le_srv_begin(stack_ptr, 1, bsal_app_profile_callback);  //will add 1 service

    //4. lbs init
    bsal_le_lbs_svr_init(stack_ptr, bsal_app_profile_callback, &button_cccd_flag);

    //5. srv_end
    bsal_stack_le_srv_end(stack_ptr);    //end srv add

    //6. start stack
    bsal_stack_startup(stack_ptr);    //start she
    
    bsal_test_app_task = rt_thread_create("lbs_task", bsal_lbs_loop, stack_ptr, 2 * 256, 5, 10);
    if (bsal_test_app_task != RT_NULL)
    {
        rt_thread_startup(bsal_test_app_task);
    }
    return 0;
}
MSH_CMD_EXPORT_ALIAS(bsal_lbs_app, bsal_lbs_app, "bluetoooth LED Button sample");









