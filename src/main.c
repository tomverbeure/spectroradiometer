
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>
#include <termios.h>

#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <getopt.h>

#include "as7265x.h"
#include "tcs34725.h"
#include "specrend.h"
#include "tcp_server.h"

// Command line options
int i2c_bus = -1;
int verbose_flag = 0;
int server_flag = 0;
int port_nr = 5000;
char dut_name[256];

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

void startup_blink(int as_i2c_node)
{
    as7265x_set_indicator_led(as_i2c_node, 0, 0);
    usleep(200000);
    as7265x_set_indicator_led(as_i2c_node, 0, 1);
    usleep(200000);
    as7265x_set_indicator_led(as_i2c_node, 0, 0);
    usleep(200000);
    as7265x_set_indicator_led(as_i2c_node, 0, 1);
    usleep(200000);
    as7265x_set_indicator_led(as_i2c_node, 0, 0);
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
        { "dut",                required_argument,  NULL,           'd'},
        { "help",               no_argument,        NULL,           'h'},
        { 0,0,0,0 }
    };

    strcpy(dut_name, "<Unknown>");
    memset(dut_name, sizeof(dut_name), 0);

    int option_index = 0;
    while((c = getopt_long(argc, argv, "i:vsp:hd:", long_options, &option_index)) != -1){
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
            case 'd':
                strncpy(dut_name, optarg, sizeof(dut_name)-1);
                break;
            case 'h':
                printf("as7265x ... TODO\n");
                exit(0);
                break;
        }
    }

    if (verbose_flag){
        printf("i2c_bus:            %d\n", i2c_bus);
        printf("verbose:            %d\n", verbose_flag);
        printf("server:             %d\n", server_flag);
        printf("port:               %d\n", port_nr);
        printf("DUT name:           %s\n", dut_name);
    }

    if (i2c_bus == -1){
        printf("i2c bus not specified.\n");
        exit(0);
    }
}

void as_output_csv(int fd,
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

    sprintf(buf, "Model Name, AS7265x\n"
                 "Time, %s\n"
                 "DUT, %s\n"
                 "Temperature, %d\n"
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

void tcs_output_csv(int fd,
                       char *dut,
                       struct tcs34725_dev_identity *di,
                       struct tcs34725_measurement_settings *ms,
                       struct tcs34725_measurement *m)
{
    char buf[65536];
    memset(buf, sizeof(buf), 0);

    struct tm *tm_info;
    tm_info = localtime(&m->timestamp);
    char time_str[80];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H:%M:%S", tm_info);

    sprintf(buf, "Model Name, TCS34725\n"
                 "Time, %s\n"
                 "DUT, %s\n"
                 "Gain, %d\n"
                 "ATime, %f\n"
                 "C, %d\n"
                 "R, %d\n"
                 "G, %d\n"
                 "B, %d\n"
                    ,time_str, dut, ms->gain, ms->atime_ms, m->c, m->r, m->g, m->b);

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

#if 0
    initscr();
    cbreak();
    timeout(-1);
#endif


    struct termios orig_settings, new_settings;
    tcgetattr(0, &orig_settings);

    new_settings = orig_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 0;
    new_settings.c_cc[VTIME] = 0;

    tcsetattr(0, TCSANOW, &new_settings);

    printf("AS7265x I2C bus: %d\n\r", i2c_bus);
    int as_i2c_node = as7265x_i2c_drv_open(i2c_bus);
    printf("AS7265x i2c_node: %d\n\r", as_i2c_node);
    as7265x_i2c_dev_addr_set(as_i2c_node, AS72651_ADDRESS);

    startup_blink(as_i2c_node);
    as7265x_init(as_i2c_node, GAIN_16X, MODE2, 36);

    struct as7265x_dev_identity di;
    struct as7265x_measurement_settings ms;
    struct as7265x_measurement m;

    as7265x_fill_dev_identify(as_i2c_node, &di);
    as7265x_fill_measurement_settings(as_i2c_node, &ms);

    printf("TCS34725 I2C bus: %d\n\r", i2c_bus);
    int tcs_i2c_node = tcs34725_i2c_drv_open(i2c_bus);
    printf("TCS34725 i2c_node: %d\n\r", tcs_i2c_node);
    tcs34725_i2c_dev_addr_set(tcs_i2c_node, TCS34725_ADDRESS);

    int tcs_id = tcs34725_id(tcs_i2c_node);
    printf("TCS id: %02x\n", tcs_id);

    tcs34725_init(tcs_i2c_node, 2, 0x40);

    struct tcs34725_dev_identity tcs_di;
    struct tcs34725_measurement_settings tcs_ms;
    struct tcs34725_measurement tcs_m;

    tcs34725_fill_dev_identify(tcs_i2c_node, &tcs_di);
    tcs34725_fill_measurement_settings(tcs_i2c_node, &tcs_ms);

    char ch = 0;
    while(ch != 'q'){
        ch = getchar();
        if (ch == 3){
            break;
        }

        float cal_data[18];
        as7265x_read_data_cal(as_i2c_node, cal_data);

        double x,y,z;
        buckets_to_xyz(cal_data, &x, &y, &z);

        printf("x,y,z: %4.3f,%4.3f,%4.3f - ", x,y,z);

        uint16_t c,r,g,b;
        tcs34725_get_data(tcs_i2c_node, &c, &r, &g, &b);

        printf("c,x,y,z: %d,%d,%d,%d\r\n", c,r,g,b);

        if (verbose_flag){
            for(int i=0;i<18;++i){
                float val = cal_data[freq_order[i]];
                printf("%3.3f,", val);
            }
            printf("\n\r");
        }

        if (ch == 'r' || ch == 'g' || ch == 'b' || ch == 'w'){
            as7265x_fill_measurement(as_i2c_node, &m);
            tcs34725_fill_measurement(tcs_i2c_node, &tcs_m);

            struct tm *tm_info;
            tm_info = localtime(&m.timestamp);
            char time_str[80];
            strftime(time_str, sizeof(time_str), "%Y%m%d_%H%M%S", tm_info);

            char filename[256];
            sprintf(filename, "as7265x_%s_%c_%s.csv", dut_name, ch, time_str);
            printf("Logging to file '%s'.\n", filename);
            int fd = creat(filename, O_RDWR | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            as_output_csv(fd, dut_name, &di, &ms, &m);
            close(fd);

            sprintf(filename, "tcs34725_%s_%c_%s.csv", dut_name, ch, time_str);
            printf("Logging to file '%s'.\n", filename);
            fd = creat(filename, O_RDWR | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            tcs_output_csv(fd, dut_name, &tcs_di, &tcs_ms, &tcs_m);
            close(fd);
        }
    }

    tcsetattr(0, TCSANOW, &orig_settings);
}
