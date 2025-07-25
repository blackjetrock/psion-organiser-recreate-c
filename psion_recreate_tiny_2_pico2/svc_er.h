typedef enum _ER_ERRORCODE
  {
       ER_FL_NPXX = -246,
  } ER_ERRORCODE;

void er_error(char *errstr);
char *er_lkup(ER_ERRORCODE e);
void er_mess(ER_ERRORCODE e);
