sudo chmod 0666 /dev/ttyUSB0

#esptool.py erase_flash

esptool.py write_flash 0x00000 ../bin/eagle.flash.bin 0x20000 ../bin/eagle.irom0text.bin 0x3FC000 ../bin/esp_init_data_default.bin

sudo gtkterm -p /dev/ttyUSB0 -s 115200
