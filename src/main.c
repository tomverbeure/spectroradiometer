
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>

#include <netdb.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 

#include "as7265x.h"
#include "specrend.h"
#include "tcp_server.h"

void buckets_to_xyz(float cal_data[18], double *x, double *y, double *z)
{
    float X = 0.0;
    float Y = 0.0;
    float Z = 0.0;

    for(int i=0;i<18;++i){
        int   freq = freqs[freq_order[i]];
        float val  = cal_data[freq_order[i]];

        int entry = (freq - 380)/5;
        if (entry > 80)
            continue;

        X += cie_colour_match[entry][0] * val;
        Y += cie_colour_match[entry][1] * val;
        Z += cie_colour_match[entry][2] * val;
    }

    double XYZ = (X + Y + Z);
    *x = X / XYZ;
    *y = Y / XYZ;
    *z = Z / XYZ;
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

#if 0
int parse_options(int argv, char **argc)
{
    int c;

    int verbose_flag;

    static  struct option long_options[] = {
        { "verbose",            no_argument,        &verbose_flag,  1},
        { "server",             no_argument,        0,              'p'},
        { "port",               required_argument,  0,              'p'},
        { 0,0,0,0 }
    };
        

    int option_index = 0;
    while((c = getopt_long(argc, argv, "p:", long_options, &option_index)) != -1){
        switch(c){
            case 0:
        }
    }
}
#endif

int main(int argv, char **argc)
{
    int socket_nr, fd; 
    
    if (setup_tcp_connection(4000, &socket_nr, &fd) != 0){
        printf("Couldn't open socket.\n");
    }

    close_tcp_connection(socket_nr);

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

    for(int i=0;i<18;++i){
        int freq = freqs[freq_order[i]];
        printf("%8d,", freq);
    }
    printf("\n\n");

    while(1){
        float cal_data[18];
        as7265x_read_cal_data(node, cal_data);

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
