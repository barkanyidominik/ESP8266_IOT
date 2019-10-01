sudo chmod 0666 /dev/ttyUSB0 && esptool.py -p /dev/ttyUSB0 -b 460800 write_flash --flash_mode dio --flash_size detect --verify --flash_freq 40m 0x00000 ../bin/eagle.flash.bin 0x010000 ../bin/eagle.irom0text.bin 0x3FC000 ../bin/esp_init_data_default_v08.bin 0x3fe000 ../bin/blank.bin

sudo gtkterm -p /dev/ttyUSB0 -s 115200

