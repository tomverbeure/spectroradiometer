
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "linux/i2c-dev.h"

#define I2C_AS72XX_SLAVE_STATUS_REG 0x00
#define I2C_AS72XX_SLAVE_WRITE_REG 0x01
#define I2C_AS72XX_SLAVE_READ_REG 0x02

#define I2C_AS72XX_SLAVE_TX_VALID 0x02
#define I2C_AS72XX_SLAVE_RX_VALID 0x01

int as7265x_i2c_drv_open(int i2c_port_nr)
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

int as7265x_i2c_dev_addr_set(int i2c_drv_node, int i2c_dev_addr)
{
	int ret_val = ioctl(i2c_drv_node, I2C_SLAVE, i2c_dev_addr);
	if (ret_val < 0) {
		perror("Could not set I2C_SLAVE.");
		exit(2);
	}
}

void as7265x_wait_tx_valid(int i2c_drv_node)
{
    int status;

    do{
        status = i2c_smbus_read_byte_data(i2c_drv_node, I2C_AS72XX_SLAVE_STATUS_REG);
    } while((status & I2C_AS72XX_SLAVE_TX_VALID) != 0);
}

void as7265x_wait_rx_valid(int i2c_drv_node)
{
    int status;

    do{
        status = i2c_smbus_read_byte_data(i2c_drv_node, I2C_AS72XX_SLAVE_STATUS_REG);
    } while((status & I2C_AS72XX_SLAVE_RX_VALID) == 0);
}


int as7265x_rd_reg(int i2c_drv_node, int reg_addr)
{
    as7265x_wait_tx_valid(i2c_drv_node);

    int retval = i2c_smbus_write_byte_data(i2c_drv_node, I2C_AS72XX_SLAVE_WRITE_REG, reg_addr);

    as7265x_wait_rx_valid(i2c_drv_node);

    int rd_data = i2c_smbus_read_byte_data(i2c_drv_node, I2C_AS72XX_SLAVE_READ_REG);

    return rd_data;
}

int main(int argv, char **argc)
{
    int node = as7265x_i2c_drv_open(6);

    printf("node: %d\n", node);

    as7265x_i2c_dev_addr_set(node, 0x49);

    int hw_version_h = as7265x_rd_reg(node, 0x00);
    int hw_version_l = as7265x_rd_reg(node, 0x01);

    printf("hw_version: 0x%02x, 0x%02x\n", hw_version_h, hw_version_l);

}
