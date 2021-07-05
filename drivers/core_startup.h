#ifndef CORE_STARTUP_H
#define CORE_STARTUP_H

#define AP_BOOT_VECTOR 0x1
#include "../cpu/timer.h"
#include "../cpu/isr.h"

int num_cores;

int BSP_ID;

char* RSDP_address;
char* RSDT_address;
char* FADT_address;
char* MADT_address;
char* LOCAL_APIC_address;

int reply;

void start_core(int id);

void execCPUID(registers_t* regs);

int checkCPUID();

void setup_multicore();

void testRequest();

void testReply();

void wakeUp();

void sendIPI(int core_id, int ipi_int_number);

int getAPIC_ID();

#endif