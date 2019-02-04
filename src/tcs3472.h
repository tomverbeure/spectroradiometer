#ifndef TCS3472_H
#define TCS3472_H

#define TCS3472_ADDRESS                 0x29

//============================================================
// Low level I2C
//============================================================

int tcs3472_i2c_drv_open(int i2c_port_nr);
int tcs3472_i2c_dev_addr_set(int i2c_drv_node, int i2c_dev_addr);

#endif
