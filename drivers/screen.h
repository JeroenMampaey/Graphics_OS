#ifndef SCREEN_H
#define SCREEN_H

#include "../cpu/ports.h"
#include "../libc/mem.h"
#include "../libc/string.h"
#define VIDEO_MEMORY 0xA0000
#define FONT_TABLE 0x90000
#define DIST_TO_SCREEN 240  //480/2

//these instruction can certainly be optimized but it isn't that important, maybe later
#define SWAP_INT(x, y) temp_int = x; x = y; y = temp_int
#define SWAP_FLOAT(x, y) temp_float = x; x = y; y = temp_float;
#define SWAP_VECT_2D(x, y) temp_vect_2d = x; x = y; y = temp_vect_2d
#define SWAP_VECT_3D(x, y) temp_vect_3d = x; x = y; y = temp_vect_3d
#define ABS(x) (((x)>=0) ? (x) : -(x))

typedef struct{   //short takes less space in memory
    short x0;
    short y0;
    short z0;
    short x1;
    short y1;
    short z1;
    short x2;
    short y2;
    short z2;
    unsigned char color;
} triangle_3d_mem;

typedef struct{
    float x;
    float y;
    float z;
} float_vector_3d;

typedef struct{
    int x;
    int y;
    int z;
} int_vector_3d;

typedef struct{
    float x;
    float y;
    float z;
} float_vector_2d;

typedef struct{
    int x;
    int y;
    int z;
} int_vector_2d;

typedef struct{
    int_vector_3d p0;
    int_vector_3d p1;
    int_vector_3d p2;
} triangle_3d;

typedef struct{
    triangle_3d tr;
    float_vector_3d normal_vec;
    unsigned char color;
} AP_package;

typedef struct{
    int_vector_2d p0;
    int_vector_2d p1;
    int_vector_2d p2;
} triangle_2d;

//to be clear: aij means the i'th row and the j'th column from the matrix
typedef struct{
    float a00;
    float a01;
    float a02;
    float a03;
    float a10;
    float a11;
    float a12;
    float a13;
    float a20;
    float a21;
    float a22;
    float a23;
    float a30;
    float a31;
    float a32;
    float a33;
} matrix_4d;

static int temp_int;
static float temp_float;
static int_vector_2d temp_vect_2d;
static int_vector_3d temp_vect_3d;
static int current_char;

matrix_4d view_matrix;

int buffer_addr[4];

int triangle_buffer_addr;
int triangle_buffer_count;

int z_buffer_addr;

int AP_buffer_begin;

void init_screen();

extern void move_dwords_asm(int from, int to, int number_of_dwords);
extern void move_dwords_reverse_asm(int from, int to, int number_of_dwords);
extern void fill_bytes_asm(int addres, int number_of_bytes);
extern void fill_dwords_asm(int addres, int number_of_dwords);
extern void clear_dwords_asm(int addres, int number_of_dwords);

void set_write_color(unsigned char color);

void set_read_color(unsigned char plane);

void drawLine(int x0, int y0, int x1, int y1, char* draw_address);

void drawTriangle2D(triangle_2d triangle, char* draw_address);

void printk(char string[]);

void printk_backspace();

void put_on_screen(char* addr, int offset);

void print_char_at(char to_print, int offset);

void clear_screen();

void scroll_up();

void scroll_down();

void initialize_buffers();

void swap_buffers(int i);

void print_string_at(char string[], char* buffer_addr);

void print_int_at(int to_print, char* buffer_addr);

void display_triangle_buffer();

void drawTriangle3D(triangle_3d triangle, float_vector_3d normal_vec, unsigned char color);

float fast_hypo_3d(float x, float y, float z);

#endif
