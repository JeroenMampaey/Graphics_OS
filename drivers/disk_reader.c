#include "disk_reader.h"

void read_disk(unsigned int LBA, unsigned char num_of_sectors, short* destination){
    /* 
        Code to read the IDE Primary Master using ATA PIO Mode with a 28-bit LBA address
        Used alot of help from https://wiki.osdev.org/ATA_PIO_Mode
    */
	port_byte_out(0x1F6, (unsigned char)(0xE0 + (LBA >> 24)));
	port_byte_out(0x1F2, num_of_sectors);
	port_byte_out(0x1F3, (unsigned char)LBA);
    port_byte_out(0x1F4, (unsigned char)(LBA >> 8));
	port_byte_out(0x1F5, (unsigned char)(LBA >> 16));
	port_byte_out(0x1F7, 0x20);

	unsigned char data_byte;
	unsigned short data_word;
	
    for(int i=0; i<num_of_sectors; i++){
		do{
			data_byte = port_byte_in(0x1F7);
		} while((data_byte & 0b00001000)==0 || (data_byte & 0b10000000)!=0);
		for(int i=0; i<256; i++, destination++){
			//it is possible to use the assembly "rep" instruction here but since PIO mode is not efficiënt anyway and I (for this small OS system) 
			//don't need it do be, I just use a for-loop
			data_word = port_word_in(0x1F0);
			*destination = data_word;
		}
		port_word_in(0x1F7);
		port_word_in(0x1F7);
		port_word_in(0x1F7);
		port_word_in(0x1F7);
	}
}

void write_disk(unsigned int LBA, unsigned char num_of_sectors, short* data){
	/* 
        Code to write the IDE Primary Master using ATA PIO Mode with a 28-bit LBA address
        Used alot of help from https://wiki.osdev.org/ATA_PIO_Mode
    */
	port_byte_out(0x1F6, (unsigned char)(0xE0 + (LBA >> 24)));
	port_byte_out(0x1F2, num_of_sectors);
	port_byte_out(0x1F3, (unsigned char)LBA);
    port_byte_out(0x1F4, (unsigned char)(LBA >> 8));
	port_byte_out(0x1F5, (unsigned char)(LBA >> 16));
	port_byte_out(0x1F7, 0x30);

	unsigned char data_byte;
	unsigned short data_word;

	for(int i=0; i<num_of_sectors; i++){
		do{
			data_byte = port_byte_in(0x1F7);
		} while((data_byte & 0b00001000)==0 || (data_byte & 0b10000000)!=0);
		for(int i=0; i<256; i++, data++){
			port_word_out(0x1F0, *data);
		}
		port_word_in(0x1F7);
		port_word_in(0x1F7);
		port_word_in(0x1F7);
		port_word_in(0x1F7);
	}

	port_byte_out(0x1F7, 0xE7);
}