void __not_in_flash_func(core1_code)(void);
void take_core1_out_of_ram(void);
void put_core1_into_ram(void);
void __not_in_flash_func(core1_ram_wait)(void);
void core1_init(void);

extern volatile int core1_ticks;
