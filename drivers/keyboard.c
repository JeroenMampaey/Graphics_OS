#include "keyboard.h"
#include "../cpu/ports.h"
#include "../cpu/isr.h"
#include "screen.h"
#include "../libc/string.h"
#include "../kernel/kernel.h"
#include "../cpu/timer.h"

//kinda like azerty but not really
const char sc_ascii[] = { '?', '?', '1', '2', '3', '4', '5', '6',     
    '7', '8', '9', '0', '-', '-', '?', '?', 'A', 'Z', 'E', 'R', 'T', 'Y', 
        'U', 'I', 'O', 'P', '(', ')', '?', '?', 'Q', 'S', 'D', 'F', 'G', 
        'H', 'J', 'K', 'L', 'M', '?', '`', '?', '\\', 'W', 'X', 'C', 'V', 
        'B', 'N', ',', ';', ':', '=', '?', '?', '?', ' '};

const char sc_name[58][10] = { "ERROR", "Esc", "1", "2", "3", "4", "5", "6", 
    "7", "8", "9", "0", "-", "=", "Backspace", "Tab", "Q", "W", "E", 
        "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Lctrl", 
        "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`", 
        "LShift", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".", 
        "/", "RShift", "Keypad *", "LAlt", "Spacebar"};


static void keyboard_callback(registers_t regs){
    unsigned char scancode = port_byte_in(0x60);
    keyboard_program[current_program](scancode);
}

void init_keyboard(){
    register_interrupt_handler(IRQ1, keyboard_callback);
}