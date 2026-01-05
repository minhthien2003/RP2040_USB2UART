#pragma once

#define CFG_TUSB_MCU          OPT_MCU_RP2040
#define CFG_TUSB_RHPORT0_MODE OPT_MODE_DEVICE

#define CFG_TUD_CDC           2   // 🔥 2 COM
#define CFG_TUD_CDC_RX_BUFSIZE 64
#define CFG_TUD_CDC_TX_BUFSIZE 64

#define CFG_TUD_ENDPOINT0_SIZE 64
