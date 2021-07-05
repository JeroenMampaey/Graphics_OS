#include "timer.h"
#include "isr.h"
#include "ports.h"


//private functions
void frequency_helper(unsigned int freq);
void sleep_helper(unsigned short unit, int differentiator);


static void timer_callback(registers_t regs){
    timer_program[current_program]();
}

//change the PIT mode, set a clock with given frequency
void frequency_mode(unsigned int freq){
    __asm__ __volatile__("cli");
    frequency_helper(freq);
    __asm__ __volatile__("sti");
}

void frequency_helper(unsigned int freq){
    unsigned int divisor = 1193180 / freq;

    //setting the timer frequentie
    unsigned char low  = (unsigned char)(divisor & 0xFF);
    unsigned char high = (unsigned char)( (divisor >> 8) & 0xFF);
    //command ports
    port_byte_out(0x43, 0x36);
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
}

//change the PIT mode, generate only one interrupt when the reload value has been completely decremented
//usefull for sleep functions
void use_single_int_mode(){
    __asm__ __volatile__("cli");
    timer_counter = 1;
    port_byte_out(0x43, 0x30);
    port_byte_out(0x40, 0x10);
    port_byte_out(0x40, 0x00);
    __asm__ __volatile__("sti");
    while(timer_counter > 0);
}

void init_timer(unsigned int freq){
    timer_counter = 0;
    register_interrupt_handler(IRQ0, timer_callback);
    frequency_helper(freq);
}

//this function requires "single_int_mode"
//this function is not very exact to be honest
//micro is assumed to be bigger than 0 and smaller than 3600
void sleep_micro(unsigned short micro){
    unsigned int top = (unsigned int)(1193182*micro);
    unsigned int bottom = 1000000;
    unsigned short reload_value = (unsigned short)(top/bottom);
    timer_counter = 1;
    __asm__ __volatile__("cli");
    port_byte_out(0x40, (unsigned char)reload_value);
    port_byte_out(0x40, (unsigned char)(reload_value >> 8));
    __asm__ __volatile__("sti");
    while(timer_counter > 0);
}

//this function requires "single_int_mode"
//this function is not very exact either but alot better than sleep_micro atleast
//mili is assumed to be smaller than 1800
void sleep_mili(unsigned short mili){
    sleep_helper(mili, 1000);
}   

//this function requires "single_int_mode"
//not extremely exact
//seconds is assumed to be smaller than 1800
void sleep_seconds(unsigned short seconds){
    sleep_helper(seconds, 1);
}


void sleep_helper(unsigned short unit, int differentiator){
    int barrier = 1193182*((int)unit);
    int subtractor = differentiator*0xFFFF;
    while((barrier-subtractor) > 0){
        timer_counter = 1;
        __asm__ __volatile__("cli");
        port_byte_out(0x40, 0xFF);
        port_byte_out(0x40, 0xFF);
        __asm__ __volatile__("sti");
        while(timer_counter > 0);
        barrier -= subtractor;
    }
    unsigned short remainder = (unsigned short)(barrier/differentiator);
    timer_counter = 1;
    __asm__ __volatile__("cli");
    port_byte_out(0x40, (unsigned char)remainder);
    port_byte_out(0x40, (unsigned char)(remainder >> 8));
    __asm__ __volatile__("sti");
    while(timer_counter > 0);
}