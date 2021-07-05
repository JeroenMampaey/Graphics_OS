#ifndef TIMER_H
#define TIMER_H

#include "../libc/string.h"
#include "../drivers/screen.h"
#include "../kernel/kernel.h"

int timer_counter;

void init_timer(unsigned int freq);

void frequency_mode(unsigned int freq);

void use_single_int_mode();

void sleep_micro(unsigned short micro);

void sleep_mili(unsigned short mili);

void sleep_seconds(unsigned short seconds);

#endif