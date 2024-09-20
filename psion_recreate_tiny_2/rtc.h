////////////////////////////////////////////////////////////////////////////////
//
// MCP7940 Registers
extern int read_seconds;
extern int read_minutes;
extern int read_hours;

extern int rtc_set_variables;
extern int rtc_set_registers;

extern int rtc_seconds;
extern int rtc_minutes;
extern int rtc_hours;

#define MCP_RTCSEC_REG       0x00
#define MCP_ST_MASK          0x80

#define MCP_RTCMIN_REG       0x01

#define MCP_RTCHOUR_REG      0x02

#define MCP_RTCWKDAY_REG     0x03
#define MCP_VBATEN_MASK      0x08

#define MCP_RTCDATE_REG      0x04

#define MCP_RTCMTH_REG       0x05

#define MCP_RTCYEAR_REG      0x06

#define MCP_RTCC_CONTROL_REG 0x07
#define MCP_OUT_MASK         0x80

#define MCP_ALM0WKDAY_REG    0x0D
#define MCP_ALM1WKDAY_REG    0x14
#define MCP_ALMPOL_MASK      0x80


