#include "kernel_program.h"
#include "../kernel/kernel.h"
#include "graphics_program.h"
#include "../drivers/screen.h"
#include "../drivers/networking.h"

//declaring private functions
void user_input(char* input);

void boot_to_kernel(){
    key_buffer[0] = '\0';
    RSDP_address = 0;
    command_ready = 0;
}

void kernel_loop_callback(){
    if(command_ready==1){
        user_input(key_buffer);
        key_buffer[0] = '\0';
        command_ready = 0;
    }
    else{
        __asm__ __volatile__(
            ".intel_syntax noprefix;"
            "hlt;"
            ".att_syntax;"
        );
    }
}

void kernel_keyboard_callback(unsigned char scancode){
    if(scancode > SC_MAX) return;
    else if(command_ready == 1) return;
    else if(scancode == BACKSPACE){
        backspace(key_buffer);
        printk_backspace();
    }
    else if(scancode == ENTER){
        printk("\n");
        command_ready = 1;
    }
    else{
        char letter = sc_ascii[(int)scancode];
        char str[2] = {letter, '\0'};
        append(key_buffer, letter);
        printk(str);
    }
}

void kernel_timer_callback(){
    if(timer_counter > 0) timer_counter--;
}

void user_input(char* input){
	if(strcmp(input, "CLEAR")==0){
		clear_screen();
	}
	else if(strcmp(input, "RUN GRAPHICS")==0){
        //do some initializations when switching to the graphics program
        kernel_to_graphics();
        current_program = 1;
        for(int i=0; i<num_cores; i++) if(i!=BSP_ID) sendIPI(i, 2);
        show_screen();
        return;
	}
    else if(strcmp(input, "VENDOR ID")==0){
        //https://wiki.osdev.org/CPUID
        //answer should be "GenuineIntel" cause this is the processor that i assume is being used
        registers_t regs;
        regs.eax = 0;
        execCPUID(&regs);
        char to_print[13];
        for(int i=0; i<4; i++){
            to_print[i] = (char)(regs.ebx >> 8*i);
        }
        for(int i=0; i<4; i++){
            to_print[i+4] = (char)(regs.edx >> 8*i);
        }
        for(int i=0; i<4; i++){
            to_print[i+8] = (char)(regs.ecx >> 8*i);
        }
        to_print[12] = '\0';
        printk(to_print);
    }
    else if(strcmp(input, "TEST MULTICORE")==0){
        use_single_int_mode();
        printk("\n");
        char to_print[10];
        for(int i=0; i<num_cores; i++){
            printk("Core with APIC ID ");
            int_to_ascii(i, to_print);
            printk(to_print);
            if(i != BSP_ID){
                reply = 0;
                sendIPI(i, 0);
                sleep_mili(10);
                if(reply==0){
                    printk(" is either not working or to slow.\n");
                }
                else{
                    printk(" is working normally.\n");
                }
            }
            else{
                printk(" is the BSP.\n");
            }
        }
        frequency_mode(60);
    }
    else if(substr_cmp(input, "CONNECT ")==0 && strlen(input)==20){
        int correct = 1;
        for(int i=8; i<20; i++){
            if((*(input+i) < '0') || (*(input+i) > '9' && *(input+i) < 'A') || (*(input+i) > 'F')) correct = 0;
        }
        if(correct == 1){
            char* data = (char*)0x700000;
            *data = ascii_to_hex(*(input+8))*16+ascii_to_hex(*(input+9));
            *(data+1) = ascii_to_hex(*(input+10))*16+ascii_to_hex(*(input+11));
            *(data+2) = ascii_to_hex(*(input+12))*16+ascii_to_hex(*(input+13));
            *(data+3) = ascii_to_hex(*(input+14))*16+ascii_to_hex(*(input+15));
            *(data+4) = ascii_to_hex(*(input+16))*16+ascii_to_hex(*(input+17));
            *(data+5) = ascii_to_hex(*(input+18))*16+ascii_to_hex(*(input+19));
            *(data+6) = mac[0];
            *(data+7) = mac[1];
            *(data+8) = mac[2];
            *(data+9) = mac[3];
            *(data+10) = mac[4];
            *(data+11) = mac[5];
            *(data+12) = 'C';
            *(data+13) = 'O';
            *(data+14) = 'N';
            *(data+15) = 'N';
            *(data+16) = 'E';
            *(data+17) = 'C';
            *(data+18) = 'T';
            sendPacket(data, 19);
        }
        else{
            printk("MAC addres is not valid.");
        }
    }
    else{
		printk(input);
	}
	printk("\n");
	printk("> ");
}

