
// ****************************************************************************
// *                                                                          *
// *    Auto-created by 'genusb' - USB Descriptor Generator version 1.05      *
// *                                                                          *
// ****************************************************************************
// *                                                                          *
// *    Interfaces : CDC x 3                                                  *
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

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#define CFG_TUSB_RHPORT0_MODE   (OPT_MODE_DEVICE)

// .--------------------------------------------------------------------------.
// |    Allow maximum number of endpoints - 7 actually required               |
// `--------------------------------------------------------------------------'

#define CFG_TUD_EP_MAX          (16)

// .--------------------------------------------------------------------------.
// |    Supports 6 Virtual UART ports (CDC)                                   |
// `--------------------------------------------------------------------------'

#define CFG_TUD_CDC             (6)

#define CFG_TUD_CDC_EP_BUFSIZE  (256)
#define CFG_TUD_CDC_RX_BUFSIZE  (256)
#define CFG_TUD_CDC_TX_BUFSIZE  (256)

// .--------------------------------------------------------------------------.
// |    Does not support WebUSB (WEB)                                         |
// `--------------------------------------------------------------------------'

#define CFG_TUD_WEB             (0)

// .--------------------------------------------------------------------------.
// |    Does not support Network (NET)                                        |
// `--------------------------------------------------------------------------'

#define CFG_TUD_NET             (0)

// .--------------------------------------------------------------------------.
// |    Does not support Mass Storage Device (MSC)                            |
// `--------------------------------------------------------------------------'

#define CFG_TUD_MSC             (0)

// .--------------------------------------------------------------------------.
// |    Does not support HID Device (HID)                                     |
// `--------------------------------------------------------------------------'

#define CFG_TUD_HID             (0)

// .--------------------------------------------------------------------------.
// |    Does not support MIDI Device (MIDI)                                   |
// `--------------------------------------------------------------------------'

#define CFG_TUD_MIDI            (0)

// .--------------------------------------------------------------------------.
// |    Does not support Audio Device (AUDIO)                                 |
// `--------------------------------------------------------------------------'

#define CFG_TUD_AUDIO           (0)

// .--------------------------------------------------------------------------.
// |    Does not support Bluetooth Device (BTH)                               |
// `--------------------------------------------------------------------------'

#define CFG_TUD_BTH             (0)

// .--------------------------------------------------------------------------.
// |    Does not support Test and Measurement Class (TMC)                     |
// `--------------------------------------------------------------------------'

#define CFG_TUD_TMC             (0)

// .--------------------------------------------------------------------------.
// |    Does not support Generic User Display (GUD)                           |
// `--------------------------------------------------------------------------'

#define CFG_TUD_GUD             (0)

// .--------------------------------------------------------------------------.
// |    Does not support Vendor Commands (VENDOR)                             |
// `--------------------------------------------------------------------------'

#define CFG_TUD_VENDOR          (0)

// ****************************************************************************
// *                                                                          *
// *    Fixups required                                                       *
// *                                                                          *
// ****************************************************************************

// TinyUSB changed the name but we didn't

#define CFG_TUD_ECM_RNDIS       CFG_TUD_NET

#endif

