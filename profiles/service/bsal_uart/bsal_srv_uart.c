/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-09-09     WaterFishJ   the first version
 */
 
#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>

#include <stdio.h>
#include <stdint.h>
#include "bsal.h"
#include "bsal_osif.h"
#include "bsal_srv_uart.h"

#define MYNEWT_VAL_BLEUART_MAX_INPUT 128

#define ESC_KEY                      0x1B
#define BACKSPACE_KEY                0x08
#define DELECT_KEY                   0x7F

/* {6E400001-B5A3-F393-E0A9-E50E24DCCA9E} */
static const struct bsal_uuid128 gatt_svr_svc_uart_uuid =
    BSAL_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
                     0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e);

/* {6E400002-B5A3-F393-E0A9-E50E24DCCA9E} */
static const struct bsal_uuid128 gatt_svr_chr_uart_write_uuid =
    BSAL_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
                     0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x6e);


/* {6E400003-B5A3-F393-E0A9-E50E24DCCA9E} */
static const struct bsal_uuid128 gatt_svr_chr_uart_read_uuid =
    BSAL_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
                     0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40, 0x6e);

/* Pointer to a console buffer */
static rt_uint8_t *console_buf;

struct bleuart_console
{
    struct rt_semaphore *rx_end;
    struct rt_ringbuffer *rx_fifo;
    rt_err_t (*old_rx_ind)(rt_device_t dev, rt_size_t size);
};

static struct bleuart_console bleuart = {0};

static P_SRV_GENERAL_CB pfn_bas_cb = NULL;
static void profile_callback(void *p)
{
		bsal_callbak_data_t *p_param = (bsal_callbak_data_t *)p;
    bool is_app_cb = false;
		
		rt_kprintf("type = %d\n", p_param->msg_type);
		rt_kprintf("profile callback u_type: %d\n", p_param->srv_uuid.u_type);
	
		if (p_param->msg_type == BSAL_CALLBACK_TYPE_READ_CHAR_VALUE)
    {
        //NO DEAL had not finished
//				rt_kprintf("read_index = %d\n", p_param->off_handle);
				is_app_cb = true;
    }
    else if (p_param->msg_type == BSAL_CALLBACK_TYPE_WRITE_CHAR_VALUE)
    {
				if (GATT_SVC_NUS_WRITE_INDEX == p_param->off_handle)
        {
						is_app_cb = true;
						rt_device_write(rt_console_get_device(), 0, (char *)p_param->data, p_param->length);
						rt_device_write(rt_console_get_device(), 0, "\n", 1);
				}
		}
		else if (p_param->msg_type == BSAL_CALLBACK_TYPE_INDIFICATION_NOTIFICATION)
    {
//				rt_kprintf("CCCD off_handle = %d\n", p_param->off_handle);
        if (GATT_SVC_NUS_CHAR_CCCD_INDEX == p_param->off_handle)
        {
            if (p_param->length == 2)
            {
                //uint16_t ccdbit = (uint16_t)p_param->data;
                is_app_cb = true;
            }
        }
    }
		if (is_app_cb && (pfn_bas_cb != NULL))
    {
        pfn_bas_cb(p_param);
    }
}

void bsal_le_uart_svr_init(void *stack_ptr, void *app_callback)
{
    struct bsal_gatt_app_srv_def ble_svc_uart_defs[] =
    {
        {
            /*** Uart Service. */
            .type = BSAL_GATT_UUID_PRIMARY_SERVICE,
            .uuid = BSAL_UUID128_DECLARE(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
                     0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e),//(bsal_uuid_any_t *)&battery_srv,//BSAL_UUID16_DECLARE(GATT_UUID_BATTERY),
            .characteristics = (bsal_gatt_chr_def_t[])
            {
                {
                    /*** Uart read characteristic */
                    .uuid = BSAL_UUID128_DECLARE(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
                     0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40, 0x6e),//(bsal_uuid_any_t *)&bas_char_bas_level,//BSAL_UUID16_DECLARE(GATT_UUID_CHAR_BAS_LEVEL),
                    .properties = BSAL_ATT_P_NOTIFY,
                    .permission = BSAL_GATT_PERM_READ_NONE,
                    .value_length = 1,
                },
								{
										/*** Uart write characteristic */
										.uuid = BSAL_UUID128_DECLARE(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
                     0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x6e),//(bsal_uuid_any_t *)&bas_char_bas_level,//BSAL_UUID16_DECLARE(GATT_UUID_CHAR_BAS_LEVEL),
                    .properties = BSAL_ATT_P_WRITE
                    | BSAL_ATT_P_WRITE_WITHOUT_RESPONSE
                    ,
                    .permission = BSAL_GATT_PERM_WRITE_NONE,
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
    bsal_stack_le_srv_reg_func(stack_ptr, &ble_svc_uart_defs, (P_SRV_GENERAL_CB *)profile_callback);
    pfn_bas_cb = (P_SRV_GENERAL_CB)app_callback;
}

static rt_err_t bleuart_rx_ind(rt_device_t dev, rt_size_t size)
{
    uint8_t ch;
    int i;

    for(i = 0; i < size; i++)
    {
        /* read a char */
        if (rt_device_read(dev, 0, &ch, 1))
        {
            rt_ringbuffer_put_force(bleuart.rx_fifo, &ch, 1);
            rt_sem_release(bleuart.rx_end);
        }
    }

    return RT_EOK;
}

static uint8_t bleuart_read(void)
{
    uint8_t ch;

    rt_sem_take(bleuart.rx_end, RT_WAITING_FOREVER);
    rt_ringbuffer_getchar(bleuart.rx_fifo, &ch);

    return ch;
}

void bsal_bleuart_deinit(void)
{
    rt_base_t level;
    rt_device_t uart_console;

    level = rt_hw_interrupt_disable();
    uart_console = rt_console_get_device();
    if(uart_console)
    {
        rt_device_set_rx_indicate(uart_console, bleuart.old_rx_ind);
    }
    rt_hw_interrupt_enable(level);

    if (console_buf != RT_NULL)
    {
        rt_free(console_buf);
        console_buf = RT_NULL;
    }

    if (bleuart.rx_end != RT_NULL)
    {
        rt_sem_delete(bleuart.rx_end);
        bleuart.rx_end = RT_NULL;
    }

    if (bleuart.rx_fifo != RT_NULL)
    {
        rt_ringbuffer_destroy(bleuart.rx_fifo);
        bleuart.rx_fifo = RT_NULL;
    }
}

void bsal_bleuart_uart_proc(void *stack_ptr, uint16_t conn_id)
{
    int off = 0;
    char ch;
    struct os_mbuf *om;
	
		bsal_uuid_any_t uuid_srv;
    uuid_srv.u_type = BSAL_UUID_TYPE_128BIT;
		rt_memcpy(uuid_srv.u128.value, gatt_svr_chr_uart_read_uuid.value, 16);
//    uuid_srv.u128.value = ;
    uint16_t start_handle = bsal_srv_get_start_handle(stack_ptr, uuid_srv);

    rt_kprintf("======== Welcome to enter bluetooth uart mode ========\n");
    rt_kprintf("Press 'ESC' to exit.\n");

    /* process user input */
    while (ESC_KEY != (ch = bleuart_read()))
    {
        if(ch != '\r' && ch != '\n')
        {
            if(ch == BACKSPACE_KEY || ch == DELECT_KEY)
            {
                if(off)
                {
                    console_buf[off--] = 0;
                    rt_kprintf("\b \b");
                }
								continue;
            }
            else
            {
                console_buf[off++] = ch;
                rt_kprintf("%c", ch);
                continue;
            }
        }    

        console_buf[off] = '\0';
        rt_kprintf("\n");
//				rt_kprintf("send connid = %d\n", conn_id);
//				rt_kprintf("send start_handle = %d\n", start_handle);
//				bsal_srv_write_data(p_param->stack_ptr, p_param->start_handle, p_param->off_handle, sizeof(console_buf), console_buf);
				bsal_srv_send_notify_data(stack_ptr, conn_id, start_handle, GATT_SVC_NUS_READ_INDEX, sizeof(console_buf), console_buf);
//        om = ble_hs_mbuf_from_flat(console_buf, off);
//        if (!om) {
//            return;
//        }
//        ble_gattc_notify_custom(g_console_conn_handle,
//                                g_bleuart_attr_read_handle, om);
        off = 0;
    }

    bsal_bleuart_deinit();
}


int bsal_bleuart_init(void *stack_ptr, uint16_t conn_id)
{
    int rc;
    rt_base_t level;
    rt_device_t uart_console;

    /* create buffer for send */
    console_buf = rt_malloc(MYNEWT_VAL_BLEUART_MAX_INPUT);
    if (console_buf == RT_NULL)
    {
        rc = -RT_ENOMEM;
        goto __exit;
    }

    /* create semaphore for the end of char recived */
    bleuart.rx_end = rt_sem_create("bleuart", 0, RT_IPC_FLAG_FIFO);
    if (bleuart.rx_end == RT_NULL)
    {
        rc = -RT_ENOMEM;
        goto __exit;
    }

    /* create recived fifo */
    bleuart.rx_fifo = rt_ringbuffer_create(MYNEWT_VAL_BLEUART_MAX_INPUT);
    if (bleuart.rx_fifo == RT_NULL)
    {
        rc = -RT_ENOMEM;
        goto __exit;
    }

    level = rt_hw_interrupt_disable();
    uart_console = rt_console_get_device();
    if(uart_console)
    {
        /* back uart console old indicate callback */
        bleuart.old_rx_ind = uart_console->rx_indicate;
        rt_device_set_rx_indicate(uart_console, bleuart_rx_ind);
    }
    rt_hw_interrupt_enable(level);

    /* Reads console and sends data over BLE */
    bsal_bleuart_uart_proc(stack_ptr, conn_id);

    return RT_EOK;

__exit:
    if (console_buf != RT_NULL)
    {
        rt_free(console_buf);
        console_buf = RT_NULL;
    }

    if (bleuart.rx_end != RT_NULL)
    {
        rt_sem_delete(bleuart.rx_end);
        bleuart.rx_end = RT_NULL;
    }

    if (bleuart.rx_fifo != RT_NULL)
    {
        rt_ringbuffer_destroy(bleuart.rx_fifo);
        bleuart.rx_fifo = RT_NULL;
    }

    return rc;
}

