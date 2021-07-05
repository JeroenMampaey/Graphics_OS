#ifndef KEYB_H
#define KEYB_H

#define BACKSPACE 0x0E
#define ENTER 0x1C
#define KEY_UP 0x48
#define KEY_DOWN 0x50
#define KEY_RIGHT 0x4D
#define KEY_LEFT 0x4B
#define SC_MAX 57

extern const char sc_name[58][10];
extern const char sc_ascii[];

void init_keyboard();

#endif