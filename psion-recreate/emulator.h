////////////////////////////////////////////////////////////////////////////////
//
// 6303 Emulator
//
////////////////////////////////////////////////////////////////////////////////



#define REG_A  (pstate.A)
#define REG_B  (pstate.B)
#define REG_D  ((((u_int16_t)REG_A) << 8) | REG_B )
#define WRITE_REG_D(XXX) (REG_A = (XXX >> 8));(REG_B = (XXX & 0xFF))

#define REG_PC (pstate.PC)
#define REG_SP (pstate.SP)
#define REG_X  (pstate.X)

#define FLAG_H_MASK  0x20
#define FLAG_I_MASK  0x10
#define FLAG_N_MASK  0x08
#define FLAG_Z_MASK  0x04
#define FLAG_V_MASK  0x02
#define FLAG_C_MASK  0x01
#define FLAG_TERM_MASK 0xFF

#define FLG_H      (pstate.FLAGS & FLAG_H_MASK)
#define FLG_I      (pstate.FLAGS & FLAG_I_MASK)
#define FLG_N      (pstate.FLAGS & FLAG_N_MASK)
#define FLG_Z      (pstate.FLAGS & FLAG_Z_MASK)
#define FLG_V      (pstate.FLAGS & FLAG_V_MASK)
#define FLG_C      (pstate.FLAGS & FLAG_C_MASK)

typedef struct _FLAG_DATA
{
  u_int8_t mask;
  char name;
} FLAG_DATA;

extern FLAG_DATA  flag_data[];
  
#define REG_FLAGS     (pstate.FLAGS)

#define B3(XXX)   (XXX & 0x0008)
#define B7(XXX)   (XXX & 0x0080)
#define B15(XXX)  (XXX & 0x8000)

#define NB3(XXX)  ( !B3(XXX))
#define NB7(XXX)  ( !B7(XXX))
#define NB15(XXX) (!B15(XXX))

// Flag Clearing
#define FL_N0        (REG_FLAGS &= (~FLAG_N_MASK))
#define FL_Z0        (REG_FLAGS &= (~FLAG_Z_MASK))
#define FL_Z1        (REG_FLAGS |= (FLAG_Z_MASK))
#define FL_V0        (REG_FLAGS &= (~FLAG_V_MASK))
#define FL_V1        (REG_FLAGS |= (FLAG_V_MASK))
#define FL_C0        (REG_FLAGS &= (~FLAG_C_MASK))
#define FL_C1        (REG_FLAGS |= (FLAG_C_MASK))
#define FL_I0        (REG_FLAGS &= (~FLAG_I_MASK))
#define FL_I1        (REG_FLAGS |= FLAG_I_MASK)
#define FL_H0        (REG_FLAGS &= (~FLAG_H_MASK))
#define FL_H1        (REG_FLAGS |= FLAG_H_MASK)




#define FL_VSET(XXX) XXX?FL_V1:FL_V0
#define FL_V_NXORC   (((FLG_N && (!FLG_C)) || ((!FLG_N) && FLG_C)))?FL_V1:FL_V0

#define FL_CSET(XXX) XXX?FL_C1:FL_C0
#define FL_C_0OR1    (FLG_C?1:0)

#define FL_V80(XXX)  if(XXX==0x80) {REG_FLAGS |= FLAG_V_MASK;} else {REG_FLAGS &= ~FLAG_V_MASK;}
#define FL_V8T(RR,MM,XX)  if( (B7(XX) && NB7(MM) && NB7(RR)) || (NB7(XX) &&  B7(MM) && B7(RR)) )FL_V1; else FL_V0;
#define FL_V8TP(RR,MM,XX) if( (B7(XX) &&  B7(MM) && NB7(RR)) || (NB7(XX) && NB7(MM) && B7(RR)) )FL_V1; else FL_V0;

#define FL_C8T(RR,MM,XX)  ( (NB7(XX) && B7(MM)) || (B7(MM) &&  B7(RR)) || ( B7(RR) && NB7(XX)))?FL_C1:FL_C0
#define FL_C8TP(RR,MM,XX) (  (B7(XX) && B7(MM)) || (B7(MM) && NB7(RR)) || (NB7(RR) &&  B7(XX)))?FL_C1:FL_C0
#define FL_C8W(XXX) if(XXX!=0x00) {REG_FLAGS |= FLAG_C_MASK;} else {REG_FLAGS &= ~FLAG_C_MASK;}
// Flag testing
#define FL_ZT(XXX)   if(XXX==0) {REG_FLAGS |= FLAG_Z_MASK;} else {REG_FLAGS &= ~FLAG_Z_MASK;}

#define FL_N8T(XXX)  if(XXX & 0x0080) {REG_FLAGS |= FLAG_N_MASK;} else {REG_FLAGS &= ~FLAG_N_MASK;}
#define FL_N16T(XXX) if(XXX & 0x8000) {REG_FLAGS |= FLAG_N_MASK;} else {REG_FLAGS &= ~FLAG_N_MASK;}

#define FL_H(RR,MM,XX)     (( B3(XX) && B3(MM)) || (B3(MM) && NB3(RR)) || (NB3(RR) && B3(XX)) )?FL_H1:FL_H0

#define FL_V16AT(RR,MM,XX) (( B15(XX) &&  B15(MM) && NB15(RR)) || (NB15(XX) && NB15(MM) &&  B15(RR)) )?FL_V1:FL_V0
#define FL_V16ST(RR,MM,XX) (( B15(XX) && NB15(MM) && NB15(RR)) || (NB15(XX) &&  B15(MM) &&  B15(RR)) )?FL_V1:FL_V0

#define FL_C16AT(RR,MM,XX) (( B15(XX) &&  B15(MM)) || (B15(MM) && NB15(RR)) || (NB15(RR) &&  B15(XX)))?FL_C1:FL_C0
#define FL_C16ST(RR,MM,XX) ((NB15(XX) &&  B15(MM)) || (B15(MM) &&  B15(RR)) || ( B15(RR) && NB15(XX)))?FL_C1:FL_C0

typedef struct _PROC6303_STATE
{
  u_int16_t  PC;
  u_int16_t  X;
  u_int16_t  SP;
  u_int8_t   A;
  u_int8_t   B;
  u_int8_t   FLAGS;
  int        memory;        // Memory reference is being used
  u_int16_t  memory_addr;   // Address of memory being referenced
} PROC6303_STATE;

extern PROC6303_STATE pstate;

////////////////////////////////////////////////////////////////////////////////
//
//

#define COLD_START_STATE   0x00
#define WARM_START_STATE   0x80

////////////////////////////////////////////////////////////////////////////////
//
// LCD controller
//

#define LCD_CTRL_REG   0x0180
#define LCD_DATA_REG   0x0181

#define SWITCH_OFF     0x01c0

#define SCA_PULSE_ENABLE  0x0200
#define SCA_PULSE_DISABLE 0x0240
#define SCA_ALARM_HIGH    0x0280
#define SCA_ALARM_LOW     0x02c0

#define TURN_BUZZER_ON  latch2_set_mask( LAT2PIN_MASK_BUZZER)
#define TURN_BUZZER_OFF latch2_clear_mask( LAT2PIN_MASK_BUZZER)

#define SCA_RESET      0x0300
#define SCA_CLOCK      0x0340


// Timer1
#define TIM1_TCSR       0x0008
#define TIM1_COUNTER_H  0x0009
#define TIM1_COUNTER_L  0x000A
#define TIM1_OCOMP_H    0x000B
#define TIM1_OCOMP_L    0x000C
#define TIM1_ICAP_H     0x000D
#define TIM1_ICAP_L     0x000E

#define PORT5           0x0015
#define PORT2           0x0003
#define PORT6           0x0017

#define PORT2_DDR       0x0001
#define PORT6_DDR       0x0016
  
////////////////////////////////////////////////////////////////////////////////
//
// Ports, registers
//

#define SERIAL_TRCSR   0x11

#define TRCSR_RDRF      0x80
#define TRCSR_ORFE      0x40
#define TRCSR_TDRE      0x20
#define TRCSR_RIE       0x10
#define TRCSR_TDRE      0x20

#define SERIAL_RDR      0x12
#define SERIAL_TDR      0x13

#define P5_CTRL         0x14
#define P5_DATA         0x15

////////////////////////////////////////////////////////////////////////////////

#define KBB_STAT        0x7b
#define KBB_NKEYS       0x74

////////////////////////////////////////////////////////////////////////////////
//
// System Variables
//

#define RTT_NUMB        0x20FF
#define TMB_YEAR        0x20C5    // Current year, 0 - 99 (0 - 255 on LZ)
#define TMB_MONS	0x20C6    // Current month, 0 - 11.
#define TMB_DAYS	0x20C7    //Current date of month, 0 - 30.
#define TMB_HOUR	0x20C8    //Current hour, 0 - 23.
#define TMB_MINS	0x20C9    //Current minutes, 0 - 59.
#define TMB_SECS	0x20CA    //Current seconds, 0 - 59.


////////////////////////////////////////////////////////////////////////////////

extern char opcode_decode[100];
extern u_int8_t opcode;
extern char *opcode_names[256];
extern int inst_length;
extern int pc_before;
extern u_int16_t sca_counter;

u_int8_t RD_ADDR(u_int16_t addr);
void update_interrupts(void);
void interrupt_request(u_int16_t vector_msb);
void serial_set_rdrf(void);
void handle_serial_register_read(u_int16_t addr);
void handle_serial_register_write(u_int16_t addr, u_int8_t value);
void handle_cursor(void);
void display_restore(void);
void display_save(void);
void handle_power_off(void);
void loop_emulator(void);
void WR_ADDR(u_int16_t addr, u_int8_t value);

#if PROT
#define RAMDATA_FIX(AAA)  ramdata[(AAA>=0x8000)?badram():AAA]
#define RAMDATA(AAA)      ramdata[((AAA>=0x8000)?badram():AAA)<BANK_START)?AAA:(AAA+ram_bank_off)]

#define ROMDATA_FIX(AAA)  romdata[(AAA>=0x8000)?badrom():AAA]
#define ROMDATA(AAA)      romdata[((AAA>=0x8000)?badrom():AAA)>BANK_START)?AAA:(AAA+rom_bank_off)]

//#define ROMDATA_FIX(AAA)  romdata[AAA]
//#define ROMDATA(AAA)      romdata[(AAA>BANK_START)?AAA:(AAA+rom_bank_off)]
#else
#define RAMDATA_FIX(AAA)  ramdata[AAA]
#define RAMDATA(AAA)      ramdata[(AAA<BANK_START)?AAA:(AAA+ram_bank_off)]

#define ROMDATA_FIX(AAA)  romdata[AAA]
#define ROMDATA(AAA)      romdata[(AAA>BANK_START)?AAA:(AAA+rom_bank_off)]
#endif

// How big ram and rom are

#define BANK_RESET       0x0360
#define BANK_NEXT_ROM    0x03e0
#define BANK_NEXT_RAM    0x03a0

#define BANK_START       0x4000

extern u_int8_t romdata[];
extern int ram_bank_off;
extern int rom_bank_off;

extern u_int8_t ramdata[RAM_SIZE];
