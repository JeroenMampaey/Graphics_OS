#ifndef KERNEL_H
#define KERNEL_H

#define MEMORY_MAP 0x91000

int current_program;
void (*loop_program[2])(void);
void (*keyboard_program[2])(unsigned char);
void (*timer_program[2])(void);

#endif