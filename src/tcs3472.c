#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>

#include "linux/i2c-dev.h"

#include "tcs3472.h"


//============================================================
// Low level I2C
//============================================================

int tcs3472_i2c_drv_open(int i2c_port_nr)
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

int tcs3472_i2c_dev_addr_set(int i2c_drv_node, int i2c_dev_addr)
{
	int ret_val = ioctl(i2c_drv_node, I2C_SLAVE, i2c_dev_addr);
	if (ret_val < 0) {
		perror("Could not set I2C_SLAVE.");
		exit(2);
	}
}

