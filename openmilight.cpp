
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <stdio.h>

#include <sys/time.h>
#include <sys/select.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

#include <RF24/RF24.h>

#include "PL1167_nRF24.h"
#include "MiLightRadio.h"

RF24 radio(RPI_V2_GPIO_P1_22, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_1MHZ);

PL1167_nRF24 prf(radio);
MiLightRadio mlr(prf);

static int debug = 0;

static int dupesPrinted = 0;

void receive()
{
  while(1){
    if(mlr.available()) {
      printf("\n");
      uint8_t packet[16];
      size_t packet_length = sizeof(packet);
      mlr.read(packet, packet_length);

      for(size_t i = 0; i < packet_length; i++) {
        printf("%02X ", packet[i]);
        fflush(stdout);
      }
    }

    int dupesReceived = mlr.dupesReceived();
    for (; dupesPrinted < dupesReceived; dupesPrinted++) {
      printf(".");
    }
    fflush(stdout);
  } 
}

void send(uint8_t data[16], uint8_t length, uint8_t resends)
{
  if(debug){
    printf("2.4GHz --> Sending %d chars (%d times): ", length, resends+1);
    for (int i = 0; i < length; i++) {
      printf("%02X ", data[i]);
    }
    printf("\n");
    //printf(" [x%d]\n", resends);
  }

  mlr.write(data, length);

  for(int i = 0; i < resends; i++){
    mlr.resend();
  }

}

void udp_raw(uint8_t resends)
{
  int sockfd;
  struct sockaddr_in servaddr, cliaddr;
  char mesg[42];

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(8899);
  bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

  while(1){
    socklen_t len = sizeof(cliaddr);
    int n = recvfrom(sockfd, mesg, 41, 0, (struct sockaddr *)&cliaddr, &len);

    mesg[n] = '\0';

    if(n == 9){
      if(debug){
        printf("UDP --> Received %d bytes\n", n);
      }
      uint8_t data[9];
      for(int i = 0; i < 9; i++){
        data[i] = (uint8_t)mesg[i];
      }
      send(data, n, resends);
    }
    else {
      fprintf(stderr, "Message has invalid size %d (expecting 9)!\n", n);
    }
  }
}

void usage(const char *arg, const char *options){
  printf("\n");
  printf("Usage: sudo %s [%s]\n", arg, options);
  printf("\n");
  printf("   -h                         Show this help\n");
  printf("   -d                         Show debug output\n");
  printf("   -l                         Listening (receiving) mode\n");
  printf("   -u                         UDP mode (raw)\n");
  printf("   -n NN<dec>                 Resends of the same message\n");
  printf("   -w XXXXXXXXXXXXXXXXXX<hex> Complete message to send, including checksum,\n");
  printf("                              but without length or crc\n");
  printf("\n");
  printf("\n");
  printf(" Inspired by sources from: - https://github.com/henryk/\n");
  printf("                           - http://torsten-traenkner.de/wissen/smarthome/openmilight.php\n");
  printf("                           - https://github.com/bakkerr/openmilight_pi\n");
  printf("\n");
}

int main(int argc, char** argv)
{
  int do_receive = 0;
  int do_udp     = 0;
  int do_command = 0;
  uint8_t resends  =   4;

  uint8_t command[16];
  uint8_t length;

  int c;

  const char *options = "hdlun:w:";

  while((c = getopt(argc, argv, options)) != -1){
    switch(c){
      case 'h':
        usage(argv[0], options);
        exit(0);
        break;
      case 'd':
        debug = 1;
        break;
      case 'l':
        do_receive = 1;
       break;
      case 'u':
        do_udp = 1;
       break;
      case 'n':
        resends = strtoll(optarg, NULL, 10);
        break;
      case 'w':
        do_command = 1;
        length = strlen(optarg)/2;
        for(int i = 0; i < length; i++) {
          char tmp[3]={0,0,0};
          tmp[0]=optarg[i*2+0];
          tmp[1]=optarg[i*2+1];
          command[i] = strtoll(tmp, NULL, 16);
        }
        break;
      case '?':
        if(optopt == 'n' || optopt == 'w'){
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        }
        else if(isprint(optopt)){
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        }
        else{
          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
        }
        return 1;
      default:
        fprintf(stderr, "Error parsing options");
        return -1;
    }
  }

  int ret = mlr.begin();

  if(ret < 0){
    fprintf(stderr, "Failed to open connection to the 2.4GHz module.\n");
    fprintf(stderr, "Make sure to run this program as root (sudo)\n\n");
    usage(argv[0], options);
    exit(-1);
  }

  if(do_receive){
    printf("Receiving mode, press Ctrl-C to end\n");
    receive();
  }
 
  if(do_udp){
    printf("UDP mode (raw), press Ctrl-C to end\n"); 
    udp_raw(resends);
  } 

  if(do_command){
    send(command, length, resends);
  }

  return 0;
}
