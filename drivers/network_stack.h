#ifndef NETWORK_STACK_H
#define NETWORK_STACK_H
#include "networking.h"

#define PACKET_DATA_BUFFER 0x700000

static int current;

unsigned int my_IP;

unsigned int DHCP_IP;

unsigned int router_IP;

static unsigned int proposed_IP;

static int DHCP_step;  //0<->no DHCP communication at the moment, 1<->DHCP discover send, 2<->DHCP request send

void init_stack();

void DHCPDiscover();

void handleReceive(char* data, unsigned short length);

#endif