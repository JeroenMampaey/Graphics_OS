#include "networking.h"
#include "network_stack.h"
/*
    Driver to use the Intel PRO/1000 MT Desktop (82540EM)
    Based on:
        -https://wiki.osdev.org/Intel_Ethernet_i217
        -https://github.com/blanham/ChickenOS/blob/master/src/device/net/e1000.c
        -https://www.intel.com/content/dam/doc/manual/pci-pci-x-family-gbe-controllers-software-dev-manual.pdf
*/

//private functions 
void writeCommand(unsigned short address, unsigned int value);
unsigned int readCommand(unsigned short address);
void NIC_handler(registers_t regs);
unsigned int readEEPROM(unsigned int addr);
int readMACAddress();
unsigned int pci_read_dword(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
void pci_write_dword(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned int dword);
void rxinit();
void txinit();

//writing to a register
//register addresses found at https://www.intel.com/content/dam/doc/manual/pci-pci-x-family-gbe-controllers-software-dev-manual.pdf page 219
void writeCommand(unsigned short address, unsigned int value){
    *((volatile unsigned int*)(base+address)) = value;
}

//reading from a register
//register addresses found at https://www.intel.com/content/dam/doc/manual/pci-pci-x-family-gbe-controllers-software-dev-manual.pdf page 219
unsigned int readCommand(unsigned short address){
    return *((volatile unsigned int*)(address+base));
}

//interrupt handler code when a packet is received
void NIC_handler(registers_t regs){
    writeCommand(0x00D0, 0x1);

    unsigned int status = readCommand(0xc0);
    if(status & 0x04){
        unsigned int val;
        val = readCommand(0x0000);
        writeCommand(0x0000, val | 0x40);
    }
    else if(status & 0x10){
        printk("good threshold.\n");
    }
    else if(status & 0x80){
        unsigned short old_cur;
        while((rx_descs[rx_cur]->status & 0x1))
        {
                handleReceive((char*)rx_descs[rx_cur]->addr_low, rx_descs[rx_cur]->length);
                rx_descs[rx_cur]->status = 0;
                old_cur = rx_cur;
                rx_cur = (rx_cur + 1) % 32;
                writeCommand(0x2818, old_cur);
        }
    }
}

//reading from EEPROM
unsigned int readEEPROM(unsigned int addr){
    unsigned short data = 0;
    unsigned int tmp = 0;
    writeCommand(0x14, 1 | (addr << 8));
    while(!((tmp = readCommand(0x14)) & 0x10));
    data = (unsigned short)((tmp >> 16) & 0xFFFF);
    return data;
}

//reading the MAC address
int readMACAddress(){
    printk("MAC-address: ");
    unsigned int tmp;
    tmp = readEEPROM(0);
    mac[0] = tmp & 0xFF;
    mac[1] = tmp >> 8;
    tmp = readEEPROM(1);
    mac[2] = tmp & 0xFF;
    mac[3] = tmp >> 8;
    tmp = readEEPROM(2);
    mac[4] = tmp & 0xFF;
    mac[5] = tmp >> 8;
    char to_print[10];
    int_to_ascii((int)(unsigned char)mac[0], to_print);
    printk(to_print);
    printk("-");
    int_to_ascii((int)(unsigned char)mac[1], to_print);
    printk(to_print);
    printk("-");
    int_to_ascii((int)(unsigned char)mac[2], to_print);
    printk(to_print);
    printk("-");
    int_to_ascii((int)(unsigned char)mac[3], to_print);
    printk(to_print);
    printk("-");
    int_to_ascii((int)(unsigned char)mac[4], to_print);
    printk(to_print);
    printk("-");
    int_to_ascii((int)(unsigned char)mac[5], to_print);
    printk(to_print);
    printk("\n");
}

//reading a dword from the PCI
unsigned int pci_read_dword(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset){
    unsigned int lbus  = (unsigned int)bus;
    unsigned int lslot = (unsigned int)slot;
    unsigned int lfunc = (unsigned int)func;
    unsigned int address = (unsigned int)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((unsigned int)0x80000000));
    port_dword_out((unsigned short)0xCF8, address);
    unsigned int retval = port_dword_in((unsigned short)0xCFC);
    return retval;
}

//writing a dword to the PCI
void pci_write_dword(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned int dword){
    unsigned int lbus  = (unsigned int)bus;
    unsigned int lslot = (unsigned int)slot;
    unsigned int lfunc = (unsigned int)func;
    unsigned int address = (unsigned int)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((unsigned int)0x80000000));
    port_dword_out((unsigned short)0xCF8, address);
    port_dword_out((unsigned short)0xCFC, dword);
}

//receiver initialisation
void rxinit(){
    unsigned char* ptr;
    e1000_rx_desc *descs;
    // Allocate buffer for receive descriptors.
    ptr = (unsigned char*)RECEIVE_BUFFER;
    descs = (e1000_rx_desc*)ptr;
    for(int i = 0; i < 32; i++)
    {
        rx_descs[i] = descs+i;
        rx_descs[i]->addr_low = RECEIVE_BUFFER+0x1000+0x3000*i;
        rx_descs[i]->addr_high = 0;
        rx_descs[i]->status = 0;
    }
    writeCommand(0x2800, (unsigned int)ptr);
    writeCommand(0x2804, 0);
    writeCommand(0x2808, 32*16);
    writeCommand(0x2810, 0);
    writeCommand(0x2818, 32-1);
    rx_cur = 0;
    writeCommand(0x0100, RCTL_EN| RCTL_SBP| RCTL_UPE | RCTL_MPE | RCTL_LBM_NONE | RTCL_RDMTS_HALF | RCTL_BAM | RCTL_SECRC  | RCTL_BSIZE_8192);
}

//transmission initialization
void txinit(){    
    unsigned char*  ptr;
    e1000_tx_desc *descs;
    // Allocate buffer for receive descriptors
    ptr = (unsigned char*)TRANSMISSION_BUFFER;
    descs = (e1000_tx_desc *)ptr;
    for(int i = 0; i < 8; i++)
    {
        tx_descs[i] = (e1000_tx_desc *)((unsigned char*)descs + i*16);
        tx_descs[i]->addr_low = 0;
        tx_descs[i]->addr_high = 0;
        tx_descs[i]->cmd = 0;
        tx_descs[i]->status = TSTA_DD;
    }
    writeCommand(0x3800, (unsigned int)(ptr));
    writeCommand(0x3804, 0);
    //now setup total length of descriptors
    writeCommand(0x3808, 8*16);
    //setup numbers
    writeCommand(0x3810, 0);
    writeCommand(0x3818, 0);
    tx_cur = 0;
    writeCommand(0x0400,  TCTL_EN
        | TCTL_PSP
        | (15 << TCTL_CT_SHIFT)
        | (64 << TCTL_COLD_SHIFT)
        | TCTL_RTLC);
    //comments from original author (https://wiki.osdev.org/Intel_Ethernet_i217):
    //(I haven't tested yet whether this is necessary myself)
    // This line of code overrides the one before it but I left both to highlight that the previous one works with e1000 cards, but for the e1000e cards 
    // you should set the TCTRL register as follows. For detailed description of each bit, please refer to the Intel Manual.
    // In the case of I217 and 82577LM packets will not be sent if the TCTRL is not configured using the following bits.
    writeCommand(0x0400,  0b0110000000000111111000011111010);
    writeCommand(0x0410,  0x0060200A);
}

void init_e1000(){
    unsigned int ldevice_ID = (unsigned int)0x100E;
    unsigned int lvendor_ID = (unsigned int)0x8086;
    int found = 0;
    unsigned char bus = 0;
    unsigned char slot = 0;
    do{
        for(slot = 0; slot<32 && found==0;slot++){
             if(pci_read_dword(bus, slot, 0, 0) == ((ldevice_ID << 16) | lvendor_ID)){
                unsigned int class_and_revision = pci_read_dword(bus, slot, 0, 0x8);
                if((class_and_revision >> 8) == 0x20000){
                    printk("Found the e1000 ethernet card!\n");
                    found = 1;
                }
             }
        }
        bus++;
    } while((bus != 0) && found==0);
    if(found==0){
        printk("Didn't find an e1000 ethernet card, OS won't start without it.\n");
        while(1);
    }
    bus--;
    slot--;
    unsigned int bar0 = pci_read_dword(bus, slot, 0, 0x10);
    if((bar0 & 1) > 0){
        printk("IO space is used, it is not supported yet however so OS won't start.\n");  //TODO, allow IO space to work aswell
        while(1);
    }
    else printk("Memory space is used.\n");
    bar0 = bar0 & ~3;
    base = bar0;
    unsigned int status_and_command = pci_read_dword(bus, slot, 0, 0x4);
    if((status_and_command & 0b100) > 0){
        printk("Bus mastering was already enabled.\n");
    }
    else{
        printk("Bus mastering is disabled.\n");
        pci_write_dword(bus, slot, 0, 0x4, status_and_command | 0b110);
        status_and_command = pci_read_dword(bus, slot, 0, 0x4);
        if((status_and_command & 0b100) > 0){
            printk("Bus mastering has now been enabled.\n");
        }
        else{
            printk("Bus mastering is still disabled, OS won't start until it is enabled.\n");
            while(1);
        }
    }
    int eeprom_exists = 0;
    unsigned int retval;
    int i=0;
    writeCommand((unsigned short)0x14, 0x1);
    for(; i<1000 && eeprom_exists==0; i++){
        retval = readCommand(0x14);
        if((retval & 0x10) > 0){
            eeprom_exists = 1;
        }
    }
    if(eeprom_exists==0){
        printk("EEPROM does not exist, only EEPROM is supported at the moment so OS won't start.\n");  //TODO, allow no EEPROM support
        while(1);
    }
    else{
        printk("EEPROM exists.\n");
    }
    readMACAddress();
    retval = readCommand(0x0000);
    writeCommand(0x0000, retval | 0x40);
    for(int i = 0; i < 0x80; i++) writeCommand(0x5200 + i*4, 0);
    printk("Linkup is done.\n");
    int interrupt_line = pci_read_dword(bus, slot, 0, 0x3C) & 0xFF;
    register_interrupt_handler(IRQ0+interrupt_line, NIC_handler);
    writeCommand(0x00D0 ,0x1F6DC);
    writeCommand(0x00D0 ,0xFF & ~4);
    readCommand(0xC0);
    printk("Interrupt handler has been installed and interrupts have been enabled.\n");
    rxinit();
    printk("Receiver initialisation is done.\n");
    txinit();
    printk("Transmission initialisation is done.\n");
    printk("E1000 setup is done!\n");
}

void sendPacket(char* data, unsigned short length){    
    tx_descs[tx_cur]->addr_low = (unsigned int)data;
    tx_descs[tx_cur]->addr_high = 0;
    tx_descs[tx_cur]->length = length;
    tx_descs[tx_cur]->cmd = CMD_EOP | CMD_IFCS | CMD_RS;
    tx_descs[tx_cur]->status = 0;
    unsigned char old_cur = tx_cur;
    tx_cur = (tx_cur + 1) % 8;
    writeCommand(0x3818, tx_cur);
    while(!(tx_descs[old_cur]->status & 0xff));
}