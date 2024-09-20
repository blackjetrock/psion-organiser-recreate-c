#sudo kill `pgrep openocd`

# Old picoprobe version
#sudo openocd -f interface/picoprobe.cfg -f target/rp2040.cfg -c "program /tree/projects/github/psion-org2-recreate/psion-recreate/build/psion_recreate.elf verify reset exit"

sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program psion_recreate_ctusb.elf verify reset exit"
