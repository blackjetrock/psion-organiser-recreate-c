uint8_t pk_rbyt_pico_flash(PAK_ADDR pak_addr);
void pk_save_pico_flash(PAK_ADDR pak_addr, int len, uint8_t *src);
void pk_format_pico_flash(int logfile);
extern PAK_ADDR flash_pak_base_read;
int pk_exist_pico_flash(char *filename);
void pk_close_pico_flash(int logfile, char *filename);
void pk_create_pico_flash(int logfile, char *filename);
void pk_open_pico_flash(int logfile, char *filename);

