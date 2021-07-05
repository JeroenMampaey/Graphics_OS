#include "disk_reader.h"

//LBA is assumed to be only 28-bits long
void read_disk(int LBA, unsigned char num_of_sectors, short* destination){
    /* 
        Temporary code to read the IDE Primary Master using ATA PIO Mode with a 28-bit LBA address
        Used alot of help from https://wiki.osdev.org/ATA_PIO_Mode
    */
    unsigned char data_byte;
	unsigned short data_word;
	unsigned short port;
    port = 0x1F6;
	data_byte = 0xE0 + (LBA >> 24);
	__asm__("out %%al, %%dx" : : "a" (data_byte), "d" (port));
    port = 0x1F2;
	data_byte = num_of_sectors;
	__asm__("out %%al, %%dx" : : "a" (data_byte), "d" (port));
    port = 0x1F3;
	data_byte = (unsigned char)LBA;
	__asm__("out %%al, %%dx" : : "a" (data_byte), "d" (port));
    port = 0x1F4;
	data_byte = (unsigned char)(LBA >> 8);
	__asm__("out %%al, %%dx" : : "a" (data_byte), "d" (port));
    port = 0x1F5;
	data_byte = (unsigned char)(LBA >> 16);
	__asm__("out %%al, %%dx" : : "a" (data_byte), "d" (port));
    port = 0x1F7;
	data_byte = 0x20;
	__asm__("out %%al, %%dx" : : "a" (data_byte), "d" (port));
    for(int i=0; i<num_of_sectors; i++){
		do{
			__asm__("in %%dx, %%al" : "=a" (data_byte) : "d" (port));
		} while((data_byte & 0b00001000)==0 || (data_byte & 0b10000000)!=0);
		port = 0x1F0;
		for(int i=0; i<256; i++, destination++){
			__asm__("in %%dx, %%ax" : "=a" (data_word) : "d" (port));
			*destination = data_word;
		}
		port = 0x1F7;
		__asm__("in %%dx, %%al" : "=a" (data_byte) : "d" (port));
		__asm__("in %%dx, %%al" : "=a" (data_byte) : "d" (port));
		__asm__("in %%dx, %%al" : "=a" (data_byte) : "d" (port));
		__asm__("in %%dx, %%al" : "=a" (data_byte) : "d" (port));
	}
}