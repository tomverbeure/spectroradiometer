
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>

#include "as7265x.h"

int main(int argv, char **argc)
{
    if (argv != 2){
        printf("Usage: as7265x <i2c bus nr>\n");
        exit(1);
    }
    printf("Bus: %d\n", atoi(argc[1]));

    int node = as7265x_i2c_drv_open(atoi(argc[1]));
    //int node = as7265x_i2c_drv_open(10);
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
        printf("%8d,", freq);
    }
    printf("\n\n");

    while(1){
        float cal_data[18];
        as7265x_read_cal_data(node, cal_data);

        for(int i=0;i<18;++i){
            float val = cal_data[freq_order[i]];
            printf("%8.4f,", val);
        }
        printf("\n");
    }

}
