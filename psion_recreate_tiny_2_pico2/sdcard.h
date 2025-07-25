
void sdcard_init(void);

void ls(const char *dir);
void run_unmount(void);
void run_mount(void);
void run_getfree();
void run_mkdir(char *arg1);
void run_cd(char *arg1);
void run_cat(char *filename);
void run_info(void);
bool sd_init_driver();
          
//void run_mount(const size_t argc, const char *argv[]);
