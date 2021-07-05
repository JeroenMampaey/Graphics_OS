#include "isr.h"
#include "idt.h"
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "../libc/string.h"
#include "timer.h"
#include "ports.h"
#include "../drivers/core_startup.h"

void isr_install(){
	for(int i=0; i<32; i++){
		exception_messages[i] = i;
	}
	set_idt_gate(0, (unsigned int)isr0);
	set_idt_gate(1, (unsigned int)isr1);
	set_idt_gate(2, (unsigned int)isr2);
    set_idt_gate(3, (unsigned int)isr3);
    set_idt_gate(4, (unsigned int)isr4);
    set_idt_gate(5, (unsigned int)isr5);
    set_idt_gate(6, (unsigned int)isr6);
    set_idt_gate(7, (unsigned int)isr7);
	set_idt_gate(8, (unsigned int)isr8); 
	set_idt_gate(9, (unsigned int)isr9); 
	set_idt_gate(10, (unsigned int)isr10); 
	set_idt_gate(11, (unsigned int)isr11); 
	set_idt_gate(12, (unsigned int)isr12);
	set_idt_gate(13, (unsigned int)isr13); 
	set_idt_gate(14, (unsigned int)isr14); 
	set_idt_gate(15, (unsigned int)isr15); 
	set_idt_gate(16, (unsigned int)isr16); 
	set_idt_gate(17, (unsigned int)isr17); 
	set_idt_gate(18, (unsigned int)isr18); 
	set_idt_gate(19, (unsigned int)isr19); 
	set_idt_gate(20, (unsigned int)isr20); 
	set_idt_gate(21, (unsigned int)isr21); 
	set_idt_gate(22, (unsigned int)isr22); 
	set_idt_gate(23, (unsigned int)isr23); 
	set_idt_gate(24, (unsigned int)isr24);
	set_idt_gate(25, (unsigned int)isr25);
	set_idt_gate(26, (unsigned int)isr26); 
	set_idt_gate(27, (unsigned int)isr27); 
	set_idt_gate(28, (unsigned int)isr28); 
	set_idt_gate(29, (unsigned int)isr29); 
	set_idt_gate(30, (unsigned int)isr30); 
	set_idt_gate(31, (unsigned int)isr31);

    // Remap the PIC
	//The Master PIC has command 0x20 and data 0x21, while the slave has command 0xA0 and data 0xA1
	//https://wiki.osdev.org/PIC
    port_byte_out(0x20, 0x11);
    port_byte_out(0xA0, 0x11);
    port_byte_out(0x21, 0x20);
    port_byte_out(0xA1, 0x28);
    port_byte_out(0x21, 0x04);
    port_byte_out(0xA1, 0x02);
    port_byte_out(0x21, 0x01);
    port_byte_out(0xA1, 0x01);
    port_byte_out(0x21, 0x0);
    port_byte_out(0xA1, 0x0);

	// Install the IRQs
    set_idt_gate(32, (unsigned int)irq0);
    set_idt_gate(33, (unsigned int)irq1);
    set_idt_gate(34, (unsigned int)irq2);
    set_idt_gate(35, (unsigned int)irq3);
    set_idt_gate(36, (unsigned int)irq4);
    set_idt_gate(37, (unsigned int)irq5);
    set_idt_gate(38, (unsigned int)irq6);
    set_idt_gate(39, (unsigned int)irq7);
    set_idt_gate(40, (unsigned int)irq8);
    set_idt_gate(41, (unsigned int)irq9);
    set_idt_gate(42, (unsigned int)irq10);
    set_idt_gate(43, (unsigned int)irq11);
    set_idt_gate(44, (unsigned int)irq12);
    set_idt_gate(45, (unsigned int)irq13);
    set_idt_gate(46, (unsigned int)irq14);
    set_idt_gate(47, (unsigned int)irq15);

	set_idt_gate(48, (unsigned int)ipi0);
	set_idt_gate(49, (unsigned int)ipi1);
	set_idt_gate(50, (unsigned int)ipi2);
	
	set_idt();	
}

void isr_install_AP(){
	set_idt();
}

void isr_handler(registers_t r){
	printk("received interrupt: ");
	char received_int[3];
	int_to_ascii(exception_messages[r.int_no], received_int);
	printk(received_int);
	printk("\n");
}

void register_interrupt_handler(unsigned char n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

void irq_handler(registers_t r) {
    
	/* After every interrupt we need to send an EOI to the PICs
     * or they will not send another interrupt again */
    if (r.int_no >= 40) port_byte_out(0xA0, 0x20); /* slave */
    port_byte_out(0x20, 0x20); /* master */

    /* Handle the interrupt in a more modular way */
    if (interrupt_handlers[r.int_no] != 0) {
        isr_t handler = interrupt_handlers[r.int_no];
        handler(r);
    }
}

void ipi_handler(registers_t r){
	/* After every interrupt we need to send an EOI to the APIC
     * or it will not send another interrupt again */
	*((volatile unsigned int*)(LOCAL_APIC_address + 0xB0)) = 0;
	/* Handle the interrupt in a more modular way */
    if (interrupt_handlers[r.int_no] != 0) {
        isr_t handler = interrupt_handlers[r.int_no];
        handler(r);
    }
}

void ipi_install(){
	register_interrupt_handler(IPI0, testRequest);
	register_interrupt_handler(IPI1, testReply);
	register_interrupt_handler(IPI2, wakeUp);
}

void irq_install(){
	//timer can only have frequenties between 18.2065 hz and 1193182 hz
	//since divisor (see init_timer code) can only be a 16 bit number (max divisor is 65535, frequency of clock is 1193182 and thus 1193182/65535 gives this 18.2065 value)
	//https://en.wikibooks.org/wiki/X86_Assembly/Programmable_Interval_Timer
	init_timer(60);
	init_keyboard();
}
