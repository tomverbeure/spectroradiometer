
I2C_BUS := $(shell i2cdetect -l | grep "i2c-tiny-usb" | cut -c5-6)

as7265x: ./src/as7265x.c
	gcc -o $@ -I ./src $^

run: as7265x
	sudo ./as7265x $(I2C_BUS)
