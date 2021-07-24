#include "../drivers/screen.h"
#include "../cpu/isr.h"
#include "../programs/kernel_program.h"
#include "kernel.h"
#include "../programs/graphics_program.h"
#include "../drivers/networking.h"
#include "../drivers/network_stack.h"
#include "../programs/editor_program.h"
#include "../drivers/disk_reader.h"
#include "../cpu/timer.h"


//private functions
void setup_kernel();
void showMemLayout();
void setup_AP();
int checkFirst();

void main(){
	setup_kernel();
	while(1){
		loop_program[current_program]();
	}
}

void AP_main(){
	setup_AP();
	while(1){
		__asm__ __volatile__(
            ".intel_syntax noprefix;"
            "hlt;"
            ".att_syntax;"
        );
		if(current_program==1) AP_drawing();
	}
}

void setup_AP(){
	//enable APIC interrupts
	*((volatile unsigned int*)(LOCAL_APIC_address + 0xF0)) = *((volatile unsigned int*)(LOCAL_APIC_address + 0xF0)) | 0x100;

	isr_install_AP();
	//enable hardware interrupts
	__asm__ __volatile__("sti");
}

void setup_kernel(){
	init_screen();
	printk("Succesfully landed in the kernel in 32-bit protected mode.\n");
	//check whether the A20 line is enabled
	//https://wiki.osdev.org/A20_Line
	int a20_on;
	__asm__ __volatile__(
		".intel_syntax noprefix;"
		"pushad;"
		"mov edi, 0x112345;"
		"mov esi, 0x012345;"
		"mov [esi], esi;"
		"mov [edi], edi;"
		"mov eax, 1;"
		"cmpsd;"
		"jne A20_ON;"
		"mov eax, 0;"
		"A20_ON:"    
		".att_syntax;"
	: "=a"(a20_on) : );
	__asm__ __volatile__(
		".intel_syntax noprefix;"
		"popad;"    
		".att_syntax;"
	);
	if(a20_on==1) printk("The A20 line is on.\n");
	else{
		printk("The A20 line is off, OS won't start with a disabled A20 line.\n");
		while(1);
	}
	//initialize all programs
	loop_program[0] = kernel_loop_callback;
	loop_program[1] = graphics_loop_callback;
	loop_program[2] = editor_loop_callback;
	keyboard_program[0] = kernel_keyboard_callback;
	keyboard_program[1] = graphics_keyboard_callback;
	keyboard_program[2] = editor_keyboard_callback;
	timer_program[0] = kernel_timer_callback;
	timer_program[1] = graphics_timer_callback;
	timer_program[2] = editor_timer_callback;
	current_program = 0;

	//setup interrupts
	ipi_install();
	isr_install();
	irq_install();

	//enable hardware interrupts
	__asm__ __volatile__("sti");
	printk("Succesfully setup interrupts.\n");
	
	//setup a multicore system using APIC
	setup_multicore();

	//setup the e1000 network card
	init_e1000();

	//setup networking
	init_stack();

	//empty the buffer of the editor program
	//buffer is 0x28CD0 bytes big, allowing for 2089 lines of ASCII char's
    clear_dwords_asm(0x1000000, 0xA334);

	//check whether this is the first time that the OS ever runs on the machine
	//I determine this by checking whether a specific sector (LBA 199) is full of 0xFF's
	//if so, do some things (make sure that the 3 editor files are empty on the disk)
	int first = checkFirst();

	//start the kernel program
	boot_to_kernel();

	//clear the screen
	clear_screen();
	if(first==1) printk("You are new here, welcome to GOS!\n\n\n\n\n");
	else printk("Welcome to GOS!\n\n\n\n\n");
	showMemLayout();
	printk("> ");
}

void showMemLayout(){
	unsigned char* addr = (unsigned char*)MEMORY_MAP;
	printk("\nBIOS memory layout:\n");
	int mul = 1;
	int total = 0;
	char str[19];
	int lower_hex;
	int upper_hex;
	for(int i=0; i<4; i++, addr++){
		total += (*addr)*mul;
		mul *= 16;
	}
	mul = 1;
	for(int e=0; e<total; e++){
		for(int i=0; i<3; i++){
			str[16] = 'x';
			str[17] = '0';
			str[18] = '\0';
			for(int j=0; j<8; j++, addr++){
				upper_hex = (0b11110000 & (int)(*addr)) >> 4;
				lower_hex = (0b00001111 & (int)(*addr));
				str[2*j+1] = (upper_hex < 10) ? upper_hex+'0' : upper_hex-10+'A';
				str[2*j] = (lower_hex < 10) ? lower_hex+'0' : lower_hex-10+'A';
			}
			reverse(str);
			printk(str);
			if(i!=2) printk("|");
		}
		printk("\n");
	}
	printk("\n\n\n");
}

int checkFirst(){
	printk("Checking whether this machine has used this OS before.\n");
	short* destination = (short*)0x2000000;
	read_disk(199, 1, destination);
	int first = 0;
	for(int i=0; i<256; i++){
		if((unsigned short)(*(destination+i)) != 0xFFFF){
			first = 1;
			*(destination+i) = 0xFFFF;
		}
	}
	if(first==1){
		printk("This is the first time the machine uses this OS.\n");
		printk("If this message does not go away in less than 30 seconds, then try restarting the machine.\n");
		write_disk(526, 255, (short*)0x1000000);
		write_disk(526+255, 71, (short*)0x101FE00);
		write_disk(852, 255, (short*)0x1000000);
		write_disk(852+255, 71, (short*)0x101FE00);
		char demo[] = "T(50,50,0,150,50,0,150,150,0,1);\nT(50,50,0,50,150,0,150,150,0,1);\nT(50,50,0,50,50,-100,50,150,-100,2);\nT(50,50,0,50,150,0,50,150,-100,2);\nT(150,50,0,150,150,0,150,150,-100,3);\nT(150,50,0,150,50,-100,150,150,-100,3);\nT(50,50,-100,150,50,-100,150,150,-100,4);\nT(50,50,-100,50,150,-100,150,150,-100,4);\nT(50,50,0,150,50,0,150,50,-100,5);\nT(50,50,0,50,50,-100,150,50,-100,5);\nT(50,150,0,150,150,0,150,150,-100,6);\nT(50,150,0,50,150,-100,150,150,-100,6);";
		char* demo_buffer = (char*)0x1000000;
		for(int i=0; i<strlen(demo); i++, demo_buffer++){
			*demo_buffer = demo[i];
		}
		write_disk(200, 255, (short*)0x1000000);
		write_disk(200+255, 71, (short*)0x101FE00);
		write_disk(199, 1, destination);
	}
	else printk("The machine has used this OS before.\n");
	return first;
}