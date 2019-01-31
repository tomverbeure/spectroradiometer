
I2C_BUS := $(shell i2cdetect -l | grep "i2c-tiny-usb" | cut -c5-6)

as7265x: ./src/as7265x.c ./src/main.c ./src/specrend.c
	gcc -o $@ -I ./src $^ -lm

run: as7265x
	./as7265x $(I2C_BUS)

.PHONY: clean
clean:
	rm -f *.o as7265x
