#ifndef TCS34725_H
#define TCS34725_H

#define TCS34725_ADDRESS                0x29

#define TCS34725_ENABLE                 0x00
#define TCS34725_ATIME                  0x01
#define TCS34725_WTIME                  0x03
#define TCS34725_AILTL                  0x04
#define TCS34725_AILTH                  0x05
#define TCS34725_AIHTL                  0x06
#define TCS34725_AIHTH                  0x07
#define TCS34725_PERS                   0x0c
#define TCS34725_CONFIG                 0x0d
#define TCS34725_CONTROL                0x0f
#define TCS34725_ID                     0x12
#define TCS34725_STATUS                 0x13
#define TCS34725_CDATAL                 0x14
#define TCS34725_CDATAH                 0x15
#define TCS34725_RDATAL                 0x16
#define TCS34725_RDATAH                 0x17
#define TCS34725_GDATAL                 0x18
#define TCS34725_GDATAH                 0x19
#define TCS34725_BDATAL                 0x1a
#define TCS34725_BDATAH                 0x1b


//============================================================
// Low level I2C
//============================================================

int tcs34725_i2c_drv_open(int i2c_port_nr);
int tcs34725_i2c_dev_addr_set(int i2c_drv_node, int i2c_dev_addr);
int tcs34725_rd_reg8(int i2c_drv_node, int reg_addr);
int tcs34725_rd_reg16(int i2c_drv_node, int reg_addr);
int tcs34725_wr_reg8(int i2c_drv_node, int reg_addr, int reg_data);


#endif
