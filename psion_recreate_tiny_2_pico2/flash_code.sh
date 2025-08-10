sudo kill `pgrep openocd`
cd /home/menadue/tree/github/openocd
sudo src/openocd -s tcl -f interface/cmsis-dap.cfg -f target/rp2350.cfg -c "program /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/psion_recreate_ctusb.elf verify reset exit"
