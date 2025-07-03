////////////////////////////////////////////////////////////////////////////////

#define NUM_CALC_MEMS 10

// Calculator memoruies are located in the stack memory space as QCodes
// need to access them

#define CALC_MEM_START 0x0000

typedef struct _NOPL_CALC
{

} NOPL_CALC;

extern NOPL_CALC calc;
