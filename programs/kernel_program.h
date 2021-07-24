#ifndef KERNEL_PROGRAM_H
#define KERNEL_PROGRAM_H

#include "../drivers/keyboard.h"
#include "../libc/string.h"
#include "../drivers/screen.h"
#include "../cpu/isr.h"
#include "../cpu/timer.h"
#include "../drivers/disk_reader.h"
#include "../drivers/core_startup.h"


static char key_buffer[256];

static char command_ready;

void boot_to_kernel();

void editor_to_kernel();

void graphics_to_kernel();

void kernel_loop_callback();

void kernel_keyboard_callback(unsigned char scancode);

void kernel_timer_callback();

#endif