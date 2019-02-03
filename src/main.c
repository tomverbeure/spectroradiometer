
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>

#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <getopt.h>

#include "as7265x.h"
#include "specrend.h"
#include "tcp_server.h"

// Command line options
int i2c_bus = -1;
int verbose_flag = 0;
int server_flag = 0;
int port_nr = 5000;

void buckets_to_XYZ(float cal_data[18], double *X, double *Y, double *Z)
{
    *X = 0.0;
    *Y = 0.0;
    *Z = 0.0;

    for(int i=0;i<18;++i){
        int   freq = freqs[freq_order[i]];
        float val  = cal_data[freq_order[i]];

        int entry = (freq - 380)/5;
        if (entry > 80)
            continue;

        *X += cie_colour_match[entry][0] * val;
        *Y += cie_colour_match[entry][1] * val;
        *Z += cie_colour_match[entry][2] * val;
    }
}

void buckets_to_xyz(float cal_data[18], double *x, double *y, double *z)
{
    double X,Y,Z;
    buckets_to_XYZ(cal_data, &X, &Y, &Z);

    double XYZ = (X + Y + Z);
    *x = X / XYZ;
    *y = Y / XYZ;
    *z = Z / XYZ;
}

void json_config_out()
{
}

void startup_blink(int i2c_node)
{
    as7265x_set_indicator_led(i2c_node, 0, 0);
    usleep(200000);
    as7265x_set_indicator_led(i2c_node, 0, 1);
    usleep(200000);
    as7265x_set_indicator_led(i2c_node, 0, 0);
    usleep(200000);
    as7265x_set_indicator_led(i2c_node, 0, 1);
    usleep(200000);
    as7265x_set_indicator_led(i2c_node, 0, 0);
}

// Function designed for chat between client and server.
#define MAX 80
void func(int sockfd)
{
    char buff[MAX];
    int n;
    // infinite loop for chat
    for (;;) {
        bzero(buff, MAX);

        // read the message from client and copy it in buffer
        read(sockfd, buff, sizeof(buff));
        // print buffer which contains the client contents
        printf("From client: %s\t To client : ", buff);
        bzero(buff, MAX);
        n = 0;
        // copy server message in the buffer
        while ((buff[n++] = getchar()) != '\n')
            ;

        // and send that buffer to client
        write(sockfd, buff, sizeof(buff));

        // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("exit", buff, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }
}

void parse_options(int argc, char **argv)
{
    int c;

    static  struct option long_options[] = {
        { "i2c_bus",            required_argument,  NULL,           'i'},
        { "verbose",            no_argument,        NULL,           'v'},
        { "server",             no_argument,        NULL,           's'},
        { "port",               required_argument,  NULL,           'p'},
        { 0,0,0,0 }
    };


    int option_index = 0;
    while((c = getopt_long(argc, argv, "i:vsp:", long_options, &option_index)) != -1){
        switch(c){
            case 'i':
                i2c_bus = atoi(optarg);
                break;
            case 'v':
                verbose_flag = 1;
                break;
            case 's':
                server_flag = 1;
                break;
            case 'p':
                port_nr = atoi(optarg);
                break;
        }
    }

    if (verbose_flag){
        printf("i2c_bus:            %d\n", i2c_bus);
        printf("verbose:            %d\n", verbose_flag);
        printf("server:             %d\n", server_flag);
        printf("port:               %d\n", port_nr);
    }

    if (i2c_bus == -1){
        printf("i2c bus not specified.\n");
        exit(0);
    }
}

void output_gs1160_csv(int fd,
                       char *dut,
                       struct as7265x_dev_identity *di,
                       struct as7265x_measurement_settings *ms,
                       struct as7265x_measurement *m)
{
    char buf[65536];
    memset(buf, sizeof(buf), 0);

    struct tm *tm_info;
    tm_info = localtime(&m->timestamp);
    char time_str[80];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H:%M:%S", tm_info);

    double X,Y,Z,x,y,z;
    buckets_to_XYZ(m->cal_data, &X, &Y, &Z);
    buckets_to_xyz(m->cal_data, &x, &y, &z);

    sprintf(buf, "Model Name, AS72625x\n"
                 "Serial Number, \n"
                 "Time, %s\n"
                 "DUT, %s\n"
                 "Temperature, %d"
                 "X, %f\n"
                 "Y, %f\n"
                 "Z, %f\n"
                 "x, %f\n"
                 "y, %f\n"
                    ,time_str, dut, m->temp[0], X,Y,Z,x,y);

    for(int i=0;i<18;i++){
        sprintf(buf+strlen(buf), "%dnm,%f\n", freqs[freq_order[i]], m->cal_data[freq_order[i]]);
    }

    write(fd, buf, strlen(buf));
}

int main(int argv, char **argc)
{
    int socket_nr, fd;

    parse_options(argv, argc);

    if (server_flag){
        if (setup_tcp_connection(4000, &socket_nr, &fd) != 0){
            printf("Couldn't open socket.\n");
        }
    }

    printf("I2C bus: %d\n", i2c_bus);

    int i2c_node = as7265x_i2c_drv_open(i2c_bus);
    printf("i2c_node: %d\n", i2c_node);
    as7265x_i2c_dev_addr_set(i2c_node, AS72651_ADDRESS);

    as7265x_revision(i2c_node, NULL, NULL, NULL, NULL);

    startup_blink(i2c_node);

    as7265x_init(i2c_node, GAIN_16X, MODE2, 36);

    for(int i=0;i<18;++i){
        int freq = freqs[freq_order[i]];
        printf("%8d,", freq);
    }
    printf("\n\n");

    while(1){
        float cal_data[18];
        as7265x_read_data_cal(i2c_node, cal_data);

        double x,y,z;
        buckets_to_xyz(cal_data, &x, &y, &z);

        printf("x,y,z: %8f, %8f, %8f - ", x,y,z);

#if 1
        for(int i=0;i<18;++i){
            float val = cal_data[freq_order[i]];
            printf("%4f,", val);
        }
        printf("\n");
#endif
    }

}
