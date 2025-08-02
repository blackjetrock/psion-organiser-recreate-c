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


extern uint8_t timedate[20];
extern uint8_t memdata[64];

// Reads first set of registers into timedate[]
// These are timekeeping registers

void read_rtc(void);
char *rtc_get_time(void);
void rtc_set_seconds(int s);
void rtc_set_minutes(int m);
void rtc_set_hours(int h);
void set_st_bit(void);
void clr_st_bit(void);
void rtc_dump(void);
void rtc_write_mem(void);
void rtc_read_mem(void);

int rtc_get_seconds(void);
int rtc_get_minutes(void);
int rtc_get_hours(void);

 

