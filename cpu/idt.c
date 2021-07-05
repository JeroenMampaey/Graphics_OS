#include "idt.h"

void set_idt_gate(int n, unsigned int handler){
	idt[n].low_offset = low_16(handler);
	idt[n].sel = KERNEL_CS;
	idt[n].always0 = 0;
	idt[n].flags = 0x8E;
	/*
	 0x8E <--> 0x10001110

	 bit 7: interrupt is present
	 bit 6-5: privilige level of caller (0=kernel..3-user)
	 bit 4: set to 0 for interrupt gates
	 bits 3-0: 1110 <-> 14 <-> "32 bit interrupt gate"
	 */
	idt[n].high_offset = high_16(handler);
}

void set_idt(){
	idt_reg.base = (unsigned int) &idt;
	idt_reg.limit = IDT_ENTRIES*sizeof(idt_gate_t)-1;
	__asm__ __volatile__("lidtl (%0)" : : "r" (&idt_reg)); //dit komt blijkbaar overeen met "lidt [&idt_reg]"
}
