#ifndef IDT_H
#define IDT_H


#define KERNEL_CS 0x08
#define low_16(address) (unsigned short)((address) & 0xFFFF);
#define high_16(address) (unsigned short)(((address) >> 16) & 0xFFFF);

//interrupt gate (handler)
typedef struct {
	unsigned short low_offset;  //lower 16 bits of handler function address
	unsigned short sel;   //kernel segment selector
	unsigned char always0;
	unsigned char flags;
	unsigned short high_offset; //higher 16 bits of handler function address
} __attribute((packed)) idt_gate_t;

//pointer to the array of interrupt handlers
typedef struct {
	unsigned short limit;
	unsigned int base;
} __attribute__((packed)) idt_register_t;

#define IDT_ENTRIES 256
idt_gate_t idt[IDT_ENTRIES];
idt_register_t idt_reg;

void set_idt_gate(int n, unsigned int handler);
void set_idt();

#endif
