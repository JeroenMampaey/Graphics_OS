#include "graphics_program.h"
#include "../drivers/screen.h"
#include "../drivers/core_startup.h"

//private functions
void demo();
//clears the screen aswell
void show_screen();
void multiply_view_matrix(matrix_4d* mul_matrix);
void add_cube(int x, int y, int z, int delta);
void matrix_control();
void reorthogonalize();

void kernel_to_graphics(){
    initialize_buffers();
    printk("Buffers have correctly been initialized!\n");
    //player start at position zero with no rotation
    view_matrix.a00=1; view_matrix.a01=0; view_matrix.a02=0; view_matrix.a03=0;
    view_matrix.a10=0; view_matrix.a11=1; view_matrix.a12=0; view_matrix.a13=0;
    view_matrix.a20=0; view_matrix.a21=0; view_matrix.a22=1; view_matrix.a23=0;
    view_matrix.a30=0; view_matrix.a31=0; view_matrix.a32=0; view_matrix.a33=1;

    demo();
    printk("Demo has been loaded!\n");
    clear_screen();

    control_count = 0;
}

void graphics_loop_callback(){
    //DO NOTHING
    //don't do a hlt since the CPU will then have to wake up everytime the player wants to move
}

void graphics_keyboard_callback(unsigned char scancode){
    if(scancode == (unsigned char)16){        //A-key
        view_matrix.a23+=10;
        show_screen();
        matrix_control();
    }
    else if(scancode == (unsigned char)30){        //Q-key
        view_matrix.a23-=10;
        show_screen();
        matrix_control();
    }
    else if(scancode == (unsigned char)72){   //upkey
        float cos = 0.990267;    //roughly an 8 degree rotation
        float sin = 0.13918;
        view_matrix.a23 -= DIST_TO_SCREEN;
        matrix_4d mul_matrix = {1, 0, 0, 0, 0, cos, sin, 0, 0, -sin, cos, 0, 0, 0, 0, 1};
        multiply_view_matrix(&mul_matrix);
        view_matrix.a23 += DIST_TO_SCREEN;
        show_screen();
        matrix_control();
    }
    else if(scancode == (unsigned char)80){   //downkey
        float cos = 0.990267;    //roughly an 8 degree rotation
        float sin = -0.13918;
        view_matrix.a23 -= DIST_TO_SCREEN;
        matrix_4d mul_matrix = {1, 0, 0, 0, 0, cos, sin, 0, 0, -sin, cos, 0, 0, 0, 0, 1};
        multiply_view_matrix(&mul_matrix);
        view_matrix.a23 += DIST_TO_SCREEN;
        show_screen();
        matrix_control();
    }
    else if(scancode == (unsigned char)75){   //leftkey
        float cos = 0.990267;    //roughly an 8 degree rotation
        float sin = -0.13918;
        view_matrix.a23 -= DIST_TO_SCREEN;
        matrix_4d mul_matrix = {cos, 0, sin, 0, 0, 1, 0, 0, -sin, 0, cos, 0, 0, 0, 0, 1};
        multiply_view_matrix(&mul_matrix);
        view_matrix.a23 += DIST_TO_SCREEN;
        show_screen();
        matrix_control();
    }
    else if(scancode == (unsigned char)77){    //rightkey
        float cos = 0.990267;    //roughly an 8 degree rotation
        float sin = 0.13918;
        view_matrix.a23 -= DIST_TO_SCREEN;
        matrix_4d mul_matrix = {cos, 0, sin, 0, 0, 1, 0, 0, -sin, 0, cos, 0, 0, 0, 0, 1};
        multiply_view_matrix(&mul_matrix);
        view_matrix.a23 += DIST_TO_SCREEN;
        show_screen();
        matrix_control();
    }

}

void graphics_timer_callback(){
    //DO NOTHING
}

void demo(){
    add_cube(50, 50, 0, 100);
    add_cube(-100, -100, 50, 100);
    add_cube(100, -50, -100, 50);
    add_cube(50, -100, 250, 30);
    add_cube(50, 50, 0, 100);
    add_cube(-100, -100, 50, 100);
    add_cube(100, -50, -100, 50);
    add_cube(50, -100, 250, 30);
    add_cube(50, 50, -200, 100);
    add_cube(-100, -100, -250, 100);
    add_cube(100, -50, -300, 50);
    add_cube(50, -100, -50, 30);
    add_cube(100, 50, -200, 100);
    add_cube(0, -100, -250, 100);
    add_cube(200, -50, -300, 50);
    add_cube(150, -100, -50, 30);
    add_cube(250, 50, 0, 100);
    add_cube(100, -100, 50, 100);
    add_cube(300, -50, -100, 50);
    add_cube(250, -100, 250, 30);
    add_cube(250, 50, -200, 100);
    add_cube(100, -100, -250, 100);
    add_cube(300, -50, -300, 50);
    add_cube(250, -100, -50, 30);
    add_cube(300, 50, -200, 100);
    add_cube(200, -100, -250, 100);
    add_cube(400, -50, -300, 50);
    add_cube(350, -100, -50, 30);
    add_cube(-100, -50, -100, 50);
    add_cube(-150, -100, 250, 30);
    add_cube(-150, 50, -200, 100);
    add_cube(-300, -100, -250, 100);
    add_cube(-100, -50, -300, 50);
    add_cube(-150, -100, -50, 30);
    add_cube(-100, 50, -200, 100);
    add_cube(-200, -100, -250, 100);
    add_cube(0, -50, -300, 50);
    add_cube(-50, -100, -50, 30);
    add_cube(50, 250, -200, 100);
    add_cube(-100, 200, -250, 100);
    add_cube(100, 150, -300, 50);
    add_cube(50, 100, -50, 30);
    add_cube(100, 250, -200, 100);
    add_cube(0, 100, -250, 100);
    add_cube(200, 150, -300, 50);
}

void multiply_view_matrix(matrix_4d* mul_matrix){
    view_matrix.a00 = mul_matrix->a00*view_matrix.a00+mul_matrix->a01*view_matrix.a10+mul_matrix->a02*view_matrix.a20+mul_matrix->a03*view_matrix.a30;
    view_matrix.a10 = mul_matrix->a10*view_matrix.a00+mul_matrix->a11*view_matrix.a10+mul_matrix->a12*view_matrix.a20+mul_matrix->a13*view_matrix.a30;
    view_matrix.a20 = mul_matrix->a20*view_matrix.a00+mul_matrix->a21*view_matrix.a10+mul_matrix->a22*view_matrix.a20+mul_matrix->a23*view_matrix.a30;
    view_matrix.a30 = mul_matrix->a30*view_matrix.a00+mul_matrix->a31*view_matrix.a10+mul_matrix->a32*view_matrix.a20+mul_matrix->a33*view_matrix.a30;
    view_matrix.a01 = mul_matrix->a00*view_matrix.a01+mul_matrix->a01*view_matrix.a11+mul_matrix->a02*view_matrix.a21+mul_matrix->a03*view_matrix.a31;
    view_matrix.a11 = mul_matrix->a10*view_matrix.a01+mul_matrix->a11*view_matrix.a11+mul_matrix->a12*view_matrix.a21+mul_matrix->a13*view_matrix.a31;
    view_matrix.a21 = mul_matrix->a20*view_matrix.a01+mul_matrix->a21*view_matrix.a11+mul_matrix->a22*view_matrix.a21+mul_matrix->a23*view_matrix.a31;
    view_matrix.a31 = mul_matrix->a30*view_matrix.a01+mul_matrix->a31*view_matrix.a11+mul_matrix->a32*view_matrix.a21+mul_matrix->a33*view_matrix.a31;
    view_matrix.a02 = mul_matrix->a00*view_matrix.a02+mul_matrix->a01*view_matrix.a12+mul_matrix->a02*view_matrix.a22+mul_matrix->a03*view_matrix.a32;
    view_matrix.a12 = mul_matrix->a10*view_matrix.a02+mul_matrix->a11*view_matrix.a12+mul_matrix->a12*view_matrix.a22+mul_matrix->a13*view_matrix.a32;
    view_matrix.a22 = mul_matrix->a20*view_matrix.a02+mul_matrix->a21*view_matrix.a12+mul_matrix->a22*view_matrix.a22+mul_matrix->a23*view_matrix.a32;
    view_matrix.a32 = mul_matrix->a30*view_matrix.a02+mul_matrix->a31*view_matrix.a12+mul_matrix->a32*view_matrix.a22+mul_matrix->a33*view_matrix.a32;
    view_matrix.a03 = mul_matrix->a00*view_matrix.a03+mul_matrix->a01*view_matrix.a13+mul_matrix->a02*view_matrix.a23+mul_matrix->a03*view_matrix.a33;
    view_matrix.a13 = mul_matrix->a10*view_matrix.a03+mul_matrix->a11*view_matrix.a13+mul_matrix->a12*view_matrix.a23+mul_matrix->a13*view_matrix.a33;
    view_matrix.a23 = mul_matrix->a20*view_matrix.a03+mul_matrix->a21*view_matrix.a13+mul_matrix->a22*view_matrix.a23+mul_matrix->a23*view_matrix.a33;
    view_matrix.a33 = mul_matrix->a30*view_matrix.a03+mul_matrix->a31*view_matrix.a13+mul_matrix->a32*view_matrix.a23+mul_matrix->a33*view_matrix.a33;
}

void matrix_control(){
    /*
        Since it's impossible to keep a matrix completely orthogonal, I keep the matrix 
        orthogonal manually, after a number of moves I renormalize all basis vectors and I use crossproduct to
        pairwise reorthogonalize the basisvectors
    */
    if(control_count < 5){
        control_count++;
        return;
    }
    else control_count = 0;
    
    reorthogonalize();

    float length1 = fast_hypo_3d(view_matrix.a00, view_matrix.a10, view_matrix.a20);
    float length2 = fast_hypo_3d(view_matrix.a01, view_matrix.a11, view_matrix.a21);
    float length3 = fast_hypo_3d(view_matrix.a02, view_matrix.a12, view_matrix.a22);

    view_matrix.a00 *= 1/length1; 
    view_matrix.a10 *= 1/length1; 
    view_matrix.a20 *= 1/length1;
    
    view_matrix.a01 *= 1/length2; 
    view_matrix.a11 *= 1/length2; 
    view_matrix.a21 *= 1/length2;
    
    view_matrix.a02 *= 1/length3;
    view_matrix.a12 *= 1/length3; 
    view_matrix.a22 *= 1/length3;
}

void reorthogonalize(){
    view_matrix.a01 = view_matrix.a12*view_matrix.a20-view_matrix.a10*view_matrix.a22;
    view_matrix.a11 = -view_matrix.a02*view_matrix.a20+view_matrix.a00*view_matrix.a22;
    view_matrix.a21 = view_matrix.a02*view_matrix.a10-view_matrix.a00*view_matrix.a12;

    view_matrix.a02 = view_matrix.a10*view_matrix.a21-view_matrix.a11*view_matrix.a20;
    view_matrix.a12 = -view_matrix.a00*view_matrix.a21+view_matrix.a01*view_matrix.a20;
    view_matrix.a22 = view_matrix.a00*view_matrix.a11-view_matrix.a01*view_matrix.a10;
}

void show_screen(){
    clear_dwords_asm(buffer_addr[0], (int)(480*80));
    display_triangle_buffer();
    swap_buffers(0);
    swap_buffers(1);
    swap_buffers(2);
    swap_buffers(3);
}

void add_cube(int x, int y, int z, int delta){
    triangle_3d_mem* addr = (triangle_3d_mem*)triangle_buffer_addr;
    addr += triangle_buffer_count;

    //front
    triangle_3d_mem tr1 = {x, y, z, x+delta, y, z, x+delta, y+delta, z, 0b00000001};
    triangle_3d_mem tr2 = {x, y, z, x, y+delta, z, x+delta, y+delta, z, 0b00000001};
    *addr = tr1;
    addr++;
    *addr = tr2;
    addr++;

    //left
    triangle_3d_mem tr3 = {x, y, z, x, y, z-delta, x, y+delta, z-delta, 0b00000010};
    triangle_3d_mem tr4 = {x, y, z, x, y+delta, z, x, y+delta, z-delta, 0b00000010};
    *addr = tr3;
    addr++;
    *addr = tr4;
    addr++;

    //right
    triangle_3d_mem tr5 = {x+delta, y, z, x+delta, y+delta, z, x+delta, y+delta, z-delta, 0b00000011};
    triangle_3d_mem tr6 = {x+delta, y, z, x+delta, y, z-delta, x+delta, y+delta, z-delta, 0b00000011};
    *addr = tr5;
    addr++;
    *addr = tr6;
    addr++;

    //back
    triangle_3d_mem tr7 = {x, y, z-delta, x+delta, y, z-delta, x+delta, y+delta, z-delta, 0b00000100};
    triangle_3d_mem tr8 = {x, y, z-delta, x, y+delta, z-delta, x+delta, y+delta, z-delta, 0b00000100};
    *addr = tr7;
    addr++;
    *addr = tr8;
    addr++;

    //bottom
    triangle_3d_mem tr9 = {x, y, z, x+delta, y, z, x+delta, y, z-delta, 0b00000101};
    triangle_3d_mem tr10 = {x, y, z, x, y, z-delta, x+delta, y, z-delta, 0b00000101};
    *addr = tr9;
    addr++;
    *addr = tr10;
    addr++;

    //top
    triangle_3d_mem tr11 = {x, y+delta, z, x+delta, y+delta, z, x+delta, y+delta, z-delta, 0b00000110};
    triangle_3d_mem tr12 = {x, y+delta, z, x, y+delta, z-delta, x+delta, y+delta, z-delta, 0b00000110};
    *addr = tr11;
    addr++;
    *addr = tr12;
    addr++;

    triangle_buffer_count += 12;
}

void AP_drawing(){
    int my_APIC_id = getAPIC_ID();
    int* BSP_count = (int*)(AP_buffer_begin+(my_APIC_id-1)*0x20000);
    int* my_count = (int*)(AP_buffer_begin+(my_APIC_id-1)*0x20000+4);
    char to_print[10];
    AP_package* my_triangle_buffer = (AP_package*)(AP_buffer_begin+(my_APIC_id-1)*0x20000+0x1000);
    int current_count = 0;
    AP_package* current_triangle = my_triangle_buffer;
    while(1){
        if(*BSP_count > current_count){
            /*if(current_count==0 && my_APIC_id==1){
                int_to_ascii((int)my_count, to_print);
                printk(to_print);
                printk("\n");
            }*/
            drawTriangle3D(current_triangle->tr, current_triangle->normal_vec, current_triangle->color);
            /*if(current_count==0 && my_APIC_id==1){
                int_to_ascii((int)my_count, to_print);
                printk(to_print);
                printk("\n");
            }*/
            current_triangle++;
            current_count++;
            /*__asm__ __volatile__(
                ".intel_syntax noprefix;"
                "push eax;"
                ".att_syntax;"
            );
            __asm__ __volatile__(
                ".intel_syntax noprefix;"
                "lock and dword ptr [eax], 0;"
                ".att_syntax;"
            : "=a"(my_count) : );
            __asm__ __volatile__(
                ".intel_syntax noprefix;"
                "pop eax;"
                ".att_syntax;"
            );*/
            *my_count += 1;
            /*if(current_count==1 && my_APIC_id==1){
                int_to_ascii(*my_count, to_print);
                printk(to_print);
                printk("\n");
                int_to_ascii((int)my_count, to_print);
                printk(to_print);
                printk("\n");
                while(1);
            }
            else{
                while(1);
            }*/
        }
        else if(*BSP_count < current_count){
            current_triangle = my_triangle_buffer;
            current_count = 0;
            /*__asm__ __volatile__(
                ".intel_syntax noprefix;"
                "push eax;"
                ".att_syntax;"
            );
            __asm__ __volatile__(
                ".intel_syntax noprefix;"
                "lock and dword ptr [eax], 0;"
                ".att_syntax;"
            : "=a"(my_count) : );
             __asm__ __volatile__(
                ".intel_syntax noprefix;"
                "pop eax;"
                ".att_syntax;"
            );*/
            *my_count = 0;
        }
    }
}