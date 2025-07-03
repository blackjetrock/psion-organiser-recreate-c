////////////////////////////////////////////////////////////////////////////////
//
//  QCode engines and information
//
////////////////////////////////////////////////////////////////////////////////



//------------------------------------------------------------------------------

#define QI_INT_SIM_FP           0x00    
#define QI_NUM_SIM_FP           0x01    
#define QI_STR_SIM_FP           0x02    
#define QI_INT_ARR_FP           0x03    
#define QI_NUM_ARR_FP           0x04    
#define QI_STR_ARR_FP           0x05    
#define QI_NUM_SIM_ABS          0x06    
#define QI_INT_SIM_IND          0x07    
#define QI_NUM_SIM_IND          0x08    
#define QI_STR_SIM_IND          0x09    
#define QI_INT_ARR_IND          0x0A    
#define QI_NUM_ARR_IND          0x0B    
#define QI_STR_ARR_IND          0x0C    
#define QI_LS_INT_SIM_FP        0x0D    
#define QI_LS_NUM_SIM_FP        0x0E    
#define QI_LS_STR_SIM_FP        0x0F    
#define QI_LS_INT_ARR_FP        0x10    
#define QI_LS_NUM_ARR_FP        0x11    
#define QI_LS_STR_ARR_FP        0x12    
#define QI_LS_NUM_SIM_ABS       0x13    
#define QI_LS_INT_SIM_IND       0x14    
#define QI_LS_NUM_SIM_IND       0x15    
#define QI_LS_STR_SIM_IND       0x16    
#define QI_LS_INT_ARR_IND       0x17    
#define QI_LS_NUM_ARR_IND       0x18    
#define QI_LS_STR_ARR_IND       0x19
#define QI_INT_FLD              0x1A    
#define QI_NUM_FLD              0x1B    
#define QI_STR_FLD              0x1C    
#define QI_LS_INT_FLD           0x1D    
#define QI_LS_NUM_FLD           0x1E    
#define QI_LS_STR_FLD           0x1F    
#define QI_STK_LIT_BYTE         0x20    
#define QI_STK_LIT_WORD         0x21    
#define QI_INT_CON              0x22    
#define QI_NUM_CON              0x23    
#define QI_STR_CON              0x24    
#define QCO_SPECIAL             0x25    
#define QCO_BREAK               0x26    
#define QCO_LT_INT              0x27    
#define QCO_LTE_INT             0x28    
#define QCO_GT_INT              0x29    
#define QCO_GTE_INT             0x2A    
#define QCO_NE_INT              0x2B    
#define QCO_EQ_INT              0x2C    
#define QCO_ADD_INT             0x2D    
#define QCO_SUB_INT             0x2E    
#define QCO_MUL_INT             0x2F    
#define QCO_DIV_INT             0x30    
#define QCO_POW_INT             0x31    
#define QCO_UMIN_INT            0x32    
#define QCO_NOT_INT             0x33    
#define QCO_AND_INT             0x34    
#define QCO_OR_INT              0x35    
#define QCO_LT_NUM              0x36    
#define QCO_LTE_NUM             0x37    
#define QCO_GT_NUM              0x38    
#define QCO_GTE_NUM             0x39    
#define QCO_NE_NUM              0x3A    
#define QCO_EQ_NUM              0x3B    
#define QCO_ADD_NUM             0x3C    
#define QCO_SUB_NUM             0x3D    
#define QCO_MUL_NUM             0x3E    
#define QCO_DIV_NUM             0x3F    
#define QCO_POW_NUM             0x40    
#define QCO_UMIN_NUM            0x41    
#define QCO_NOT_NUM             0x42    
#define QCO_AND_NUM             0x43    
#define QCO_OR_NUM              0x44    
#define QCO_LT_STR              0x45    
#define QCO_LTE_STR             0x46    
#define QCO_GT_STR              0x47    
#define QCO_GTE_STR             0x48    
#define QCO_NE_STR              0x49    
#define QCO_EQ_STR              0x4A    
#define QCO_ADD_STR             0x4B    
#define QCO_AT                  0x4C    
#define QCO_BEEP                0x4D    
#define QCO_CLS                 0x4E    
#define QCO_CURSOR              0x4F    
#define QCO_ESCAPE              0x50    
#define QCO_GOTO                0x51    
#define QCO_OFF                 0x52    
#define QCO_ONERR               0x53    
#define QCO_PAUSE               0x54    
#define QCO_POKEB               0x55    
#define QCO_POKEW               0x56    
#define QCO_RAISE               0x57    
#define QCO_RANDOMIZE           0x58    
#define QCO_STOP                0x59    
#define QCO_TRAP                0x5A    
#define QCO_APPEND              0x5B    
#define QCO_CLOSE               0x5C    
#define QCO_COPY                0x5D    
#define QCO_CREATE              0x5E    
#define QCO_DELETE              0x5F    
#define QCO_ERASE               0x60    
#define QCO_FIRST               0x61    
#define QCO_LAST                0x62    
#define QCO_NEXT                0x63    
#define QCO_BACK                0x64    
#define QCO_OPEN                0x65    
#define QCO_POSITION            0x66    
#define QCO_RENAME              0x67    
#define QCO_UPDATE              0x68    
#define QCO_USE                 0x69    
#define QCO_KSTAT               0x6A    
#define QCO_EDIT                0x6B    
#define QCO_INPUT_INT           0x6C    
#define QCO_INPUT_NUM           0x6D    
#define QCO_INPUT_STR           0x6E    
#define QCO_PRINT_INT           0x6F    
#define QCO_PRINT_NUM           0x70    
#define QCO_PRINT_STR           0x71    
#define QCO_PRINT_SP            0x72    
#define QCO_PRINT_CR            0x73    
#define QCO_LPRINT_INT          0x74    
#define QCO_LPRINT_NUM          0x75    
#define QCO_LPRINT_STR          0x76    
#define QCO_LPRINT_SP           0x77    
#define QCO_LPRINT_CR           0x78    
#define QCO_RETURN              0x79    
#define QCO_RETURN_NOUGHT       0x7A    
#define QCO_RETURN_ZERO         0x7B    
#define QCO_RETURN_NULL         0x7C    
#define QCO_PROC                0x7D    
#define QCO_BRA_FALSE           0x7E    
#define QCO_ASS_INT             0x7F    
#define QCO_ASS_NUM             0x80    
#define QCO_ASS_STR             0x81    
#define QCO_DROP_BYTE           0x82    
#define QCO_DROP_WORD           0x83    
#define QCO_DROP_NUM            0x84    
#define QCO_DROP_STR            0x85    
#define QCO_INT_TO_NUM          0x86    
#define QCO_NUM_TO_INT          0x87    
#define QCO_END_FIELDS          0x88    
#define QCO_RUN_ASSEM           0x89    
#define RTF_ADDR                0x8A    
#define RTF_ASC                 0x8B    
#define RTF_DAY                 0x8C    
#define RTF_DISP                0x8D    
#define RTF_ERR                 0x8E    
#define RTF_FIND                0x8F    
#define RTF_FREE                0x90    
#define RTF_GET                 0x91    
#define RTF_HOUR                0x92    
#define RTF_IABS                0x93    
#define RTF_INT                 0x94    
#define RTF_KEY                 0x95    
#define RTF_LEN                 0x96    
#define RTF_LOC                 0x97    
#define RTF_MENU                0x98    
#define RTF_MINUTE              0x99    
#define RTF_MONTH               0x9A    
#define RTF_PEEKB               0x9B    
#define RTF_PEEKW               0x9C    
#define RTF_RECSIZE             0x9D    
#define RTF_SECOND              0x9E    
#define RTF_IUSR                0x9F    
#define RTF_VIEW                0xA0    
#define RTF_YEAR                0xA1    
#define RTF_COUNT               0xA2    
#define RTF_EOF                 0xA3    
#define RTF_EXIST               0xA4    
#define RTF_POS                 0xA5    
#define RTF_ABS                 0xA6    
#define RTF_ATAN                0xA7    
#define RTF_COS                 0xA8    
#define RTF_DEG                 0xA9    
#define RTF_EXP                 0xAA    
#define RTF_FLT                 0xAB    
#define RTF_INTF                0xAC    
#define RTF_LN                  0xAD    
#define RTF_LOG                 0xAE    
#define RTF_PI                  0xAF    
#define RTF_RAD                 0xB0    
#define RTF_RND                 0xB1    
#define RTF_SIN                 0xB2    
#define RTF_SQR                 0xB3    
#define RTF_TAN                 0xB4    
#define RTF_VAL                 0xB5    
#define RTF_SPACE               0xB6    
#define RTF_DIR                 0xB7    
#define RTF_CHR                 0xB8    
#define RTF_DATIM               0xB9    
#define RTF_SERR                0xBA    
#define RTF_FIX                 0xBB    
#define RTF_GEN                 0xBC    
#define RTF_SGET                0xBD    
#define RTF_HEX                 0xBE    
#define RTF_SKEY                0xBF    
#define RTF_LEFT                0xC0    
#define RTF_LOWER               0xC1    
#define RTF_MID                 0xC2    
#define RTF_NUM                 0xC3    
#define RTF_RIGHT               0xC4    
#define RTF_REPT                0xC5    
#define RTF_SCI                 0xC6    
#define RTF_UPPER               0xC7    
#define RTF_SUSR                0xC8    
#define RTF_SADDR               0xC9    

// LZ QCode
#define RTF_DOW                 0xD7
#define RTF_LTPERCENT           0xCC
#define RTF_GTPERCENT           0xCD
#define RTF_PLUSPERCENT         0xCE
#define RTF_MINUSPERCENT        0xCF
#define RTF_TIMESPERCENT        0xD0
#define RTF_DIVIDEPERCENT       0xD1
#define RTF_OFFX                0xD2
#define RTF_COPYW               0xD3
#define RTF_DELETEW             0xD4
#define RTF_UDG                 0xD5
#define RTF_CLOCK               0xD6
#define RTF_DOW                 0xD7
#define RTF_FINDW               0xD8
#define RTF_MENUN               0xD9
#define RTF_WEEK                0xDA
#define RTF_ACOS                0xDB
#define RTF_ASIN                0xDC
#define RTF_DAYS                0xDD
#define RTF_MAX                 0xDE
#define RTF_MEAN                0xDF
#define RTF_MIN                 0xE0
#define RTF_STD                 0xE1
#define RTF_SUM                 0xE2
#define RTF_VAR                 0xE3
#define RTF_DAYNAME             0xE4
#define RTF_DIRW                0xE5
#define RTF_MONTHSTR            0xE6

////////////////////////////////////////////////////////////////////////////////
//
// Error codes
//

#define ER_AL_NC  255   // NO MORE ALLOCATOR CELLS
#define ER_AL_NR  254   // NO MORE ROOM
#define ER_MT_EX  253   // EXPONENT OVERFLOW (OR UNDERFLOW)
#define ER_MT_IS  252   // CONVERSION FROM STRING TO NUMERIC FAILED
#define ER_MT_DI  251   // DIVIDE BY ZERO
#define ER_MT_FL  250   // CONVERSION FROM NUMERIC TO STRING FAILED
#define ER_IM_OV  249   // BCD STACK OVERFLOW
#define ER_IM_UN  248   // BCD STACK UNDERFLOW
#define ER_FN_BA  247   // BAD ARGUMENT IN FUNCTION CALL
#define ER_PK_NP  246   // NO PACK IN SLOT
#define ER_PK_DE  245   // DATAPACK ERROR (WRITE ERROR)
#define ER_PK_RO  244   // ATTEMPTED WRITE TO READ ONLY PACK
#define ER_PK_DV  243   // BAD DEVICE NAME
#define ER_PK_CH  242   // PACK CHANGED
#define ER_PK_NB  241   // PACK NOT BLANK
#define ER_PK_IV  240   // UNKNOWN PACK TYPE
#define ER_FL_PF  239   // PACK FULL
#define ER_FL_EF  238   // END OF FILE
#define ER_FL_BR  237   // BAD RECORD TYPE
#define ER_FL_BN  236   // BAD FILE NAME
#define ER_FL_EX  235   // FILE ALREADY EXISTS
#define ER_FL_NX  234   // FILE DOES NOT EXIST
#define ER_FL_DF  233   // DIRECTORY FULL
#define ER_FL_CY  232   // PACK NOT COPYABLE
#define ER_DV_CA  231   // INVALID DEVICE CALL
#define ER_DV_NP  230   // DEVICE NOT PRESENT
#define ER_DV_CS  229   // CHECKSUM ERROR
#define ER_EX_SY  228   // SYNTAX ERROR
#define ER_EX_MM  227   // MISMATCHED BRACKETS
#define ER_EX_FA  226   // WRONG NUMBER OF FUNCTION ARGS
#define ER_EX_AR  225   // SUBSCRIPT OR DIMENSION ERROR
#define ER_EX_TV  224   // TYPE VIOLATION
#define ER_LX_ID  223   // IDENTIFIER TOO LONG
#define ER_LX_FV  222   // BAD FIELD VARIABLE NAME
#define ER_LX_MQ  221   // UNMATCHED QUOTES IN STRING
#define ER_LX_ST  220   // STRING TOO LONG
#define ER_LX_US  219   // UNRECOGNISED SYMBOL
#define ER_LX_NM  218   // NON-VALID NUMERIC FORMAT
#define ER_TR_PC  217   // MISSING PROCEDURE DECLARATION
#define ER_TR_DC  216   // ILLEGAL DECLARATION
#define ER_TR_IN  215   // NON-INTEGER DIMENSION
#define ER_TR_DD  214   // NAME ALREADY DECLARED
#define ER_TR_ST  213   // STRUCTURE ERROR
#define ER_TR_ND  212   // NESTING TOO DEEP
#define ER_TR_NL  211   // LABEL REQUIRED
#define ER_TR_CM  210   // MISSING COMMA
#define ER_TR_BL  209   // BAD LOGICAL FILE NAME
#define ER_TR_PA  208   // ARGUMENTS MAY NOT BE TARGET OF ASSIGN
#define ER_TR_FL  207   // TOO MANY FIELDS
#define ER_RT_BK  206   // BREAK KEY
#define ER_RT_NP  205   // WRONG NUMBER OF PARAMETERS
#define ER_RT_UE  204   // UNDEFINED EXTERNAL
#define ER_RT_PN  203   // PROCEDURE NOT FOUND
#define ER_RT_ME  202   // MENU ERROR
#define ER_RT_NF  201   // FIELD NOT FOUND
#define ER_PK_BR  200   // PACK BAD READ ERROR
#define ER_RT_FO  199   // FILE ALREADY OPEN (OPEN/DELETE)
#define ER_RT_RB  198   // RECORD TOO BIG
#define ER_LG_BN  197   // BAD PROCEDURE NAME
#define ER_RT_FC  196   // FILE NOT OPEN (CLOSE)
#define ER_RT_IO  195   // INTEGER OVERFLOW
#define ER_GN_BL  194   // BATTERY TOO LOW
#define ER_GN_RF  193   // DEVICE READ FAIL
#define ER_GN_WF  192   // DEVICE WRITE FAIL

#define ER_FL_NP  300


//------------------------------------------------------------------------------

#define QCO_OPEN     0x65
#define QCO_CREATE   0x5E

////////////////////////////////////////////////////////////////////////////////

// QCode state
// Used to pass execution state of a single QCode

typedef struct _NOBJ_QCS
{
  NOBJ_QCODE qcode;
  NOBJ_INT   integer;
  NOBJ_INT   integer2;
  NOBJ_INT   result;
  NOPL_FLOAT num_result;
  NOPL_FLOAT num;
  NOPL_FLOAT num2;
  uint16_t   arr_idx;
  uint16_t   ind_ptr;
  uint8_t    len;
  uint8_t    len2;
  uint8_t    data8;
  char       str[NOBJ_STRING_MAXLEN+1];
  char       str2[NOBJ_STRING_MAXLEN+1];
  uint16_t   str_addr;
  uint16_t   addr;
  char       procpath[NOBJ_FILENAME_MAXLEN];
  uint8_t    max_sz;
  int        i;
  uint8_t    field_flag;
  int        done;
  // Foloowing values are for the list processing
  int        flist_flag;
  int        count;
  int        num_i;
} NOBJ_QCS;

typedef void (*NOBJ_QC_ACTION)(NOBJ_MACHINE *m, NOBJ_QCS *s);

#define NOBJ_QC_NUM_ACTIONS 3

typedef struct
{
  NOBJ_QCODE      qcode;
  char            *name;
  NOBJ_QC_ACTION  action[NOBJ_QC_NUM_ACTIONS];
  
} NOBJ_QCODE_INFO;

////////////////////////////////////////////////////////////////////////////////

typedef int    (*QC_BYTE_LEN_FN)(int i, NOBJ_QCODE *qc);
typedef char * (*QC_BYTE_PRT_FN)(int i, NOBJ_QCODE *qc);

typedef struct
{
  char *code;
  QC_BYTE_LEN_FN len_fn;
  QC_BYTE_PRT_FN prt_fn;
} QC_BYTE_CODE;

////////////////////////////////////////////////////////////////////////////////

typedef struct _QCODE_DESC
{
  uint8_t qcode;
  char *bytes;
  char *pops;
  char *pushes;
  char *desc;  
} QCODE_DESC;

extern QCODE_DESC qcode_decode[];

////////////////////////////////////////////////////////////////////////////////

void qcode_get_string_push_stack(NOBJ_MACHINE *m);
void execute_qcode(NOBJ_MACHINE *m, int single_step);
uint8_t qcode_next_8(NOBJ_MACHINE *m);
uint16_t qcode_next_16(NOBJ_MACHINE *m);
void db_qcode(char *s, ...);
char *qcode_name(NOBJ_QCODE qcode);


extern FILE *exdbfp;


#if 1
#define dbq(fmt...) dbpfq(__FUNCTION__, fmt)
#define dbq_num(text, num) dbq_num_f(__FUNCTION__, text, num)
#define dbq_num_exploded(text, num) dbq_num_exploded_f(__FUNCTION__, text, num)
#else
#define dbq(fmt...) ;
#define dbq_num(text, num) ;
#define dbq_num_exploded(text, num) ;
#endif

void dbpfq(const char *caller, char *fmt, ...);
char *decode_qc_txt(int *i,  NOBJ_QCODE **qc);
extern int qcode_sizeof_qcode_decode;
extern int qcode_sizeof_qc_byte_code;

int null_qc_byte_len_fn_2(int i, NOBJ_QCODE *qc);
int null_qc_byte_len_fn_1(int i, NOBJ_QCODE *qc);
char *qc_byte_prt_fn_v(int i, NOBJ_QCODE *qc);
char *qc_byte_prt_fn_V(int i, NOBJ_QCODE *qc);
char *qc_byte_prt_fn_B(int i, NOBJ_QCODE *qc);
char *qc_byte_prt_fn_O(int i, NOBJ_QCODE *qc);
char *qc_byte_prt_fn_f(int i, NOBJ_QCODE *qc);
char *qc_byte_prt_fn_m(int i, NOBJ_QCODE *qc);
char *qc_byte_prt_fn_I(int i, NOBJ_QCODE *qc);
char *qc_byte_prt_fn_D(int i, NOBJ_QCODE *qc);
char *qc_byte_prt_fn_S(int i, NOBJ_QCODE *qc);
int qc_byte_len_fn_S(int i, NOBJ_QCODE *qc);
char *qc_byte_prt_fn_F(int i, NOBJ_QCODE *qc);
int qc_byte_len_fn_F(int i, NOBJ_QCODE *qc);
char *null_qc_byte_prt_fn(int i, NOBJ_QCODE *qc);
int null_qc_byte_fn(int i, NOBJ_QCODE *qc);
void pr_uint8(uint8_t n);
void pr_var_space_size(NOBJ_VAR_SPACE_SIZE *x);
void pr_qcode_space_size(NOBJ_QCODE_SPACE_SIZE *x);
void pr_global_varname_size(NOBJ_GLOBAL_VARNAME_SIZE *x);
void pr_external_varname_size(NOBJ_EXTERNAL_VARNAME_SIZE *x);
void pr_num_parameters(NOBJ_NUM_PARAMETERS *x);
void pr_parameter_types(NOBJ_PROC *p);
void decode_qc(int *i,  NOBJ_QCODE **qc);
void qca_push_proc(NOBJ_MACHINE *m, NOBJ_QCS *s);
void dbpfq(const char *caller, char *fmt, ...);
void qca_assign_int_field(char *string, int val);
void qcode_init(void);
void qca_right(NOBJ_MACHINE *m, NOBJ_QCS *s);
