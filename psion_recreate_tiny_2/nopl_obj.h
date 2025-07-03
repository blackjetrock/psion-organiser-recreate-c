// New OPL object code definitions.
//
// This is backwards compatible with QCode where possible.
//

#define NOPL_CONFIG_FORCE_VARS_UPPERCASE 1

#define NOPL_MAX_TOKEN           80
#define NOPL_MAX_OP_STACK        200
#define NOPL_MAX_LOCAL           128
#define NOPL_MAX_GLOBAL          128

#define NOPL_MAX_FIELDS          16

#define MAX_QCODE_HEADER         60000
#define NOBJ_TRUE                0xFFFF
#define NOBJ_FALSE               0x0000

#define SIZEOF_INT 2
#define SIZEOF_NUM 8

typedef uint8_t NOBJ_PARAMETER_TYPE;
typedef int16_t NOBJ_INT;

typedef struct _NOBJ_VAR_SPACE_SIZE
{
  uint16_t size;
} NOBJ_VAR_SPACE_SIZE;

typedef struct _NOBJ_QCODE_SPACE_SIZE
{
  uint16_t size;
} NOBJ_QCODE_SPACE_SIZE;

typedef struct _NOBJ_NUM_PARAMETERS
{
  uint8_t num;
} NOBJ_NUM_PARAMETERS;

typedef struct _NOBJ_GLOBAL_VARNAME_SIZE
{
  uint16_t size;
} NOBJ_GLOBAL_VARNAME_SIZE;

typedef struct _NOBJ_EXTERNAL_VARNAME_SIZE
{
  uint16_t size;
} NOBJ_EXTERNAL_VARNAME_SIZE;

typedef struct _NOBJ_STRLEN_FIXUP_SIZE
{
  uint16_t size;
} NOBJ_STRLEN_FIXUP_SIZE;

typedef struct _NOBJ_ARYSZ_FIXUP_SIZE
{
  uint16_t size;
} NOBJ_ARYSZ_FIXUP_SIZE;

typedef uint8_t NOBJ_VARTYPE;

enum
  {
   NOBJ_VARTYPE_INT      = 0x00,
   NOBJ_VARTYPE_FLT      = 0x01,
   NOBJ_VARTYPE_STR      = 0x02,
   NOBJ_VARTYPE_INTARY   = 0x03,
   NOBJ_VARTYPE_FLTARY   = 0x04,
   NOBJ_VARTYPE_STRARY   = 0x05,
   NOBJ_VARTYPE_VAR_ADDR = 0x06,  // Address of a variable
   NOBJ_VARTYPE_UNKNOWN  = 0x10,
   NOBJ_VARTYPE_VOID     = 0x11,
  };

typedef uint16_t NOBJ_ADDR;

typedef struct _NOBJ_VARNAME_ENTRY
{
  uint8_t len;
  uint8_t varname[NOBJ_VARNAME_MAXLEN];
  NOBJ_VARTYPE  type;
  NOBJ_ADDR  address;
} NOBJ_VARNAME_ENTRY;

typedef uint8_t NOBJ_STRLEN_FIXUP_LEN;

typedef struct _NOBJ_STRLEN_FIXUP_ENTRY
{
  NOBJ_ADDR           address;
  NOBJ_STRLEN_FIXUP_LEN  len;
} NOBJ_STRLEN_FIXUP_ENTRY;

typedef uint16_t NOBJ_ARYSZ_FIXUP_LEN;

typedef struct _NOBJ_ARYSZ_FIXUP_ENTRY
{
  NOBJ_ADDR              address;
  NOBJ_ARYSZ_FIXUP_LEN   len;
} NOBJ_ARYSZ_FIXUP_ENTRY;

typedef uint8_t NOBJ_QCODE;

typedef struct _NOBJ_PROC
{
  NOBJ_VAR_SPACE_SIZE        var_space_size;
  NOBJ_QCODE_SPACE_SIZE      qcode_space_size;
  NOBJ_NUM_PARAMETERS        num_parameters;
  NOBJ_PARAMETER_TYPE        parameter_types[NOBJ_MAX_PARAMETERS];
  NOBJ_GLOBAL_VARNAME_SIZE   global_varname_size;
  int                        global_varname_num;
  NOBJ_VARNAME_ENTRY         global_varname[NOBJ_GLOBAL_VARNAME_SPACE_MAX];
  NOBJ_EXTERNAL_VARNAME_SIZE external_varname_size;
  int                        external_varname_num;
  NOBJ_VARNAME_ENTRY         external_varname[NOBJ_EXTERNAL_VARNAME_SPACE_MAX];
  NOBJ_STRLEN_FIXUP_SIZE     strlen_fixup_size;
  int                        strlen_fixup_num;
  NOBJ_STRLEN_FIXUP_ENTRY    strlen_fixup[NOBJ_STRLEN_FIXUP_MAX];
  NOBJ_ARYSZ_FIXUP_SIZE      arysz_fixup_size;
  int                        arysz_fixup_num;
  NOBJ_STRLEN_FIXUP_ENTRY    arysz_fixup[NOBJ_ARYSZ_FIXUP_MAX];
  NOBJ_QCODE                 *qcode;  
} NOBJ_PROC;

typedef uint16_t NOBJ_SP;

// An execution machine. This is what the QCode executes in
     
typedef struct _NOBJ_MACHINE
{
  // The machine stack where PROCs are loaded and executed
  uint8_t stack[NOBJ_MACHINE_STACK_SIZE];
  
  // Stack pointer (where next byte will be loaded
  NOBJ_SP rta_sp;
  int rta_pc;
  int rta_fp;
  int rtb_trap;
  int cursor_flag;
  int clock_flag;
  int rta_escf;
  int onerr_handler;
  int rtb_eror;
  int error_occurred;
  char error_string[80];
} NOBJ_MACHINE;


////////////////////////////////////////////////////////////////////////////////
// Shunting algorithm operator stack
// Name is the token string
// type is the type of the expression that is being parsed, nbot this
// entry specifically. It is determined as each expression is processed
// For instance, it starts unknown and could then be INT, then move to FLT
// if a decimal point is seen.
// It is needed for tokens like '=' to be selected correctly, as there are
// different tokens for string = and int =
//
//
// Information about a variable
//
////////////////////////////////////////////////////////////////////////////////

#define VAR_DECLARE     0
#define VAR_REF         1
#define VAR_PARAMETER   2
#define VAR_FIELD       3
#define NUM_CALC_MEMORY 10

typedef enum _NOPL_VAR_CLASS
  {
    NOPL_VAR_CLASS_UNKNOWN,
    NOPL_VAR_CLASS_LOCAL,
    NOPL_VAR_CLASS_GLOBAL,
    NOPL_VAR_CLASS_EXTERNAL,
    NOPL_VAR_CLASS_CALC_MEMORY,
    NOPL_VAR_CLASS_PARAMETER,
    NOPL_VAR_CLASS_CREATE,
    NOPL_VAR_CLASS_OPEN,
    NOPL_VAR_CLASS_FIELDVAR,
    
  } NOPL_VAR_CLASS;

typedef enum _NOPL_OP_ACCESS
  {
    NOPL_OP_ACCESS_UNKNOWN,
    NOPL_OP_ACCESS_READ,
    NOPL_OP_ACCESS_WRITE,
    NOPL_OP_ACCESS_EXP,
    NOPL_OP_ACCESS_NO_EXP,
    NOPL_OP_ACCESS_WRITE_ARRAY_STK_IDX,  // For pop index and stack array reference (Flist)
    NOPL_OP_ACCESS_FIELDVAR,
  } NOPL_OP_ACCESS;

typedef struct _NOBJ_VAR_INFO
{
  char name[NOBJ_VARNAME_MAXLEN];
  int is_ref;     // Reference or declare
  NOPL_VAR_CLASS class;
  NOBJ_VARTYPE type;
  int max_array;
  int max_string;
  int num_indices;

  uint16_t offset;    // Offset from FP
} NOBJ_VAR_INFO;

#define MAX_VAR_INFO  300

typedef struct _NOBJ_COND_INFO
{
  char name[NOBJ_VARNAME_MAXLEN];
  int cond_type;              // Type of conditional
  int cond_idx;               // Index of conditional (label)
} NOBJ_COND_INFO;

typedef struct _OP_STACK_ENTRY
{
  char                    name[256];  // Needs to be large as we put string literals here
  int                     buf_id;
  NOBJ_INT                integer;

  NOBJ_VARTYPE            type;       // Original type
  //  NOBJ_VARTYPE            req_type;   // Required type
  NOBJ_VARTYPE            qcode_type; // Type the QCode needs to be (based on inputs for
                                      // forced operators like <=. They need to have an
                                      // output type that is forced (int for <=) but
                                      // use a FLT type if the inputs are FLT
  struct _NOBJ_VAR_INFO   vi;
  struct _NOBJ_COND_INFO  ci;
  // Bytes may be added after the qcode
  int                     num_bytes;
  int                     bytes[NOPL_MAX_SUFFIX_BYTES];
  int                     level;                 // Used for conditionals
  int                     num_parameters;        // Number of parameters of a function
  NOBJ_VARTYPE            parameter_type[NOBJ_MAX_PARAMETERS];
  NOPL_OP_ACCESS          access;                // Read or write (variables)
                                                 // EXP or NO_EXP for RETURN
  int                     percent;               // Is this a percentage?
  int                     trapped;               // Non zero if this is a trapped command and we need
                                                 // to generate a TRAP QCode

} OP_STACK_ENTRY;

#define MAX_EXP_BUF_P   20


typedef struct _EXP_BUFFER_ENTRY
{
  char name[255];
  OP_STACK_ENTRY op;
  int node_id;               // Node ID of this entry
  int p_idx;                 // Number of pointers back to argument nodes
  int p[MAX_EXP_BUF_P];      // Node id of each node we point to
  int nxt;
} EXP_BUFFER_ENTRY;

