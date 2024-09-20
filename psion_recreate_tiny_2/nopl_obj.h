// New OPL object code definitions.
//
// This is backwards compatible with QCode where possible.
//

typedef uint8_t NOBJ_PARAMETER_TYPE;

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
   NOBJ_VARTYPE_INT    = 0x00,
   NOBJ_VARTYPE_FLT    = 0x01,
   NOBJ_VARTYPE_STR    = 0x02,
   NOBJ_VARTYPE_INTARY = 0x03,
   NOBJ_VARTYPE_FLTARY = 0x04,
   NOBJ_VARTYPE_STRARY = 0x05,
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

// An execution machine. This is what the QCode executes in
     
typedef struct _NOBJ_MACHINE
{
  // The machine stack where PROCs are loaded
  uint8_t stack[NOBJ_MACHINE_STACK_SIZE];
  
  // Stack pointer (where next byte will be loaded
  int sp;
} NOBJ_MACHINE;




