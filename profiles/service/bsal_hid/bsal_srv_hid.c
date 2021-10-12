





#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>

#include <stdio.h>
#include <stdint.h>
#include "bsal.h"
#include "bsal_osif.h"
#include "bsal_srv_hid.h"


static P_SRV_GENERAL_CB pfn_bas_cb = NULL;


static uint8_t rep_map_data[] =
{
    0x05, 0x01, // Usage Page (Generic Desktop)
    0x09, 0x02, // Usage (Mouse)

    0xA1, 0x01, // Collection (Application)

    // Report ID 1: Mouse buttons + scroll/pan
    0x85, 0x01,       // Report Id 1
    0x09, 0x01,       // Usage (Pointer)
    0xA1, 0x00,       // Collection (Physical)
    0x95, 0x05,       // Report Count (3)
    0x75, 0x01,       // Report Size (1)
    0x05, 0x09,       // Usage Page (Buttons)
    0x19, 0x01,       // Usage Minimum (01)
    0x29, 0x05,       // Usage Maximum (05)
    0x15, 0x00,       // Logical Minimum (0)
    0x25, 0x01,       // Logical Maximum (1)
    0x81, 0x02,       // Input (Data, Variable, Absolute)
    0x95, 0x01,       // Report Count (1)
    0x75, 0x03,       // Report Size (3)
    0x81, 0x01,       // Input (Constant) for padding
    0x75, 0x08,       // Report Size (8)
    0x95, 0x01,       // Report Count (1)
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x38,       // Usage (Wheel)
    0x15, 0x81,       // Logical Minimum (-127)
    0x25, 0x7F,       // Logical Maximum (127)
    0x81, 0x06,       // Input (Data, Variable, Relative)
    0x05, 0x0C,       // Usage Page (Consumer)
    0x0A, 0x38, 0x02, // Usage (AC Pan)
    0x95, 0x01,       // Report Count (1)
    0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)
    0xC0,             // End Collection (Physical)

    // Report ID 2: Mouse motion
    0x85, 0x02,       // Report Id 2
    0x09, 0x01,       // Usage (Pointer)
    0xA1, 0x00,       // Collection (Physical)
    0x75, 0x0C,       // Report Size (12)
    0x95, 0x02,       // Report Count (2)
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x30,       // Usage (X)
    0x09, 0x31,       // Usage (Y)
    0x16, 0x01, 0xF8, // Logical maximum (2047)
    0x26, 0xFF, 0x07, // Logical minimum (-2047)
    0x81, 0x06,       // Input (Data, Variable, Relative)
    0xC0,             // End Collection (Physical)
    0xC0,             // End Collection (Application)

    // Report ID 3: Advanced buttons
    0x05, 0x0C,       // Usage Page (Consumer)
    0x09, 0x01,       // Usage (Consumer Control)
    0xA1, 0x01,       // Collection (Application)
    0x85, 0x03,       // Report Id (3)
    0x15, 0x00,       // Logical minimum (0)
    0x25, 0x01,       // Logical maximum (1)
    0x75, 0x01,       // Report Size (1)
    0x95, 0x01,       // Report Count (1)

    0x09, 0xCD,       // Usage (Play/Pause)
    0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)
    0x0A, 0x83, 0x01, // Usage (AL Consumer Control Configuration)
    0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)
    0x09, 0xB5,       // Usage (Scan Next Track)
    0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)
    0x09, 0xB6,       // Usage (Scan Previous Track)
    0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)

    0x09, 0xEA,       // Usage (Volume Down)
    0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)
    0x09, 0xE9,       // Usage (Volume Up)
    0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)
    0x0A, 0x25, 0x02, // Usage (AC Forward)
    0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)
    0x0A, 0x24, 0x02, // Usage (AC Back)
    0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)
    0xC0              // End Collection
};


static void hid_profile_callback(void *p)
{
    bsal_callbak_data_t *p_param = (bsal_callbak_data_t *)p;
    bool is_app_cb = false;
    rt_kprintf("msg_type = %d\n", p_param->msg_type);
    if (p_param->msg_type == BSAL_CALLBACK_TYPE_READ_CHAR_VALUE)
    {
        rt_kprintf("off_handle = %d\n", p_param->off_handle);
        if (GATT_SVC_HID_REPORT_MAP_INDEX == p_param->off_handle)
        {
            is_app_cb = true;
            rt_kprintf("report map\n");
//            bsal_srv_write_data(p_param->stack_ptr, p_param->start_handle, p_param->off_handle, sizeof(rep_map_data), rep_map_data);
        }
    }
    if (is_app_cb && (pfn_bas_cb != NULL))
    {
        pfn_bas_cb(p_param);
    }
}


void bsal_le_hid_svr_init(void *stack_ptr, void *app_callback)
{
    struct bsal_gatt_app_srv_def ble_svc_hid_defs[] =
    {
        {
            /*** Hid Service. */
            .type = BSAL_GATT_UUID_PRIMARY_SERVICE,
            .uuid = BSAL_UUID16_DECLARE(GATT_UUID_HID),
            .characteristics = (bsal_gatt_chr_def_t[])
            {
//                {
//                    /*** Protocol Mode characteristic */
//                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_PROTOCOL_MODE),
//                    .properties = BSAL_ATT_P_READ | BSAL_ATT_P_WRITE_WITHOUT_RESPONSE,
//                    .permission = BSAL_GATT_PERM_READ_NONE,
//                    .value_length = 1,
//                },
                {
                    /*** Input Report characteristics */
                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_INPUT_REPORT),
                    .properties = BSAL_ATT_P_READ | BSAL_ATT_P_WRITE | BSAL_ATT_P_NOTIFY,
                    .permission = BSAL_GATT_PERM_READ_NONE,
                    .value_length = 1,
                },
//                {
//                    /*** Output Report characteristics */
//                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_INPUT_REPORT),
//                    .properties = BSAL_ATT_P_READ | BSAL_ATT_P_WRITE | BSAL_ATT_P_NOTIFY,
//                    .permission = BSAL_GATT_PERM_READ_NONE,
//                    .value_length = 1,
//                },
//                {
//                    /*** Feature Report characteristics */
//                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_INPUT_REPORT),
//                    .properties = BSAL_ATT_P_READ | BSAL_ATT_P_WRITE | BSAL_ATT_P_NOTIFY,
//                    .permission = BSAL_GATT_PERM_READ_NONE,
//                    .value_length = 1,
//                },
//                {
//                    /*** Report Map characteristics */
//                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_REPORT_MAP),
//                    .properties = BSAL_ATT_P_READ,
//                    .permission = BSAL_GATT_PERM_READ_NONE,
//                    .value_length = 1,
////                    .value_length = sizeof(rep_map_data),
////                    .val_handle = rep_map_data,
//                },
//                {
//                    /*** Boot Mouse Input Report characteristics */
//                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_BOOT_MOUSE_INPUT_REPORT),
//                    .properties = BSAL_ATT_P_READ | BSAL_ATT_P_WRITE | BSAL_ATT_P_NOTIFY,
//                    .permission = BSAL_GATT_PERM_READ_NONE,
//                    .value_length = 1,
//                },
//                {
//                    /*** HID Information characteristics */
//                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_HID_INFORMATION),
//                    .properties = BSAL_ATT_P_READ,
//                    .permission = BSAL_GATT_PERM_READ_NONE,
//                    .value_length = 1,
//                },
//                {
//                    /*** HID Control Point characteristics */
//                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_HID_CONTROL_POINT),
//                    .properties = BSAL_ATT_P_WRITE_WITHOUT_RESPONSE,
//                    .permission = BSAL_GATT_PERM_READ_NONE,
//                    .value_length = 1,
//                },
                {
                    0, /* No more characteristics in this service. */
                }
            },
        },
//        {
//            /*** Battery Service. */
//            .type = BSAL_GATT_UUID_PRIMARY_SERVICE,
//            .uuid = BSAL_UUID16_DECLARE(GATT_UUID_BATTERY),
//            .characteristics = (bsal_gatt_chr_def_t[])
//            {
//                {
//                    /*** Battery Level characteristic */
//                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_BAS_LEVEL),
//                    .properties = BSAL_ATT_P_READ | BSAL_ATT_P_NOTIFY,
//                    .permission = BSAL_GATT_PERM_READ_NONE,
//                    .value_length = 1,
//                },
//                {
//                    0, /* No more characteristics in this service. */
//                }
//            },
//        },
//        {
//            /*** Device Information Service. */
//            .type = BSAL_GATT_UUID_PRIMARY_SERVICE,
//            .uuid = BSAL_UUID16_DECLARE(GATT_UUID_DEVICE_INFORMATION),
//            .characteristics = (bsal_gatt_chr_def_t[])
//            {
//                {
//                    /*** Manufacturer Name String characteristic */
//                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_MANUFACTURER_NAME_STRING),
//                    .properties = BSAL_ATT_P_READ,
//                    .permission = BSAL_GATT_PERM_READ_NONE,
//                    .value_length = 1,
//                },
//                {
//                    /*** PnP ID characteristic */
//                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_PNP_ID),
//                    .properties = BSAL_ATT_P_READ,
//                    .permission = BSAL_GATT_PERM_READ_NONE,
//                    .value_length = 1,
//                },
//                {
//                    0, /* No more characteristics in this service. */
//                }
//            },
//        },
//        {
//            /*** Generic Attribute Service. */
//            .type = BSAL_GATT_UUID_PRIMARY_SERVICE,
//            .uuid = BSAL_UUID16_DECLARE(GATT_UUID_GENERIC_ATTRIBUTE),
//            /* This service is empty */
//        },
//        {
//            /*** Generic Access Service. */
//            .type = BSAL_GATT_UUID_PRIMARY_SERVICE,
//            .uuid = BSAL_UUID16_DECLARE(GATT_UUID_GENERIC_ACCESS),
//            .characteristics = (bsal_gatt_chr_def_t[])
//            {
//                {
//                    /*** Device Name characteristic */
//                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_DEVICE_NAME),
//                    .properties = BSAL_ATT_P_READ | BSAL_ATT_P_WRITE,
//                    .permission = BSAL_GATT_PERM_READ_NONE,
//                    .value_length = 1,
//                },
//                {
//                    /*** Appearance characteristic */
//                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_GAP_APPEARANCE),
//                    .properties = BSAL_ATT_P_READ,
//                    .permission = BSAL_GATT_PERM_READ_NONE,
//                    .value_length = 1,
//                },
//                {
//                    /*** Peripheral Preferred Connection Parameters characteristic */
//                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_PPCP),
//                    .properties = BSAL_ATT_P_READ,
//                    .permission = BSAL_GATT_PERM_READ_NONE,
//                    .value_length = 1,
//                },
//                {
//                    /*** Central Address Resolution characteristic */
//                    .uuid = BSAL_UUID16_DECLARE(GATT_UUID_CHAR_CENTRAL_ADDRESS_RESOLUTION),
//                    .properties = BSAL_ATT_P_READ,
//                    .permission = BSAL_GATT_PERM_READ_NONE,
//                    .value_length = 1,
//                },
//                {
//                    0, /* No more characteristics in this service. */
//                }
//            },
//        },
        {
            0, /* No more services. */
        },
    };
    bsal_stack_le_srv_reg_func(stack_ptr, &ble_svc_hid_defs, (P_SRV_GENERAL_CB *)hid_profile_callback);
    pfn_bas_cb = (P_SRV_GENERAL_CB)app_callback;
}






