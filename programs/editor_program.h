#ifndef EDITOR_PROGRAM_H
#define EDITOR_PROGRAM_H

#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "kernel_program.h"

#define BUFFER_BEGIN 0x1000000

static int current_mem_addr;

static int current_mem_position;

void kernel_to_editor();

void editor_loop_callback();

void editor_keyboard_callback(unsigned char scancode);

void editor_timer_callback();

#endif