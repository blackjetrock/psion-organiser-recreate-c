
// ****************************************************************************
// *                                                                          *
// *    Auto-created by 'genusb' - USB Descriptor Generator version 1.05      *
// *                                                                          *
// ****************************************************************************
// *                                                                          *
// *    Interfaces : CDC                                                      *
// *                                                                          *
// ****************************************************************************

// The MIT License (MIT)
//
// Copyright 2021-2022, "Hippy"
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include "tusb.h"
#include "pico/unique_id.h"

#define MSC_ENABLED 1 // 0 does not work for some reason

// ****************************************************************************
// *                                                                          *
// *    VID and PID Definitions                                               *
// *                                                                          *
// ****************************************************************************

#define USBD_VID                (0xF055)
#define USBD_PID                (0x0202)

// ****************************************************************************
// *                                                                          *
// *    USB Device Descriptor Strings                                         *
// *                                                                          *
// ****************************************************************************

enum {
    USBD_STR_LANGUAGE,          // 0
    USBD_STR_MANUFACTURER,      // 1
    USBD_STR_PRODUCT,           // 2
    USBD_STR_SERIAL_NUMBER,     // 3
    USBD_STR_CDC_0_NAME,        // 4
    USBD_STR_CDC_1_NAME,        // 5
    USBD_STR_CDC_2_NAME,        // 6
    USBD_STR_CDC_3_NAME,        // 7
    USBD_STR_CDC_4_NAME,        // 8
    USBD_STR_CDC_5_NAME,        // 9
#if MSC_ENABLED
    USBD_STR_MSC,
#endif
};

char *const usbd_desc_str[] = {
    [USBD_STR_MANUFACTURER]     = "Org II",
    [USBD_STR_PRODUCT]          = "Recreate C",
    [USBD_STR_SERIAL_NUMBER]    = NULL,
    [USBD_STR_CDC_0_NAME]       = "CDC0",
    [USBD_STR_CDC_1_NAME]       = "CDC1",
    [USBD_STR_CDC_2_NAME]       = "CDC2",
    [USBD_STR_CDC_3_NAME]       = "CDC3",
    //[USBD_STR_CDC_4_NAME]       = "CDC4",
    //[USBD_STR_CDC_5_NAME]       = "CDC5",
#if MSC_ENABLED
    [USBD_STR_MSC]       = "Psion-MSC",
#endif
};

// ****************************************************************************
// *                                                                          *
// *    Device Descriptor                                                     *
// *                                                                          *
// ****************************************************************************

static const tusb_desc_device_t usbd_desc_device = {
    .bLength                    = sizeof(tusb_desc_device_t),
    .bDescriptorType            = TUSB_DESC_DEVICE,
    .bcdUSB                     = 0x0200,
    .bDeviceClass               = TUSB_CLASS_MISC,
    .bDeviceSubClass            = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol            = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0            = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor                   = USBD_VID,
    .idProduct                  = USBD_PID,
    .bcdDevice                  = 0x0100,
    .iManufacturer              = USBD_STR_MANUFACTURER,
    .iProduct                   = USBD_STR_PRODUCT,
    .iSerialNumber              = USBD_STR_SERIAL_NUMBER,
    .bNumConfigurations         = 1,
};

// ****************************************************************************
// *                                                                          *
// *    Endpoint Definitions                                                  *
// *                                                                          *
// ****************************************************************************

// .--------------------------------------------------------------------------.
// |    Virtual Serial Ports                                                  |
// `--------------------------------------------------------------------------'

#define EPNUM_CDC_0_CMD         (0x81)
#define EPNUM_CDC_0_DATA        (0x82)

#define EPNUM_CDC_1_CMD         (0x83)
#define EPNUM_CDC_1_DATA        (0x84)

#define EPNUM_CDC_2_CMD         (0x85)
#define EPNUM_CDC_2_DATA        (0x86)

#define EPNUM_CDC_3_CMD         (0x87)
#define EPNUM_CDC_3_DATA        (0x88)

#define EPNUM_CDC_4_CMD         (0x89)
#define EPNUM_CDC_4_DATA        (0x8A)

#define EPNUM_CDC_5_CMD         (0x8B)
#define EPNUM_CDC_5_DATA        (0x8C)

#define USBD_CDC_CMD_SIZE       (64)
#define USBD_CDC_DATA_SIZE      (64)

#if MSC_ENABLED
#define USBD_MSC_OUT	0x0B
#define USBD_MSC_IN		0x8E
#endif

// ****************************************************************************
// *                                                                          *
// *    Device Configuration                                                  *
// *                                                                          *
// ****************************************************************************

#define USBD_MAX_POWER_MA       (250)

//#define USBD_DESC_LEN           ( (TUD_CONFIG_DESC_LEN                    ) + \
                                  (TUD_CDC_DESC_LEN       * CFG_TUD_CDC   )   )

#define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN  + (TUD_CDC_DESC_LEN * CFG_TUD_CDC)) + (TUD_MSC_DESC_LEN)

enum {
    ITF_NUM_CDC_0,  ITF_NUM_CDC_0_DATA,
    ITF_NUM_CDC_1,  ITF_NUM_CDC_1_DATA,
    ITF_NUM_CDC_2,  ITF_NUM_CDC_2_DATA,
    ITF_NUM_CDC_3,  ITF_NUM_CDC_3_DATA,
    //ITF_NUM_CDC_4,  ITF_NUM_CDC_4_DATA,
    //ITF_NUM_CDC_5,  ITF_NUM_CDC_5_DATA,
#if MSC_ENABLED
    ITF_NUM_MSC_0,    ITF_NUM_MSC_0_DATA,
#endif
    ITF_NUM_TOTAL
};

static const uint8_t usbd_desc_cfg[USBD_DESC_LEN] = {

    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL,
                          USBD_STR_LANGUAGE,
                          USBD_DESC_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP,
                          USBD_MAX_POWER_MA),

    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0,
                      USBD_STR_CDC_0_NAME,
                         EPNUM_CDC_0_CMD, USBD_CDC_CMD_SIZE,
                         EPNUM_CDC_0_DATA & 0x7F,
                         EPNUM_CDC_0_DATA, USBD_CDC_DATA_SIZE),

    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_1,
                      USBD_STR_CDC_1_NAME,
                         EPNUM_CDC_1_CMD, USBD_CDC_CMD_SIZE,
                         EPNUM_CDC_1_DATA & 0x7F,
                         EPNUM_CDC_1_DATA, USBD_CDC_DATA_SIZE),

    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_2,
                      USBD_STR_CDC_2_NAME,
                         EPNUM_CDC_2_CMD, USBD_CDC_CMD_SIZE,
                         EPNUM_CDC_2_DATA & 0x7F,
                         EPNUM_CDC_2_DATA, USBD_CDC_DATA_SIZE),

    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_3,
                      USBD_STR_CDC_3_NAME,
                         EPNUM_CDC_3_CMD, USBD_CDC_CMD_SIZE,
                         EPNUM_CDC_3_DATA & 0x7F,
                         EPNUM_CDC_3_DATA, USBD_CDC_DATA_SIZE),
#if 0
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_4,
                      USBD_STR_CDC_4_NAME,
                         EPNUM_CDC_4_CMD, USBD_CDC_CMD_SIZE,
                         EPNUM_CDC_4_DATA & 0x7F,
                         EPNUM_CDC_4_DATA, USBD_CDC_DATA_SIZE),

    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_5,
                      USBD_STR_CDC_5_NAME,
                         EPNUM_CDC_5_CMD, USBD_CDC_CMD_SIZE,
                         EPNUM_CDC_5_DATA & 0x7F,
                         EPNUM_CDC_5_DATA, USBD_CDC_DATA_SIZE),
#endif

#if MSC_ENABLED
  	TUD_MSC_DESCRIPTOR(ITF_NUM_MSC_0, USBD_STR_MSC, USBD_MSC_OUT, USBD_MSC_IN, 64),  
#endif
};

// ****************************************************************************
// *                                                                          *
// *    USB Device Callbacks                                                  *
// *                                                                          *
// ****************************************************************************

const uint8_t *tud_descriptor_device_cb(void) {
    return (const uint8_t *)&usbd_desc_device;
}

const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return usbd_desc_cfg;
}

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    #define DESC_STR_MAX_LENGTH (20)
    static uint16_t desc_str[DESC_STR_MAX_LENGTH];

    uint8_t len;
    if (index == USBD_STR_LANGUAGE) {
        desc_str[1] = 0x0409; // Supported language is English
        len = 1;
    } else {
        if (index >= sizeof(usbd_desc_str) / sizeof(usbd_desc_str[0])) {
            return NULL;
        }
        if (index == USBD_STR_SERIAL_NUMBER) {
            pico_unique_board_id_t id;
            pico_get_unique_board_id(&id);
            // byte by byte conversion
            for (len = 0; len < 16; len += 2) {
                const char *hexdig = "0123456789ABCDEF";
                desc_str[1 + len + 0] = hexdig[id.id[len >> 1] >> 4];
                desc_str[1 + len + 1] = hexdig[id.id[len >> 1] & 0x0F];
            }
        } else {
            const char *str = usbd_desc_str[index];
            for (len = 0; len < DESC_STR_MAX_LENGTH - 1 && str[len]; ++len) {
                desc_str[1 + len] = str[len];
            }
        }
    }

    // first byte is length (including header), second byte is string type
    desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * len + 2);

    return desc_str;
}

