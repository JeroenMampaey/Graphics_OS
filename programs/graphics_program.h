#ifndef GRAPHICS_PROGRAM_H
#define GRAPHICS_PROGRAM_H

#include "../drivers/keyboard.h"
#include "../drivers/screen.h"

static int control_count;

void graphics_loop_callback();

void graphics_keyboard_callback(unsigned char scancode);

void graphics_timer_callback();

void kernel_to_graphics();

void AP_drawing();

void show_screen();

#endif