#include "networking.h"

unsigned int pci_read_dword(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset){
    unsigned int lbus  = (unsigned int)bus;
    unsigned int lslot = (unsigned int)slot;
    unsigned int lfunc = (unsigned int)func;
    unsigned int address = (unsigned int)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((unsigned int)0x80000000));
    port_dword_out((unsigned short)0xCF8, address);
    unsigned int retval = port_dword_in((unsigned short)0xCFC);
    return retval;
}

void find_brute_force(){
    unsigned int ldevice_ID = (unsigned int)0x100E;
    unsigned int lvendor_ID = (unsigned int)0x8086;
    int count = 0;
    unsigned char bus = 0;
    do{
        for(unsigned char slot = 0; slot < 32; slot++){
            if(pci_read_dword(bus, slot, 0, 0) == ((ldevice_ID << 16) | lvendor_ID)){
                unsigned int class_and_revision = pci_read_dword(bus, slot, 0, 0x8);
                if((class_and_revision >> 8) != 0x20000) return;
                printk("Found the ethernet card!\n");
                unsigned int bar0 = pci_read_dword(bus, slot, 0, 0x10);
                if((bar0 & 1) > 0){
                    printk("IO space is used.\n");
                }
                else{
                    printk("Memory space is used\n");
                    char to_print[20];
                    bar0 = bar0 & ~3;
                    int_to_ascii((int)bar0, to_print);
                    printk(to_print);
                }
            }
        }
        bus++;
    } while(bus != 0);
}