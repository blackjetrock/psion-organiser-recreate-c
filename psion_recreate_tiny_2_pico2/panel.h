////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////


#define MAX_ITEMS  10


typedef void (*FN_PANEL_INIT)(void);
typedef void (*FN_PANEL_TASKS)(void);
typedef void (*FN_PANEL_ITEM)(void);

typedef enum
  {
    PANEL_ITEM_TYPE_INT = 1,
    PANEL_ITEM_TYPE_FN,
  } PANEL_ITEM_TYPE;

typedef struct
{
  int x;
  int y;
  int length;
  char *title;
  PANEL_ITEM_TYPE type;
  union
  {
    int *value;
    FN_PANEL_ITEM fn;
  };
} PANEL_ITEM_T;

typedef struct
{
  FN_PANEL_INIT init_fn;
  PANEL_ITEM_T items[MAX_ITEMS];
} PANEL_T;

void do_panel(PANEL_T *panel, int itf, int cls);

