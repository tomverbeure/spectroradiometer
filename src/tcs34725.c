#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>
#include <sys/ioctl.h>

#include "linux/i2c-dev.h"

#include "tcs34725.h"


//============================================================
// Low level I2C
//============================================================

int tcs34725_i2c_drv_open(int i2c_port_nr)
{
    char i2c_dev_path[256];
    sprintf(i2c_dev_path, "/dev/i2c-%d", i2c_port_nr);

    int ret_val = 0;

    int i2c_drv_node = open(i2c_dev_path, O_RDWR);
    if (i2c_drv_node < 0)
    {
        perror("Unable to open device node.");
        exit(1);
    }

    return i2c_drv_node;
}

int tcs34725_i2c_dev_addr_set(int i2c_drv_node, int i2c_dev_addr)
{
	int ret_val = ioctl(i2c_drv_node, I2C_SLAVE, i2c_dev_addr);
	if (ret_val < 0) {
		perror("Could not set I2C_SLAVE.");
		exit(2);
	}
}

int tcs34725_rd_reg8(int i2c_drv_node, int reg_addr)
{
    int rd_data = i2c_smbus_read_byte_data(i2c_drv_node, (1<<7) | reg_addr);
}

int tcs34725_rd_reg16(int i2c_drv_node, int reg_addr)
{
    uint16_t rd_data = 0;
    i2c_smbus_read_i2c_block_data(i2c_drv_node, (1<<7) | (0<<5) | reg_addr, 2, (char *)&rd_data);

    return rd_data;
}

int tcs34725_wr_reg8(int i2c_drv_node, int reg_addr, int reg_data)
{
    i2c_smbus_write_byte_data(i2c_drv_node, (1<<7) | reg_addr, reg_data);
}

int tcs34725_id(int i2c_drv_node)
{
    int tcs_version = tcs34725_rd_reg8(i2c_drv_node, TCS34725_ID);

    return tcs_version;
}

void tcs34725_init(int i2c_drv_node, int gain, int atime)
{
    tcs34725_wr_reg8(i2c_drv_node, TCS34725_CONFIG, 0);
    tcs34725_wr_reg8(i2c_drv_node, TCS34725_CONTROL, gain);
    tcs34725_wr_reg8(i2c_drv_node, TCS34725_ATIME, atime);
    tcs34725_wr_reg8(i2c_drv_node, TCS34725_WTIME, 0);

    tcs34725_wr_reg8(i2c_drv_node, TCS34725_ENABLE, 1);
    usleep(3000);
    tcs34725_wr_reg8(i2c_drv_node, TCS34725_ENABLE, 3);
}

void tcs34725_get_data(int i2c_drv_node, uint16_t *c, uint16_t *r, uint16_t *g, uint16_t *b)
{
    *c = tcs34725_rd_reg16(i2c_drv_node, TCS34725_CDATAL);
    *r = tcs34725_rd_reg16(i2c_drv_node, TCS34725_RDATAL);
    *g = tcs34725_rd_reg16(i2c_drv_node, TCS34725_GDATAL);
    *b = tcs34725_rd_reg16(i2c_drv_node, TCS34725_BDATAL);
}

void tcs34725_fill_dev_identify(int i2c_drv_node, struct tcs34725_dev_identity *di)
{
    di->id = tcs34725_id(i2c_drv_node);
}

void tcs34725_fill_measurement_settings(int i2c_drv_node, struct tcs34725_measurement_settings *ms)
{
    int gain_hw = tcs34725_rd_reg8(i2c_drv_node, TCS34725_CONTROL) & 3;

    ms->gain = gain_hw == 0 ? 1  :
               gain_hw == 1 ? 4  :
               gain_hw == 2 ? 16 :
                              64 ;

    int atime_hw = tcs34725_rd_reg8(i2c_drv_node, TCS34725_ATIME);

    ms->atime_ms = 2.4f * (256 - atime_hw);
}

void tcs34725_fill_measurement(int i2c_drv_node, struct tcs34725_measurement *m)
{
    time(&m->timestamp);
    tcs34725_get_data(i2c_drv_node, &m->c, &m->r, &m->g, &m->b);
}


