#ifndef KERNEL_H
#define KERNEL_H

#define MEMORY_MAP 0x91000

int current_program;
void (*loop_program[3])(void);
void (*keyboard_program[3])(unsigned char);
void (*timer_program[3])(void);

#endif