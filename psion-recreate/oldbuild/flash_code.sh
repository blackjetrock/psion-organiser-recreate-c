sudo kill `pgrep openocd`
sudo openocd -f interface/picoprobe.cfg -f target/rp2040.cfg -c "program psion_recreate.elf verify reset exit"
