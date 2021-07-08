#ifndef NETWORK_STACK_H
#define NETWORK_STACK_H
#include "networking.h"

unsigned int my_IP;

void init_stack();

void addEthernetHeader(char* data, char* destination_MAC, unsigned short protocol_number);

void addNetworkHeader(char* data, unsigned int source_IP, unsigned int destination_IP, unsigned char protocol, unsigned short length, unsigned short identification);

void addUDPHeader(char* data, unsigned short source_port, unsigned short destination_port, unsigned short length);

unsigned short DHCPDiscover(char* data);

void handleReceive(char* data, unsigned short length);

#endif