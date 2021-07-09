#include "network_stack.h"

//private functions
int DHCP_option(char* data, unsigned char option);

//start up networking
void init_stack(){
    my_IP = 0;
    DHCP_IP = 0;
    router_IP = 0;
    proposed_IP = 0;
    current = 0;
    DHCP_step = 0;
}

/*
    https://en.wikipedia.org/wiki/Ethernet_frame
    https://en.wikipedia.org/wiki/IPv4#Header
    https://en.wikipedia.org/wiki/User_Datagram_Protocol
    https://en.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol
*/
void handleReceive(char* data, unsigned short length){
    //small test program which i keep here temporarily
    if(length>19 && *(data+12) == 'C' && *(data+13) == 'O' && *(data+14)=='N' && *(data+15)=='N' && *(data+16)=='E' && *(data+17)=='C' && *(data+18)=='T'){
        printk("\nReceived an ethernet connect message!\n");
        return;
    }
    char source_mac[6];
    char destination_mac[6];
    unsigned int source_IP;
    unsigned int destination_IP;
    for(int i=0; i<6; i++){
        destination_mac[i] = *(data+i);
        source_mac[i] = *(data+6+i);
    }
    if((unsigned char)(*(data+12)) != 0x08 || (unsigned char)(*(data+13)) != 0x00) return;  //only protocol that i consider at the moment is IPv4
    source_IP = ((unsigned int)(unsigned char)(*(data+26)) << 24) + ((unsigned int)(unsigned char)(*(data+27)) << 16) + ((unsigned int)(unsigned char)(*(data+28)) << 8) + (unsigned int)(unsigned char)(*(data+29));
    destination_IP = ((unsigned int)(unsigned char)(*(data+30)) << 24) + ((unsigned int)(unsigned char)(*(data+31)) << 16) + ((unsigned int)(unsigned char)(*(data+32)) << 8) + (unsigned int)(unsigned char)(*(data+33));
    int destination_type = 0b11;  //0<->not meant for this computer, 0b01<->broadcast, 0b10<->explicitly for this computer
    for(int i=0; i<6 && destination_type!=0; i++){
        if(destination_mac[i] != mac[i]) destination_type &= 0b01;
        if((unsigned char)destination_mac[i] != 0xFF) destination_type &= 0b10;
    }
    if(destination_IP!=my_IP) destination_type &= 0b01;
    if(destination_IP!=0xFFFFFFFF) destination_type &= 0b10;
    if(destination_type==0) return;
    unsigned short source_port = ((unsigned short)(unsigned char)(*(data+34)) << 8) +  (unsigned short)(unsigned char)(*(data+35));
    unsigned short destination_port = ((unsigned short)(unsigned char)(*(data+36)) << 8) +  (unsigned short)(unsigned char)(*(data+37));
    if(destination_port==68 && source_port==67){
        int index = DHCP_option(data, 0x35);
        if((unsigned char)(*(data+index+1)) == 0x02 && DHCP_step==1){
            printk("\nDHCP offer has been received.\n");
            index = DHCP_option(data, 0x03);
            if(index != -1){
                router_IP = ((unsigned int)(unsigned char)(*(data+index+1)) << 24) + ((unsigned int)(unsigned char)(*(data+index+2)) << 16) + ((unsigned int)(unsigned char)(*(data+index+3)) << 8) + (unsigned int)(unsigned char)(*(data+index+4));
                printk("Router IP has been found.\n");
            }
            index = DHCP_option(data, 0x36);
            if(index != -1){
                DHCP_IP = ((unsigned int)(unsigned char)(*(data+index+1)) << 24) + ((unsigned int)(unsigned char)(*(data+index+2)) << 16) + ((unsigned int)(unsigned char)(*(data+index+3)) << 8) + (unsigned int)(unsigned char)(*(data+index+4));
                printk("DHCP server IP has been found.\n");
            }
            proposed_IP = ((unsigned int)(unsigned char)(*(data+58)) << 24) + ((unsigned int)(unsigned char)(*(data+59)) << 16) + ((unsigned int)(unsigned char)(*(data+60)) << 8) + (unsigned int)(unsigned char)(*(data+61)); 
            DHCPRequest();
        }
        else if((unsigned char)(*(data+index+1)) == 0x05 && DHCP_step==2){
            printk("DHCP acknowledgement has been received.\n");
            my_IP = ((unsigned int)(unsigned char)(*(data+58)) << 24) + ((unsigned int)(unsigned char)(*(data+59)) << 16) + ((unsigned int)(unsigned char)(*(data+60)) << 8) + (unsigned int)(unsigned char)(*(data+61));
            if(my_IP == proposed_IP){
                printk("This computer now has an IP-address.\n");
                DHCP_step = 0;
            }
            else my_IP = 0;
        }
    }
}

//https://en.wikipedia.org/wiki/Ethernet_frame
void addEthernetHeader(char* data, char* destination_MAC, unsigned short protocol_number){
    for(int i=0; i<6; i++){
        *(data+i) = *(destination_MAC+i);
        *(data+6+i) = mac[i];
    }
    *(data+12) = (unsigned char)(protocol_number >> 8);
    *(data+13) = (unsigned char)(protocol_number & 0xFF);
}

//https://en.wikipedia.org/wiki/IPv4#Header
//https://en.wikipedia.org/wiki/IPv4_header_checksum
void addNetworkHeader(char* data, unsigned int source_IP, unsigned int destination_IP, unsigned char protocol, unsigned short length, unsigned short identification){
    data += 14;
    *data = 0x45;
    *(data+1) = 0;
    *(data+2) = (unsigned char)((length+20) >> 8);
    *(data+3) = (unsigned char)((length+20) & 0xFF);
    *(data+4) = (unsigned char)(identification >> 8);
    *(data+5) = (unsigned char)(identification & 0xFF);
    *(data+6) = 0;
    *(data+7) = 0;
    *(data+8) = 100;
    *(data+9) = protocol;
    *(data+12) = (unsigned char)(source_IP >> 24);
    *(data+13) = (unsigned char)((source_IP >> 16) & 0xFF);
    *(data+14) = (unsigned char)((source_IP >> 8) & 0xFF);
    *(data+15) = (unsigned char)(source_IP & 0xFF);
    *(data+16) = (unsigned char)(destination_IP >> 24);
    *(data+17) = (unsigned char)((destination_IP >> 16) & 0xFF);
    *(data+18) = (unsigned char)((destination_IP >> 8) & 0xFF);
    *(data+19) = (unsigned char)(destination_IP & 0xFF);

    unsigned int total_sum = 0;
    for(int i=0; i<10; i++){
        if(i!=5){
            unsigned short data1 = (unsigned short)(unsigned char)(*(data+2*i));
            unsigned short data2 = (unsigned short)(unsigned char)(*(data+2*i+1));
            total_sum += (data1 << 8) + data2;
        }
    }

    //since i assume a 20 byte header, this is overkill
    while((total_sum & 0xFFFF) != total_sum) total_sum = (total_sum & 0xFFFF) + (total_sum >> 16); 
    unsigned short checksum = total_sum;
    checksum = ~checksum;
    *(data+10) = (unsigned char)(checksum >> 8);
    *(data+11) = (unsigned char)(checksum & 0xFF);
}

//https://en.wikipedia.org/wiki/User_Datagram_Protocol
void addUDPHeader(char* data, unsigned short source_port, unsigned short destination_port, unsigned short length){
    data += 34;
    *data = (unsigned char)(source_port >> 8);
    *(data+1) = (unsigned char)(source_port & 0xFF);
    *(data+2) = (unsigned char)(destination_port >> 8);
    *(data+3) = (unsigned char)(destination_port & 0xFF);
    *(data+4) = (unsigned char)((length+8) >> 8);
    *(data+5) = (unsigned char)((length+8) & 0xFF);
    *(data+6) = 0;  //field is optional apparently in IPv4
    *(data+7) = 0;
}

//https://en.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol
void DHCPDiscover(){
    char* data = (char*)(PACKET_DATA_BUFFER+current);
    char dest_mac[6];
    for(int i=0; i<6; i++){
        dest_mac[i] = 0xFF;
    }
    unsigned short length = 256;
    addUDPHeader(data, 68, 67, length);
    length += 8;
    addNetworkHeader(data, 0, 0xFFFFFFFF, 0x11, length, 0);
    length += 20;
    addEthernetHeader(data, dest_mac, 0x0800);
    length += 14;
    data += 42;
    *data = 0x01;
    *(data+1) = 0x01;
    *(data+2) = 0x06;
    *(data+3) = 0x00;
    *(data+4) = 0x39;
    *(data+5) = 0x03;
    *(data+6) = 0xF3;
    *(data+7) = 0x26;
    for(int i=0; i<20; i++) *(data+8+i) = 0x00;
    *(data+28) = 0x00;
    *(data+29) = 0x05;
    *(data+30) = 0x3C;
    *(data+31) = 0x04;
    *(data+32) = 0x8D;
    *(data+33) = 0x59;
    for(int i=0; i<10; i++) *(data+34+i) = 0x00;
    for(int i=0; i<192; i++) *(data+44+i) = 0x00;
    *(data+236) = 0x63;
    *(data+237) = 0x82;
    *(data+238) = 0x53;
    *(data+239) = 0x63;
    *(data+240) = 0x35;
    *(data+241) = 0x01;
    *(data+242) = 0x01;
    *(data+243) = 0x32;
    *(data+244) = 0x04;
    *(data+245) = 0xC0;
    *(data+246) = 0xA8;
    *(data+247) = 0x01;
    *(data+248) = 0x64;
    *(data+249) = 0x37;
    *(data+250) = 0x04;
    *(data+251) = 0x01;
    *(data+252) = 0x03;
    *(data+253) = 0x0F;
    *(data+254) = 0x06;
    *(data+255) = 0xFF;
    current += length;
    current = (current > PACKET_DATA_BUFFER+0x100000) ? PACKET_DATA_BUFFER : current;
    data -= 42;
    sendPacket(data, length);
    DHCP_step = 1;
}

//https://en.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol
void DHCPRequest(){
    char* data = (char*)(PACKET_DATA_BUFFER+current);
    char dest_mac[6];
    for(int i=0; i<6; i++){
        dest_mac[i] = 0xFF;
    }
    unsigned short length = 256;
    addUDPHeader(data, 68, 67, length);
    length += 8;
    addNetworkHeader(data, 0, 0xFFFFFFFF, 0x11, length, 0);
    length += 20;
    addEthernetHeader(data, dest_mac, 0x0800);
    length += 14;
    data += 42;
    *data = 0x01;
    *(data+1) = 0x01;
    *(data+2) = 0x06;
    *(data+3) = 0x00;
    *(data+4) = 0x39;
    *(data+5) = 0x03;
    *(data+6) = 0xF3;
    *(data+7) = 0x26;
    for(int i=0; i<4; i++) *(data+8+i) = 0x00;
    for(int i=0; i<4; i++) *(data+12+i) = (unsigned char)((proposed_IP >> (8*(3-i))) & 0xFF);
    for(int i=0; i<4; i++) *(data+16+i) = 0x00;
    for(int i=0; i<4; i++) *(data+20+i) = (unsigned char)((DHCP_IP >> (8*(3-i))) & 0xFF);
    for(int i=0; i<4; i++) *(data+24+i) = 0x00;
    *(data+28) = 0x00;
    *(data+29) = 0x05;
    *(data+30) = 0x3C;
    *(data+31) = 0x04;
    *(data+32) = 0x8D;
    *(data+33) = 0x59;
    for(int i=0; i<10; i++) *(data+34+i) = 0x00;
    for(int i=0; i<192; i++) *(data+44+i) = 0x00;
    *(data+236) = 0x63;
    *(data+237) = 0x82;
    *(data+238) = 0x53;
    *(data+239) = 0x63;
    *(data+240) = 0x35;
    *(data+241) = 0x01;
    *(data+242) = 0x03;
    *(data+243) = 0x32;
    *(data+244) = 0x04;
    *(data+245) = (unsigned char)((proposed_IP >> 24) & 0xFF);
    *(data+246) = (unsigned char)((proposed_IP >> 16) & 0xFF);
    *(data+247) = (unsigned char)((proposed_IP >> 8) & 0xFF);
    *(data+248) = (unsigned char)(proposed_IP & 0xFF);
    *(data+249) = 0x36;
    *(data+250) = 0x04;
    *(data+251) = (unsigned char)((DHCP_IP >> 24) & 0xFF);
    *(data+252) = (unsigned char)((DHCP_IP >> 16) & 0xFF);
    *(data+253) = (unsigned char)((DHCP_IP >> 8) & 0xFF);
    *(data+254) = (unsigned char)(DHCP_IP & 0xFF);
    *(data+255) = 0xFF;
    current += length;
    current = (current > PACKET_DATA_BUFFER+0x100000) ? PACKET_DATA_BUFFER : current;
    data -= 42;
    sendPacket(data, length);
    DHCP_step = 2;
}

//https://en.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol
int DHCP_option(char* data, unsigned char option){
    int index = 282;
    unsigned char option_id;
    while((option_id = (unsigned char)(*(data+index))) != 0xFF){
        index++;
        if(option_id == option) return index;
        else{
            int length = (int)(unsigned char)(*(data+index));
            index += length+1;
        }
    }
    return -1; //option not found
}