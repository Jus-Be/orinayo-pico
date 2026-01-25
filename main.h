#define TINYUSB_AUDIO_FUNC_DESC_LEN    90
#define TINYUSB_AUDIO_STD_AS_INTF_CNT  2
#define TINYUSB_AUDIO_CTRL_REQ_BUF_SZ  64
#define TINYUSB_AUDIO_MAX_EP_IN_SZ     192

#define TUD_AUDIO_DESC_LEN              0x64
#define TUD_AUDIO_AS_INT_DESC_CNT       2
#define CFG_TUD_AUDIO_CTRL_BUF_SIZE     64
#define CFG_TUD_AUDIO_EP_SZ_IN          192
#include "tusb.h"
#include "audio_device.h"