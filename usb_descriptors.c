#include "tusb.h"

// ================================
// Device Descriptor
// ================================
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,

    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,

    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0xCafe,   
    .idProduct          = 0x4002,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

// Callback
uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &desc_device;
}

// ================================
// Configuration Descriptor
// ================================

#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN * 4)

#define EPNUM_CDC0_NOTIF   0x81
#define EPNUM_CDC0_OUT     0x02
#define EPNUM_CDC0_IN      0x82

#define EPNUM_CDC1_NOTIF   0x83
#define EPNUM_CDC1_OUT     0x04
#define EPNUM_CDC1_IN      0x84

uint8_t const desc_configuration[] = {
    
    // Config, you have to modify the 5 parameter when change the Number of CDC
    TUD_CONFIG_DESCRIPTOR(1, 8, 0, CONFIG_TOTAL_LEN,
                           0x00, 100),

    // CDC 0
    TUD_CDC_DESCRIPTOR(0, 4, 0x81, 8, 0x02, 0x82, 64),

    // CDC 1
    TUD_CDC_DESCRIPTOR(2, 5, 0x83, 8, 0x04, 0x84, 64),

    // CDC 2
    TUD_CDC_DESCRIPTOR(4, 6, 0x85, 8, 0x06, 0x86, 64),

    // CDC 3
    TUD_CDC_DESCRIPTOR(6, 7, 0x87, 8, 0x08, 0x88, 64),
};

uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index;
    return desc_configuration;
}

// ================================
// String Descriptors
// ================================

char const* string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 },   // 0: English (0x0409)
    "RP2040 Lab",                    // 1: Manufacturer
    "Dual CDC UART Bridge",          // 2: Product
    "RP2040-0001",                   // 3: Serial
    "CDC UART0",                     // 4: CDC0
    "CDC UART1",                     // 5: CDC1
};

static uint16_t _desc_str[32];

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;

    uint8_t chr_count;

    if (index == 0) {
        _desc_str[1] = 0x0409;
        chr_count = 1;
    } else {
        if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) {
            return NULL;
        }

        const char* str = string_desc_arr[index];
        chr_count = strlen(str);

        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
    }

    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    return _desc_str;
}
