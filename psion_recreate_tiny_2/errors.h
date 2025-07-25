


////////////////////////////////////////////////////////////////////////////////
//
// Psion error codes
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

#define ER_TM_MN  310   // Month number out of range

void runtime_error(int error_code, char *fmt, ...);
void trappable_runtime_error(int error_code, char *fmt, ...);
char *error_text(int error_code);
void runtime_error_msg(int error_code, char *fmt, ...);
void runtime_error_print(void);
