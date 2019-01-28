
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>

#include "linux/i2c-dev.h"

#include "as7265x.h"

//============================================================
// Low level I2C
//============================================================

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

void as7265x_clear_status(int i2c_drv_node)
{
    int status;

    do{
        status = i2c_smbus_read_byte_data(i2c_drv_node, I2C_AS72XX_SLAVE_STATUS_REG);
        if ((status & I2C_AS72XX_SLAVE_RX_VALID) != 0){
            // Issue dummy read to clear the RX_VALID bit.
            i2c_smbus_read_byte_data(i2c_drv_node, I2C_AS72XX_SLAVE_READ_REG);
        }
    } while((status & I2C_AS72XX_SLAVE_RX_VALID) != 0);
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
    usleep(2000);

    as7265x_wait_tx_valid(i2c_drv_node);

    int retval = i2c_smbus_write_byte_data(i2c_drv_node, I2C_AS72XX_SLAVE_WRITE_REG, reg_addr);

    as7265x_wait_rx_valid(i2c_drv_node);

    int rd_data = i2c_smbus_read_byte_data(i2c_drv_node, I2C_AS72XX_SLAVE_READ_REG);

    return rd_data;
}

void as7265x_wr_reg(int i2c_drv_node, int reg_addr, int reg_data)
{
    usleep(2000);

    as7265x_wait_tx_valid(i2c_drv_node);

    int retval = i2c_smbus_write_byte_data(i2c_drv_node, I2C_AS72XX_SLAVE_WRITE_REG, (reg_addr | 0x80));

    as7265x_wait_tx_valid(i2c_drv_node);

    retval = i2c_smbus_write_byte_data(i2c_drv_node, I2C_AS72XX_SLAVE_WRITE_REG, reg_data);
}

//============================================================
// Higher level api
//============================================================

void as7265x_dev_sel(int i2c_drv_node, int dev_nr)
{
    as7265x_wr_reg(i2c_drv_node, AS72651_DEV_SEL, dev_nr);
}

void as7265x_init(int i2c_drv_node, int gain, int mode, int int_time)
{
    as7265x_wr_reg(i2c_drv_node, AS72651_LED_CONFIG, 0x00);
    as7265x_wr_reg(i2c_drv_node, AS72651_CONTROL_SETUP, (gain << 4) | (mode << 2));
    as7265x_wr_reg(i2c_drv_node, AS72651_INT_TIME, int_time);
}


void as7265x_revision(int i2c_drv_node, int *hw_version, int *sw_major_version, int *sw_patch_version, int *sw_build_version)
{
#if 1
    // HW VERSION
    int hw_version_h = as7265x_rd_reg(i2c_drv_node, AS72651_DEVICE_TYPE);
    int hw_version_l = as7265x_rd_reg(i2c_drv_node, AS72651_HW_VERSION);

    printf("hw_version: 0x%02x, 0x%02x\n", hw_version_h, hw_version_l);

    if (hw_version) *hw_version = (hw_version_h <<8) | hw_version_l;
#endif

#if 1
    // SW MAJOR VERSION
    as7265x_wr_reg(i2c_drv_node, AS72651_FW_VERSION_H, 0x01);
    int sw_major_version_h  = as7265x_rd_reg(i2c_drv_node, AS72651_FW_VERSION_H);

    as7265x_wr_reg(i2c_drv_node, AS72651_FW_VERSION_L, 0x01);
    int sw_major_version_l  = as7265x_rd_reg(i2c_drv_node, AS72651_FW_VERSION_L);

    printf("sw_major_version: 0x%02x, 0x%02x\n", sw_major_version_h, sw_major_version_l);

    if (sw_major_version) *sw_major_version = (sw_major_version_h <<8) | sw_major_version_l;

    // SW PATCH VERSION
    as7265x_wr_reg(i2c_drv_node, AS72651_FW_VERSION_H, 0x02);
    int sw_patch_version_h  = as7265x_rd_reg(i2c_drv_node, AS72651_FW_VERSION_H);

    as7265x_wr_reg(i2c_drv_node, AS72651_FW_VERSION_L, 0x02);
    int sw_patch_version_l  = as7265x_rd_reg(i2c_drv_node, AS72651_FW_VERSION_L);

    printf("sw_patch_version: 0x%02x, 0x%02x\n", sw_patch_version_h, sw_patch_version_l);

    if (sw_patch_version) *sw_patch_version = (sw_patch_version_h <<8) | sw_patch_version_l;

    // SW BUILD VERSION
    as7265x_wr_reg(i2c_drv_node, AS72651_FW_VERSION_H, 0x03);
    int sw_build_version_h  = as7265x_rd_reg(i2c_drv_node, AS72651_FW_VERSION_H);

    as7265x_wr_reg(i2c_drv_node, AS72651_FW_VERSION_L, 0x03);
    int sw_build_version_l  = as7265x_rd_reg(i2c_drv_node, AS72651_FW_VERSION_L);

    printf("sw_build_version: 0x%02x, 0x%02x\n", sw_build_version_h, sw_build_version_l);

    if (sw_build_version) *sw_build_version = (sw_build_version_h <<8) | sw_build_version_l;
#endif

}

void as7265x_set_indicator_led(int i2c_drv_node, int dev_nr, int enable)
{
    as7265x_dev_sel(i2c_drv_node, dev_nr);

    int rd_data = as7265x_rd_reg(i2c_drv_node, AS72651_LED_CONFIG);
    rd_data = rd_data & (~0x01) | enable | 0x02;
    as7265x_wr_reg(i2c_drv_node, AS72651_LED_CONFIG, rd_data);
}

void as7265x_read_and_print(int i2c_drv_node, int reg_addr)
{
    int rd_data;
    rd_data = as7265x_rd_reg(i2c_drv_node, reg_addr);
    printf("addr: 0x%02x = 0x%02x\n", reg_addr, rd_data);
}

void as7265x_coef_read(int i2c_drv_node, int coef_read)
{
    int rd_data;

    as7265x_wr_reg(i2c_drv_node, 0x54, 0x55);
    rd_data = as7265x_rd_reg(i2c_drv_node, 0x54);
    printf("wr: 0x55, rd: 0x%02x\n", rd_data);

    as7265x_wr_reg(i2c_drv_node, 0x54, 0xaa);
    rd_data = as7265x_rd_reg(i2c_drv_node, 0xaa);
    printf("wr: 0x55, rd: 0x%02x\n", rd_data);
}

void as7265x_read_raw_data(int i2c_drv_node, int16_t * destination)
{
    uint8_t rawData[2];

    // collect R,S,T,U,V, W data
    as7265x_wr_reg(i2c_drv_node, AS72651_DEV_SEL, 0);
    for(int i = 0; i < 6; i++) {
        for(int j = 0; j < 2; j++) {
            rawData[j] = as7265x_rd_reg(i2c_drv_node, AS72651_RAW_VALUE_0_H + 2*i + j);
        }

        destination[i] = (int16_t) ( ((int16_t) rawData[0] << 8) | rawData[1]);
    }

    // collect J,I,G,H,K, L data
    as7265x_wr_reg(i2c_drv_node, AS72651_DEV_SEL, 1);
    for(int i = 0; i < 6; i++) {
        for(int j = 0; j < 2; j++) {
            rawData[j] = as7265x_rd_reg(i2c_drv_node, AS72651_RAW_VALUE_0_H + 2*i + j);
        }

        destination[i + 6] = (int16_t) ( ((int16_t) rawData[0] << 8) | rawData[1]);
    }

    // collect D,C,A,B,E, F data
    as7265x_wr_reg(i2c_drv_node, AS72651_DEV_SEL, 2);
    for(int i = 0; i < 6; i++) {
        for(int j = 0; j < 4; j++) {
            rawData[j] = as7265x_rd_reg(i2c_drv_node, AS72651_RAW_VALUE_0_H + 2*i + j);
        }

        destination[i + 12] = (int16_t) ( ((int16_t) rawData[0] << 8) | rawData[1]);
    }

}


void as7265x_read_cal_data(int i2c_drv_node, float * destination)
{
    uint8_t rawData[4];

    // collect R,S,T,U,V, W data
    as7265x_wr_reg(i2c_drv_node, AS72651_DEV_SEL, 0);
    for(int i = 0; i < 6; i++) {
        for(int j = 0; j < 4; j++) {
            rawData[j] = as7265x_rd_reg(i2c_drv_node, AS72651_CAL_CHAN0_0 + 4*i + j);
        }

        uint32_t x = ((uint32_t) rawData[0] << 24) | ((uint32_t) rawData[1] << 16) | ((uint32_t) rawData[2] << 8) | rawData[3];
        destination[i] = *(float*)&x;
    }

    // collect J,I,G,H,K, L data
    as7265x_wr_reg(i2c_drv_node, AS72651_DEV_SEL, 1);
    for(int i = 0; i < 6; i++) {
        for(int j = 0; j < 4; j++) {
            rawData[j] = as7265x_rd_reg(i2c_drv_node, AS72651_CAL_CHAN0_0 + 4*i + j);
        }

        uint32_t x = ((uint32_t) rawData[0] << 24) | ((uint32_t) rawData[1] << 16) | ((uint32_t) rawData[2] << 8) | rawData[3];
        destination[i + 6] = *(float*)&x;
    }

    //collect D,C,A,B,E, F data
    as7265x_wr_reg(i2c_drv_node, AS72651_DEV_SEL, 2);
    for(int i = 0; i < 6; i++) {
        for(int j = 0; j < 4; j++) {
            rawData[j] = as7265x_rd_reg(i2c_drv_node, AS72651_CAL_CHAN0_0 + 4*i + j);
        }

        uint32_t x = ((uint32_t) rawData[0] << 24) | ((uint32_t) rawData[1] << 16) | ((uint32_t) rawData[2] << 8) | rawData[3];
        destination[i + 12] = *(float*)&x;
    }

}

int main(int argv, char **argc)
{
    int node = as7265x_i2c_drv_open(6);
    printf("node: %d\n", node);
    as7265x_i2c_dev_addr_set(node, 0x49);

    as7265x_revision(node, NULL, NULL, NULL, NULL);

    as7265x_init(node, GAIN_16X, MODE2, 36);

    as7265x_set_indicator_led(node, 0, 0);
    usleep(200000);
    as7265x_set_indicator_led(node, 0, 1);
    usleep(200000);
    as7265x_set_indicator_led(node, 0, 0);
    usleep(200000);
    as7265x_set_indicator_led(node, 0, 1);
    usleep(200000);
    as7265x_set_indicator_led(node, 0, 0);

    //                   0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17
    uint16_t freqs[18] = {610, 680, 730, 760, 810, 860, 560, 585, 645, 705, 900, 940, 410, 435, 460, 485, 510, 535};
    int freq_order[18] = { 12, 13, 14, 15, 16, 17, 6, 7, 0, 8, 1, 9, 2, 3, 4, 5, 10, 11 };

    for(int i=0;i<18;++i){
        int freq = freqs[freq_order[i]];
        printf("%10d, ", freq);
    }
    printf("\n\n");

    while(1){
        float cal_data[18];
        as7265x_read_cal_data(node, cal_data);

        for(int i=0;i<18;++i){
            float val = cal_data[freq_order[i]];
            printf("%10.4f, ", val);
        }
        printf("\n");
        sleep(1);
    }

}
