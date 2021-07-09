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

void addEthernetHeader(char* data, char* destination_MAC, unsigned short protocol_number);

void addNetworkHeader(char* data, unsigned int source_IP, unsigned int destination_IP, unsigned char protocol, unsigned short length, unsigned short identification);

void addUDPHeader(char* data, unsigned short source_port, unsigned short destination_port, unsigned short length);

void DHCPDiscover();

void DHCPRequest();

void ARPReply(char* sender_MAC, unsigned int sender_IP);

void handleReceive(char* data, unsigned short length);

#endif