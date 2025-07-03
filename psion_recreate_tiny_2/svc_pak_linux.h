

typedef struct
{
  FILE *fp;             // File pointer to use when accessing
                        // the files on the Linux filesystem
} LINUX_FILE_INFO;

extern LINUX_FILE_INFO linux_file_info[NOPL_NUM_LOGICAL_FILES];

uint8_t pk_rbyt_linux(PAK_ADDR pak_addr);
void pk_save_linux(PAK_ADDR pak_addr, int len, uint8_t *src);
void pk_format_linux(int logfile);
void pk_open_linux(int logfile, char *filename);
void pk_create_linux(int logfile, char *filename);
void pk_close_linux(int logfile, char *filename);
int pk_exist_linux(char *filename);
