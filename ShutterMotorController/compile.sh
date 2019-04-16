make COMPILE=gcc BOOT=none APP=0 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=4

#sudo chmod 0666 /dev/ttyUSB0 && esptool.py -p /dev/ttyUSB0 -b 460800 write_flash --flash_mode qio --flash_size detect --verify --flash_freq 40m 0x00000 ../bin/blank.bin 0x10000 ../bin/blank.bin 0xFC000 ../bin/esp_init_data_default.bin 0x7e000 ../bin/blank.bin 0xfe000 ../bin/blank.bin

##sudo chmod 0666 /dev/ttyUSB0 && esptool.py -p /dev/ttyUSB0 -b 460800 write_flash --flash_mode qio --flash_size detect --verify --flash_freq 40m 0x00000 ../bin/eagle.flash.bin 0x10000 ../bin/eagle.irom0text.bin 0xFC000 ../bin/esp_init_data_default.bin 0x7e000 ../bin/blank.bin 0xfe000 ../bin/blank.bin

#sudo chmod 0666 /dev/ttyUSB0 && esptool.py -p /dev/ttyUSB0 -b 57600 write_flash --flash_mode qio --flash_size detect --verify --flash_freq 40m 0x00000 ../bin/eagle.flash.bin 0x10000 ../bin/eagle.irom0text.bin

#sudo chmod 0666 /dev/ttyUSB0 && esptool.py -p /dev/ttyUSB0 -b 115200 write_flash --flash_mode qio --flash_size detect --verify --flash_freq 40m 0x00000 ../bin/at/noboot/eagle.flash.bin 0x10000 ../bin/at/noboot/eagle.irom0text.bin 0x7e000 ../bin/blank.bin 0x1fe000 ../bin/blank.bin

#sudo chmod 0666 /dev/ttyUSB0 && esptool.py -p /dev/ttyUSB0 -b 115200 dump_mem 0x0 
#sudo putty /dev/ttyUSB0 -serial -sercfg 115200,8,n,1,n


##sudo gtkterm -p /dev/ttyUSB0 -s 115200
