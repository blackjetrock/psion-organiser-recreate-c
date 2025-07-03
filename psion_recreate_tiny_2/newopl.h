#define NOBJ_MAX_PROCNAME                  16
#define NOBJ_MAX_PARAMETERS               255

// Maximum variable name length
// We have 8 chracaters for the name, plus a suffix for type. This can be:
// %,$
// ADDR uses (no index values):
// %()
// $()
//
// LOCAL and GLOBAL use array names with indices, which is considerably
// longer, so we use a different length to save RAM space where we don't need
// that storage

#define NOBJ_VARNAME_MAXLEN                 (8+1+1+2)
#define NOBJ_DECLARE_VARNAME_MAXLEN         (8+1+1+2+6+1+6)

#define NOBJ_FILENAME_MAXLEN               12
#define NOBJ_STRING_MAXLEN                 256
#define NOBJ_GLOBAL_VARNAME_SPACE_MAX      64
#define NOBJ_EXTERNAL_VARNAME_SPACE_MAX    64
#define NOBJ_STRLEN_FIXUP_MAX              64
#define NOBJ_ARYSZ_FIXUP_MAX               64
#define NOBJ_MAX_QCODE_SIZE             65535

//#define NOBJ_MACHINE_STACK_SIZE         65535
#define NOBJ_MACHINE_STACK_SIZE         0x3eff
#define NOBJ_MACHINE_STACK_LOW          0x0100
#define NOBJ_MACHINE_STACK_HIGH         0x3f00

#define NOBJ_PRT_MAX_LINE                 800


