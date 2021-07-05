#include "core_startup.h"

//private functions
int validate_RSDP(char* RSDP, int ACPI_version);
int validate_RSDT(char* RSDT, unsigned int length);
void read_RSDP(int ACPI_version);
void coreInfo();
int getAPIC_ID();


void setup_multicore(){
    //check whether CPU supports CPUID
    int result = checkCPUID();
    if(result!=0) printk("CPUID has been found.\n");
    else{
        printk("CPUID is not available, OS won't start without CPUID.\n");
        while(1);
    }

    //check APIC availability
    registers_t regs;
    regs.eax = 1;
    execCPUID(&regs);
    if((regs.edx && 0b1000000000) == 0){
        printk("CPU does not have a local APIC, OS won't start without APIC.\n");
        while(1);
    }
    else{
        printk("Local APIC is available.\n");
    }

    //find RSDP
    //RSDP can be found by looking for the string "RSD PTR " in the extended BIOS area or the main BIOS area
    //it would be even better to try to find out the size of the extended BIOS area first
    int found = 0;
    char* found_address;
    //look in the extended BIOS data area
    char* lookaddres = (char*)0x9FC00;
    for(; lookaddres<(char*)0xA0000 && found==0; lookaddres+=2){
        if((char)(*(lookaddres))=='R' && (char)(*(lookaddres+1))=='S' && (char)(*(lookaddres+2))=='D' && (char)(*(lookaddres+3))==' ' && (char)(*(lookaddres+4))=='P' && (char)(*(lookaddres+5))=='T' && (char)(*(lookaddres+6))=='R' && (char)(*(lookaddres+7))==' '){
            found = 1;
            found_address = lookaddres;
        }
    }
    //look in the main BIOS data area
    lookaddres = (char*)0x000E0000;
    for(; lookaddres<(char*)0x000FFFFF && found==0; lookaddres+=2){
        if((char)(*(lookaddres))=='R' && (char)(*(lookaddres+1))=='S' && (char)(*(lookaddres+2))=='D' && (char)(*(lookaddres+3))==' ' && (char)(*(lookaddres+4))=='P' && (char)(*(lookaddres+5))=='T' && (char)(*(lookaddres+6))=='R' && (char)(*(lookaddres+7))==' '){
            found = 1;
            found_address = lookaddres;
        }
    }
    if(found > 0){
        unsigned char revision = (unsigned char)(*(found_address+15));
        int is_valid;
        if(revision==0){
            printk("ACPI version 1.0\n");
            is_valid = validate_RSDP(found_address, 0);
        }
        else{
            printk("ACPI version is higher than 1.0\n");
            is_valid = validate_RSDP(found_address, 1);
        }
        if(is_valid==0){
            printk("RSDP appears to be invalid, OS won't start without a valid RSDP.\n");
            while(1);   
        }
        else{
            printk("RSDP address is valid\n");
            RSDP_address = found_address;
            read_RSDP((revision==0) ? 0 : 1);
        }
    }
    else{
        printk("RSDP has not been found, OS won't start without RSDP.\n");
        while(1);
    }
    
    //find all cores on this processor
    //also find the LOCAL APIC
    coreInfo();
    char to_print[10];
    BSP_ID = getAPIC_ID();
    int_to_ascii(BSP_ID, to_print);
    printk("APIC ID of the executing processor is ");
    printk(to_print);
    printk(".\n");

    //start every available core
    use_single_int_mode();
    for(int i=0; i<num_cores; i++){
        start_core(i);
    }

    for(int i=0; i<num_cores; i++){
        if(i != BSP_ID){
            reply = 0;
            sendIPI(i, 0);
            sleep_mili(10);
            printk("Core with APIC ID ");
            int_to_ascii(i, to_print);
            printk(to_print);
            if(reply==0){
                printk(" is either not working or to slow, OS won't start until this core if fixed.\n");
                while(1);
            }
            else{
                printk(" is working normally.\n");
            }
        }
    }
    printk("Every core is working normally.\n");
    frequency_mode(60);

    //enable APIC interrupts if they weren't enabled already
	*((volatile unsigned int*)(LOCAL_APIC_address + 0xF0)) = *((volatile unsigned int*)(LOCAL_APIC_address + 0xF0)) | 0x100;
}

//startup a core with APIC ID "id"
//https://wiki.osdev.org/SMP
void start_core(int id){
    if(id >= num_cores){
        printk("Core with given ID does not exist\n");
        return;
    }
    else if(id == BSP_ID){
        printk("Core with given ID is already running.\n");
        return;
    }
    *((volatile unsigned int*)(LOCAL_APIC_address + 0x280)) = 0;
    *((volatile unsigned int*)(LOCAL_APIC_address + 0x310)) = (*((volatile unsigned int*)(LOCAL_APIC_address + 0x310)) & 0x00ffffff) | (id << 24);
    *((volatile unsigned int*)(LOCAL_APIC_address + 0x300)) = (*((volatile unsigned int*)(LOCAL_APIC_address + 0x300)) & 0xfff00000) | 0x00C500;
    do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile unsigned int*)(LOCAL_APIC_address + 0x300)) & (1 << 12));
    *((volatile unsigned int*)(LOCAL_APIC_address + 0x310)) = (*((volatile unsigned int*)(LOCAL_APIC_address + 0x310)) & 0x00ffffff) | (id << 24);
    *((volatile unsigned int*)(LOCAL_APIC_address + 0x300)) = (*((volatile unsigned int*)(LOCAL_APIC_address + 0x300)) & 0xfff00000) | 0x008500;
    do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile unsigned int*)(LOCAL_APIC_address + 0x300)) & (1 << 12));
    sleep_mili(10);
    for(int j=0; j < 2; j++){
        *((volatile unsigned int*)(LOCAL_APIC_address + 0x280)) = 0; 
        *((volatile unsigned int*)(LOCAL_APIC_address + 0x310)) = (*((volatile unsigned int*)(LOCAL_APIC_address + 0x310)) & 0x00ffffff) | (id << 24);
        *((volatile unsigned int*)(LOCAL_APIC_address + 0x300)) = (*((volatile unsigned int*)(LOCAL_APIC_address + 0x300)) & 0xfff0f800) | 0x000600 | AP_BOOT_VECTOR;
        sleep_micro(200);
        do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile unsigned int*)(LOCAL_APIC_address + 0x300)) & (1 << 12));
    }
    printk("A core has started.\n");
}

void sendIPI(int core_id, int ipi_int_number){
    *((volatile unsigned int*)(LOCAL_APIC_address + 0x310)) = (*((volatile unsigned int*)(LOCAL_APIC_address + 0x310)) & 0x00ffffff) | (core_id << 24);
    *((volatile unsigned int*)(LOCAL_APIC_address + 0x300)) = (*((volatile unsigned int*)(LOCAL_APIC_address + 0x300)) & 0xfff00000) | (0x04000 | 0x30+ipi_int_number);
}

void testRequest(){
    sendIPI(BSP_ID, 1);
}

void testReply(){
    reply = 1;
}

void wakeUp(){
    //DO NOTHING
}


//result is zero if CPUID is not available
int checkCPUID(){
    //https://wiki.osdev.org/CPUID
    //CPUID availability is detected by trying to modify bit 0x200000 in eflags
    //this bit is modifiable only when the CPUID instruction is supported
    __asm__ __volatile__(
        ".intel_syntax noprefix;"
        "push eax;"
        "pushfd;"
        "pushfd;"
        "mov eax, 0x00200000;"
        "xor dword [esp], eax;"
        "popfd;"
        "pushfd;"
        "pop eax;"
        "xor eax,[esp];"
        "popfd;"
        ".att_syntax;"
    );
    unsigned int result;
    __asm__(".intel_syntax noprefix;and eax,0x00200000;.att_syntax" : "=a" (result) : );
    __asm__ __volatile__(
        ".intel_syntax noprefix;"
        "pop eax;"
        ".att_syntax"
    );
    return result;
}

void execCPUID(registers_t* regs){
    __asm__ __volatile__(
        ".intel_syntax noprefix;"
        "push eax;"
        "push ebx;"
        "push edx;"
        "push ecx;"
        ".att_syntax;"
    );
    __asm__ __volatile__(
        ".intel_syntax noprefix;"
        "cpuid;"
        ".att_syntax;"
    : "=b"(regs->ebx), "=c"(regs->ecx), "=d"(regs->edx) : "a"(regs->eax));
    __asm__ __volatile__(
        ".intel_syntax noprefix;"
        "pop ecx;"
        "pop edx;"
        "pop ebx;"
        "pop eax;"
        ".att_syntax;"
    );
}

//validate the correctness of the RSDP
//https://wiki.osdev.org/RSDP
//ACPI_version = 0 <-> ACPI 1.0
//ACPI_version = 1 <-> ACPI 2.0 or higher
//returns 0 if the RSDP is invalid, 1 if it is valid
int validate_RSDP(char* RSDP, int ACPI_version){
    int total_sum = 0;
    for(int i=0; i<20; i++){
        total_sum += (int)(unsigned char)(*(RSDP+i));
    }
    if((unsigned char)total_sum != 0) return 0;
    else if(ACPI_version==0) return 1;
    else{
        total_sum = 0;
        for(int i=20; i<36; i++){
            total_sum += (int)(unsigned char)(*(RSDP+i));
        }
        if((unsigned char)total_sum != 0) return 0;
        else return 1;
    }
}

//validate the correctness of the RSDT
//by counting all bytes in the structure and make sure the
//lower byte of the sum is zero
//https://wiki.osdev.org/XSDT
//https://wiki.osdev.org/RSDT
int validate_RSDT(char* RSDT, unsigned int length){
    int total_sum = 0;
    for(int i=0; i<length; i++){
        total_sum += (int)(unsigned char)(*(RSDT+i));
    }
    return ((unsigned char)total_sum == 0) ? 1 : 0;
}

//save some things from the RSDP so I do not need to look them up every time 
void read_RSDP(int ACPI_version){
    //RSDP structure can be found at https://wiki.osdev.org/RSDP
    //I assume the XSDT address to be limited to 32-bits (which it most likely is)
    //even when i use the XSDT, i'll still call it the RSDT to avoid making it confusing
    int* address = (int*)RSDP_address;
    int* RSDT_pointer;
    address += (ACPI_version==0) ? 4 : 6;
    RSDT_pointer = (int*)(*address);
    RSDT_pointer++;
    unsigned int length = *RSDT_pointer;
    RSDT_pointer--;
    if(validate_RSDT((char*)RSDT_pointer, length)==1){
        printk("Valid RSDT has been found.\n");
        RSDT_address = (char*)RSDT_pointer;
    }
    else{
        printk("RSDT is invalid, OS won't start without a valid RSDT.\n");
        while(1);
    }
    unsigned int num_of_entries = (length-36)/8;
    RSDT_pointer += 9;
    for(unsigned int i=0; i<num_of_entries; i++, RSDT_pointer+=2){
        int* new_address = (int*)(*RSDT_pointer);
        unsigned int signature = *new_address;
        //0x50434146 <-> "PCAF", the signature of the FADT table (normally it's "FACP" but since intel uses little endian it is reversed)
        if(signature == 0x50434146){
            printk("Found the FADT.\n");
            FADT_address = (char*)new_address;
        }
        //0x43495041 <-> "CIPA", the signature of the MADT table (normally it's "APIC" but since intel uses little endian it is reversed)
        else if(signature == 0x43495041){
            printk("Found the MADT.\n");
            MADT_address = (char*)new_address;
        }
    }
}

//find all cores in the MADT and print display relevant information
//https://wiki.osdev.org/MADT
void coreInfo(){
    char to_print[10];
    char* address = MADT_address;
    int* address_int = (int*)MADT_address;
    address_int++;
    int length = *address_int;
    address_int += 8;
    LOCAL_APIC_address = (char*)(*address_int);
    length -= 44;
    address += 44;
    int count = 0;
    while(length>0){
        if(*address == 0){   //found a processor local APIC which indicates a single physical processor
            printk("Core found with APIC ID ");
            int_to_ascii((int)(*(address+3)), to_print);
            printk(to_print);
            printk(".\n");
            address += 8;
            length -= 8;
            count++;
        }
        else if(*address == 1){
            address += 12;
            length -= 12;
        }
        else if(*address == 2){
            address += 10;
            length -= 10;
        }
        else if(*address == 3){
            address += 10;
            length -= 10;
        }
        else if(*address == 4){
            address += 6;
            length -= 6;
        }
        else if(*address == 5){
            address += 12;
            length -= 12;
        }
        else if(*address == 9){
            address += 16;
            length -= 16;
        }
        else{
            printk("Invalid entry value found in the MADT: ");
            int_to_ascii((int)(*address), to_print);
            printk(to_print);
            printk("\n");
        }
    }
    num_cores = count;
}

//get APIC ID of processor that runs this code
//https://en.wikipedia.org/wiki/CPUID
int getAPIC_ID(){
    registers_t regs;
    regs.eax = 1;
    execCPUID(&regs);
    return (regs.ebx >> 24);
}