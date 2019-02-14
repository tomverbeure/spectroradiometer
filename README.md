**This is very much work in progress! Don't expect anything to simply work out of the box!**

# Description

This project explores the possibilities of the AMS AS7265x set of 3 sensors and compares how it performs
against a true spectroradiometer.


The overall setup is extremely simple:

* [Compact AS7265X Spectrometer](https://www.tindie.com/products/onehorse/compact-as7265x-spectrometer/) from Tindie.
* [STM32 Blue Pill](https://wiki.stm32duino.com/index.php?title=Blue_Pill)

    The well known dirt cheap (less than $2) controller which has a USB device interface and an I2C master (among many other things.)

* [STM32 I2C Star]([https://github.com/daniel-thompson/i2c-star) firmware

    This firmware implements a USB to I2C bridge. There is a Linux driver that supports this bridge
    that comes standard with the Linux kernel.

    After compiling, I flashed the firmware with an STLINK dongle.

Here's [a breadboard example](./assets/Breadboard Prototype.JPG) with the full setup.


# First Contact Testing



# Various information:

* AMS TCS34725 Color Sensor

    * [Datasheet](https://ams.com/tcs34725)
    * [Adafruit library](https://github.com/adafruit/Adafruit_TCS34725)

* AMS AS7265x Spectral Sensor

    * [Datasheet](https://ams.com/as7265x)


General color info

* [Taos Colorimetry Tutorial](https://ams.com/documents/20143/36005/LightSensors_AN000519_1-00.pdf/fb874ce7-0c27-0f38-0582-9cd564d38fdd)
