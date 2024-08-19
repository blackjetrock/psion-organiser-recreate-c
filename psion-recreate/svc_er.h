typedef enum _ER_ERRORCODE
  {
   ER_FL_NP = -246,
  } ER_ERRORCODE;


char *er_lkup(ER_ERRORCODE e);
void er_mess(ER_ERRORCODE e);
