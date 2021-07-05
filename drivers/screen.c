#include "screen.h"
#include "core_startup.h"


//private functions
int valid_color(unsigned char color);
int valid_plane(unsigned char plane);
int valid_planar_coord_3D(int x, int y);
int valid_coord(int x, int y);
void scroll_up();
void project_and_normal(triangle_3d_mem* triangle_addr, triangle_3d *new_tr, float_vector_3d *normal_vec);
void view_and_project(float_vector_3d* point, int_vector_3d* projected);
void displayTriangle3D(triangle_3d* tr, float_vector_3d* normal_vec, unsigned char color);
int calculate_depth(triangle_3d* triangle, float_vector_3d* normal_vec, int x_coord, int y_coord);
void increment_BSP_counter(int i);
void empty_BSP_counter(int i);

void init_screen(){
	//setup write and read mode (both 0)
	//http://www.retroarchive.org/swag/EGAVGA/0222.PAS.html
	//https://web.stanford.edu/class/cs140/projects/pintos/specs/freevga/vga/vgamem.htm
	unsigned short port = 0x3ce;
	unsigned char result;
	port_byte_out(port, 0x05);
	port += 1;
	result = port_byte_in(port);
	result &= 0b11110100;
	port_byte_out(port, result);

	//set current_char to zero
	current_char = 0;
}

void set_write_color(unsigned char color){
	/*VGA standard 4-bit color pallete can be found here: https://www.fountainware.com/EXPL/vga_color_palettes.htm
	 * instructions on using port 0x3c4 can be found here: https://wiki.osdev.org/VGA_Hardware#Port_0x3C4.2C_0x3CE.2C_0x3D4*/
	if(!valid_color(color)) return; //only 4-bit colors allowed  
	unsigned short port = 0x3c4;
	unsigned char result;
	port_byte_out(port, 0x02);
	port += 1;
	result = port_byte_in(port);
	result |= color;
	color |= 0b11110000;
	result &= color;
	port_byte_out(port, result);
}

void set_read_color(unsigned char plane){
	/*VGA reading planes: http://www.retroarchive.org/swag/EGAVGA/0222.PAS.html
	and: https://web.stanford.edu/class/cs140/projects/pintos/specs/freevga/vga/graphreg.htm#04*/
	if(!valid_plane(plane)) return;
	unsigned short port = 0x3ce;
	unsigned char result;
	port = 0x3ce;
	port_byte_out(port, 0x04);
	port += 1;
	result = port_byte_in(port);
	result |= plane;
	plane |= 0b11111100;
	result &= plane;
	port_byte_out(port, result);
}


void drawLine(int x0, int y0, int x1, int y1, char* draw_address){
	if(!(valid_coord(x0, y0) && valid_coord(x1, y1))) return;
	//variant of the efficient line drawing algoritm found at https://github.com/ssloy/tinyrenderer/wiki/Lesson-1:-Bresenham%E2%80%99s-Line-Drawing-Algorithm
	unsigned char buffer = 0b00000000;
	unsigned char buffer_adder = 0b10000000;
	//unlike the line drawing algorithm found at the link, this one is not completely symmetrical when swapping x and y
	if(ABS(x0-x1) < ABS(y0-y1)){
		if(y0 > y1){
			SWAP_INT(x0, x1);
			SWAP_INT(y0, y1);
		}

		int real_x = x0/8;
		buffer_adder = buffer_adder >> (x0-real_x*8);

		int dx = x1-x0; 
		int dy = y1-y0;
		int derror = ABS(dx)*2;
		int error = 0; 

		draw_address += 80*(479-y0)+real_x;

		for(int y=y0; y<y1; y++){
			buffer |= buffer_adder;
			error += derror;
			*draw_address |= buffer;
			if(error > dy){
				buffer = 0b00000000;
				buffer_adder = (x1>x0) ? buffer_adder >> 1 : buffer_adder << 1;
				error -= dy*2;
			}
			if(buffer_adder == 0b00000000){
				draw_address += (x1>x0) ? 1 : -1;
				buffer_adder = (x1>x0) ? 0b10000000 : 0b00000001;
			}
			draw_address -= 80;
		}
	}
	else{
		if(x0 > x1){
			SWAP_INT(x0, x1);
			SWAP_INT(y0, y1);
		}

		int real_x = x0/8;
		buffer_adder = buffer_adder >> (x0-real_x*8);

		int dx = x1-x0; 
		int dy = y1-y0;
		int derror = ABS(dy)*2;
		int error = 0; 

		draw_address += 80*(479-y0)+real_x;

		int already_written = 0;

		for(int x=x0; x<x1; x++){
			buffer |= buffer_adder;
			buffer_adder = buffer_adder >> 1;
			error += derror;
			if(error > dx){
				*draw_address |= buffer;
				buffer = 0b00000000;
				already_written = 1;
				draw_address += (y1>y0) ? -80 : 80;
				error -= dx*2;
			}
			if(buffer_adder == 0b00000000){
				if(already_written==0){
					*draw_address |= buffer;
					buffer = 0b00000000;
				}
				draw_address += 1;
				buffer_adder = 0b10000000;
			}
			already_written = 0;
		}
	}
}

//TODO serious optimizations are needed here lol
void drawTriangle3D(triangle_3d triangle, float_vector_3d normal_vec, unsigned char color){
	//the x and y coords from this triangle are assumed to be like normal x and y coords
	//z coords is interpreted as a depth and is only allowed to be positive

	//intializing variables
	//,alot of variables
	//!!!!! this is not clean at all -> should be fixed !!!!!
	unsigned short* z_buffer_begin;
	unsigned char* mem_pointer_1;
	unsigned char* mem_pointer_2;
	unsigned char* mem_pointer_3;
	unsigned char* mem_pointer_4;
	int begin_byte;
	unsigned char cycling_byte;
	unsigned char writing_byte;
	int second_half;
	int segment_height;
	float alpha;
	float beta;
	int vec1_x;
	int vec1_y;
	int vec2_x;
	int vec2_y;
	int x_coord;
	int y_coord;
	int z_coord;
	float dot_1;
	float dot_2;
	//make it so that y2 >= y1 >= y0
	int_vector_3d temp_local_vect;
	if (triangle.p0.y>triangle.p1.y){
		temp_local_vect = triangle.p0;
		triangle.p0 = triangle.p1;
		triangle.p1 = temp_local_vect;
	} 
    if (triangle.p0.y>triangle.p2.y){
		temp_local_vect = triangle.p0;
		triangle.p0 = triangle.p2;
		triangle.p2 = temp_local_vect;
	} 
    if (triangle.p1.y>triangle.p2.y){
		temp_local_vect = triangle.p1;
		triangle.p1 = triangle.p2;
		triangle.p2 = temp_local_vect;
	}
	int total_height = triangle.p2.y-triangle.p0.y;
	float p_factor = (float)triangle.p0.z/fast_hypo_3d((float)triangle.p0.x, (float)triangle.p0.y, DIST_TO_SCREEN);
	for (int i=0; i<total_height; i++) {
		y_coord = i+triangle.p0.y;
		if(y_coord >= 240 || y_coord < -240) continue;
		second_half = i>(triangle.p1.y-triangle.p0.y) || triangle.p1.y==triangle.p0.y;  
        segment_height = (int)(second_half ? (triangle.p2.y-triangle.p1.y) : (triangle.p1.y-triangle.p0.y)); 
        alpha = (float)i/total_height;
        beta  = (float)(i-(second_half ? (triangle.p1.y-triangle.p0.y) : 0))/segment_height; // be careful with divisions by zero
		vec1_x = (int)(triangle.p0.x+(triangle.p2.x-triangle.p0.x)*alpha);
		vec1_y = (int)(triangle.p0.y+(triangle.p2.y-triangle.p0.y)*alpha);
		vec2_x = second_half ? (int)(triangle.p1.x+(triangle.p2.x-triangle.p1.x)*beta) : (int)(triangle.p0.x+(triangle.p1.x-triangle.p0.x)*beta);
		vec2_y = second_half ? (int)(triangle.p1.y+(triangle.p2.y-triangle.p1.y)*beta) : (int)(triangle.p0.y+(triangle.p1.y-triangle.p0.y)*beta);
		//vec2_x >= vec1_x
		int temp_local_int;
		if(vec1_x > vec2_x){
			temp_local_int = vec1_x;
			vec1_x = vec2_x;
			vec2_x = temp_local_int;
			temp_local_int = vec1_y;
			vec1_y = vec2_y;
			vec2_y = temp_local_int;
		}
		begin_byte = ((vec1_x+320) >> 3);
		writing_byte = 0b00000000;
		cycling_byte = 0b10000000 >> ((vec1_x+320) % 8);
		z_buffer_begin = (unsigned short*)z_buffer_addr + (vec1_x+320) + 640*(479-i-triangle.p0.y-240);
		mem_pointer_1 = (unsigned char*)buffer_addr[0] + begin_byte + 80*(479-i-triangle.p0.y-240);
		mem_pointer_2 = (unsigned char*)buffer_addr[1] + begin_byte + 80*(479-i-triangle.p0.y-240);
		mem_pointer_3 = (unsigned char*)buffer_addr[2] + begin_byte + 80*(479-i-triangle.p0.y-240);
		mem_pointer_4 = (unsigned char*)buffer_addr[3] + begin_byte + 80*(479-i-triangle.p0.y-240);
		for(int j=0; j<=(vec2_x-vec1_x); j++, z_buffer_begin++){
			x_coord = vec1_x + j;
			//use vector calculus to find the z_distance from the camera to triangle.p0
			dot_1 = x_coord*normal_vec.x + y_coord*normal_vec.y - DIST_TO_SCREEN*normal_vec.z;
			dot_2 = p_factor*(triangle.p0.x*normal_vec.x + triangle.p0.y*normal_vec.y - DIST_TO_SCREEN*normal_vec.z);
			if(dot_1!=0){
				z_coord = (int)(DIST_TO_SCREEN*dot_2/dot_1);
			}
			else{
				z_coord = 0xffff;
			}
			if((int)(*z_buffer_begin) > z_coord){
				writing_byte |= cycling_byte;
				int original = (*z_buffer_begin);
				(*z_buffer_begin) = (unsigned short)(z_coord);
			}
			cycling_byte >>= 1;
			if(cycling_byte == 0b00000000){
				*mem_pointer_1 = ((color & 0b00000001) == 0) ?  *mem_pointer_1 & (0b11111111 - writing_byte) : *mem_pointer_1 | writing_byte;
				*mem_pointer_2 = ((color & 0b00000010) == 0) ?  *mem_pointer_2 & (0b11111111 - writing_byte) : *mem_pointer_2 | writing_byte;
				*mem_pointer_3 = ((color & 0b00000100) == 0) ?  *mem_pointer_3 & (0b11111111 - writing_byte) : *mem_pointer_3 | writing_byte;
				*mem_pointer_4 = ((color & 0b00001000) == 0) ?  *mem_pointer_4 & (0b11111111 - writing_byte) : *mem_pointer_4 | writing_byte;
				cycling_byte = 0b10000000;
				writing_byte = 0b00000000;
				mem_pointer_1++; mem_pointer_2++; mem_pointer_3++; mem_pointer_4++;
			}
			*mem_pointer_1 = ((color & 0b00000001) == 0) ?  *mem_pointer_1 & (0b11111111 - writing_byte) : *mem_pointer_1 | writing_byte;
			*mem_pointer_2 = ((color & 0b00000010) == 0) ?  *mem_pointer_2 & (0b11111111 - writing_byte) : *mem_pointer_2 | writing_byte;
			*mem_pointer_3 = ((color & 0b00000100) == 0) ?  *mem_pointer_3 & (0b11111111 - writing_byte) : *mem_pointer_3 | writing_byte;
			*mem_pointer_4 = ((color & 0b00001000) == 0) ?  *mem_pointer_4 & (0b11111111 - writing_byte) : *mem_pointer_4 | writing_byte;
		}
    } 
}

void drawTriangle2D(triangle_2d triangle, char* draw_address){
	if(!(valid_coord(triangle.p0.x, triangle.p0.y) && valid_coord(triangle.p1.x, triangle.p1.y) && valid_coord(triangle.p2.x, triangle.p2.y))) return;
	//variant of the algoritm found here: https://github.com/ssloy/tinyrenderer/wiki/Lesson-2:-Triangle-rasterization-and-back-face-culling
	
	//intializing variables
	//,alot of variables
	triangle.p0.y = 479-triangle.p0.y; triangle.p1.y = 479-triangle.p1.y; triangle.p2.y = 479-triangle.p2.y;
	char* mem_pointer;
	int begin_bit;
	int begin_byte;
	int starting_byte;
	int final_byte;
	int end_bit;
	int end_byte;
	int second_half;
	int segment_height;
	float alpha;
	float beta;
	int vec1_x;
	int vec1_y;
	int vec2_x;
	int vec2_y;

	//make it so that y2 >= y1 >= y0
	if (triangle.p0.y>triangle.p1.y){
		SWAP_VECT_2D(triangle.p0, triangle.p1);
	} 
    if (triangle.p0.y>triangle.p2.y){
		SWAP_VECT_2D(triangle.p0, triangle.p2);

	} 
    if (triangle.p1.y>triangle.p2.y){
		SWAP_VECT_2D(triangle.p1, triangle.p2);
	}
	int total_height = triangle.p2.y-triangle.p0.y; 
	for (int i=0; i<total_height; i++) {
		second_half = i>(triangle.p1.y-triangle.p0.y) || triangle.p1.y==triangle.p0.y;  
        segment_height = (int)(second_half ? (triangle.p2.y-triangle.p1.y) : (triangle.p1.y-triangle.p0.y)); 
        alpha = (float)i/total_height; 
        beta  = (float)(i-(second_half ? (triangle.p1.y-triangle.p0.y) : 0))/segment_height; // be careful with divisions by zero
		vec1_x = (int)(triangle.p0.x+(triangle.p2.x-triangle.p0.x)*alpha);
		vec1_y = (int)(triangle.p0.y+(triangle.p2.y-triangle.p0.y)*alpha);
		vec2_x = second_half ? (int)(triangle.p1.x+(triangle.p2.x-triangle.p1.x)*beta) : (int)(triangle.p0.x+(triangle.p1.x-triangle.p0.x)*beta);
		vec2_y = second_half ? (int)(triangle.p1.y+(triangle.p2.y-triangle.p1.y)*beta) : (int)(triangle.p0.y+(triangle.p1.y-triangle.p0.y)*beta);
		//vec2_x >= vec1_x
		if(vec1_x > vec2_x){
			SWAP_INT(vec1_x, vec2_x);
			SWAP_INT(vec1_y, vec2_y);
		}
		begin_bit = (vec1_x % 8);
		begin_byte = (vec1_x >> 3);
		end_bit = (vec2_x % 8);
		end_byte = (vec2_x >> 3);
		starting_byte = 0b11111111 >> begin_bit;
		final_byte = 0b11111111 - (0b01111111 >> end_bit);
		mem_pointer = draw_address + begin_byte + 80*(i+triangle.p0.y);
		if(begin_byte==end_byte){
			starting_byte &= final_byte;
			*mem_pointer |= starting_byte;
		}
		else{
			*mem_pointer |= starting_byte;
			begin_byte++;
			mem_pointer++;
			fill_bytes_asm((int)mem_pointer, (end_byte-begin_byte));
			mem_pointer += (end_byte-begin_byte);
			*mem_pointer |= final_byte;
		}
    } 
}


//TODO: make use of the already existing print_string_at function
void printk(char string[]){
	set_write_color(0b00000111);
	int i = 0;
	unsigned char draw_byte;
	char* font_addr;
	char* video_mem;
	for(int i=0; i<strlen(string); i++){
		if(string[i]=='\n'){
			current_char += (80 - (current_char % 80));
			current_char += 15*80;    //a character is 16 pixels high
		}
		else{
			video_mem = (char*)(VIDEO_MEMORY+current_char);
			font_addr = (char*)(FONT_TABLE + ((int)string[i] << 4));   //every character takes in 16 bytes
			for(int j=0; j<16; j++){
				*video_mem = *font_addr;
				video_mem += 80;
				font_addr += 1;
			}
			current_char++;
			if((current_char % 80)==0) current_char += 15*80;     //a character is 16 pixels high
		}
		if(current_char==480*80){     //we are at the end of the screen
			scroll_up();
			current_char -= 80*16;
		}
	};
}

void print_string_at(char string[], char* buffer_addr){
	char* font_addr;
	char* mem_addr;
	for(int i=0; i<strlen(string); i++){
		font_addr = (char*)(FONT_TABLE + ((int)string[i] << 4));  //every character takes in 16 bytes
		mem_addr = buffer_addr + i;
		for(int j=0; j<16; j++){
			*mem_addr = *font_addr;
			mem_addr += 80;
			font_addr += 1;
		}
	}
	
}

void print_int_at(int to_print, char* buffer_addr){
	char int_str[20];
	int_to_ascii(to_print, int_str);
	set_write_color(0b00000111);
    print_string_at(int_str, buffer_addr);
}

//lazy implementation but who cares
void printk_backspace(){
	if(current_char>0){
		int line_return = ((current_char % 80)==0);
		current_char -= 1+line_return*15*80;
		printk(" ");
		current_char -= 1+line_return*15*80;
	}
}

void scroll_up(){
	//since we assume only kernel text to be present on the screen and gray<->0111 it suffices to read plane 0
	set_read_color(0);
	set_write_color(0b00000111);
	
	//use assembly functions since using C appeared to be to inefficiënt
	//move everything down
	move_dwords_asm((int)(VIDEO_MEMORY+0x500), (int)VIDEO_MEMORY, (int)(464*80/4));
	//clear last line
	clear_dwords_asm((int)(VIDEO_MEMORY+0x9100), (int)(16*80/4));
}

void clear_screen(){
	set_write_color(0b00001111);
	clear_dwords_asm((int)VIDEO_MEMORY, 480*80/4);
	current_char = 0;
}

void initialize_buffers(){
	/* Reserve 4 buffers starting at address 0x20000
	   every buffer will have a size of 480*80 bytes = 0x9600 bytes
	   thus in total 4*0x9600 bytes = 0x25800 bytes
	   these buffers will thus take space from address 0x20000 until address 0x45800
	*/
	clear_dwords_asm(0x20000, (int)(4*480*80/4));
	//initialize all buffer addresses
	buffer_addr[0] = 0x20000;
	buffer_addr[1] = 0x29600;
	buffer_addr[2] = 0x32C00;
	buffer_addr[3] = 0x3C200;

	/* Reserve 1 buffers starting at address 0x45800
	   of size 0x3000 bytes, this buffer will thus take space from address 0x45800 until 0x48800
	   this buffer allows for 646 triangles (triangle_3d_mem)
	*/
	clear_dwords_asm(0x45800, (int)(0x3000/4));
	triangle_buffer_addr = (int)0x45800;
	//the buffer has at the moment zero triangles;
	triangle_buffer_count = 0;

	/*
	z_buffer an array starting at addres 0x100000 with 640*480 shorts <-> 0x96000 bytes <-> from 0x100000 to 0x196000
	*/
	fill_dwords_asm(0x100000, (int)(0x96000/4));
	z_buffer_addr = 0x100000;

	/*
	seperate triangle buffer for each core except the BSP, these buffers are placed several bytes after the stacks of every core
	every buffer is 0x20000 bytes big <-> (num_cores-1)*0x20000 bytes in total
	the first 1000 bytes for every core can be used for information passing between cores
	the other 01F000 bytes can be used for 2089 triangles (AP_package)
	*/
	AP_buffer_begin = 0x200000+0x10000*num_cores+0x10000;
	clear_dwords_asm(AP_buffer_begin, (int)((num_cores-1)*(0x20000/4)));
}

void swap_buffers(int i){
	//swap the i'th buffer (write this buffer to the screen)
	unsigned char color = 1 << i;
	set_write_color(color);
	move_dwords_asm(buffer_addr[i], 0xA0000, (int)(480*80/4));
}

void display_triangle_buffer(){
	fill_dwords_asm(z_buffer_addr, (int)(0x96000/4));
	triangle_3d_mem* addr = (triangle_3d_mem*)triangle_buffer_addr;
	triangle_3d new_tr;
	float_vector_3d normal_vec;
	int num_of_AP_cores = num_cores-1;
	for(int i=0; i<triangle_buffer_count; i++, addr++){
		project_and_normal(addr, &new_tr, &normal_vec);
		displayTriangle3D(&new_tr, &normal_vec, addr->color);
	}
	int done;
	do{
		done = 1;
		for(int i=0; i<num_of_AP_cores; i++){
			int* BSP_count = (int*)(AP_buffer_begin+i*0x20000);
    		int* AP_count = (int*)(AP_buffer_begin+i*0x20000+4);
			if(*BSP_count!=*AP_count){
				done = 0;
				break;
			}
		}
	}while(done==0);
	for(int i=0; i<num_of_AP_cores; i++){
		empty_BSP_counter(i);
	}
}


//divide the triangles in groups that do not overlap, 
//this way every other core can get a group of triangles and draw the triangles
//autonomically without worrying about locks or synchronisation
void displayTriangle3D(triangle_3d* tr, float_vector_3d* normal_vec, unsigned char color){
	int num_of_AP_cores = num_cores-1;
	int length_per_core = 640/num_of_AP_cores;

	//dont waste time calculating triangles that are behind you
	if(tr->p0.z == -1 || tr->p1.z == -1 || tr->p2.z == -1) return;

	//dont waste time calculating triangles that are offscreen
	if(!valid_coord(tr->p0.x+320, tr->p0.y+240) && !valid_coord(tr->p1.x+320, tr->p1.y+240) && !valid_coord(tr->p2.x+320, tr->p2.y+240)) return; 

	//dont waste time calculating lines
	if(tr->p0.x == tr->p1.x && tr->p0.y == tr->p1.y) return;
	if(tr->p0.x == tr->p2.x && tr->p0.y == tr->p2.y) return;
	if(tr->p1.x == tr->p2.x && tr->p1.y == tr->p2.y) return;

	//make it so that x2 >= x1 >= x0
	if(tr->p0.x > tr->p1.x){SWAP_VECT_3D(tr->p0, tr->p1);}
    if(tr->p0.x > tr->p2.x){SWAP_VECT_3D(tr->p0, tr->p2);}
    if(tr->p1.x > tr->p2.x){SWAP_VECT_3D(tr->p1, tr->p2);}
	for(int i=0; i<num_of_AP_cores; i++){	
		AP_package* current_AP_triangle_buffer = (AP_package*)(AP_buffer_begin+i*0x20000+0x1000);
		current_AP_triangle_buffer += *((int*)(AP_buffer_begin+i*0x20000));
		int x0 = -320+length_per_core*i;
        int x1 = (i==(num_of_AP_cores-1)) ? 320 : -320+length_per_core*(i+1);
		if(x0 > tr->p2.x || x1 <= tr->p0.x) continue;
        else if(x0 <= tr->p0.x && x1 > tr->p2.x){
			*current_AP_triangle_buffer = (AP_package){*tr, *normal_vec, color};
            increment_BSP_counter(i);
        }
        else if(x0 <= tr->p0.x && x0 <= tr->p1.x && x1 > tr->p1.x){
			x1--;
            int b1 = tr->p1.y+((x1-tr->p1.x)*(tr->p2.y-tr->p1.y))/(tr->p2.x-tr->p1.x);
            int b0 = tr->p0.y+((x1-tr->p0.x)*(tr->p2.y-tr->p0.y))/(tr->p2.x-tr->p0.x);
            *current_AP_triangle_buffer = (AP_package){{{tr->p0.x, tr->p0.y, calculate_depth(tr, normal_vec, tr->p0.x, tr->p0.y)}, {tr->p1.x, tr->p1.y, calculate_depth(tr, normal_vec, tr->p1.x, tr->p1.y)}, {x1, b1, calculate_depth(tr, normal_vec, x1, b1)}}, *normal_vec, color};
            increment_BSP_counter(i);
			current_AP_triangle_buffer++;
            *current_AP_triangle_buffer = (AP_package){{{tr->p0.x, tr->p0.y, calculate_depth(tr, normal_vec, tr->p0.x, tr->p0.y)}, {x1, b0, calculate_depth(tr, normal_vec, x1, b0)}, {x1, b1, calculate_depth(tr, normal_vec, x1, b1)}}, *normal_vec, color};
            increment_BSP_counter(i);
        }
        else if(x0 <= tr->p0.x){
			x1--;
            int b1 = tr->p0.y+((x1-tr->p0.x)*(tr->p1.y-tr->p0.y))/(tr->p1.x-tr->p0.x);
            int b0 = tr->p0.y+((x1-tr->p0.x)*(tr->p2.y-tr->p0.y))/(tr->p2.x-tr->p0.x);
            *current_AP_triangle_buffer = (AP_package){{{tr->p0.x, tr->p0.y, calculate_depth(tr, normal_vec, tr->p0.x, tr->p0.y)}, {x1, b0, calculate_depth(tr, normal_vec, x1, b0)}, {x1, b1, calculate_depth(tr, normal_vec, x1, b1)}}, *normal_vec, color};
            increment_BSP_counter(i);
        }
        else if(x1 > tr->p2.x && x0 <= tr->p1.x && x1 > tr->p1.x){
			x1--;
            int b1 = tr->p1.y+((x0-tr->p1.x)*(tr->p1.y-tr->p0.y))/(tr->p1.x-tr->p0.x);
            int b2 = tr->p2.y+((x0-tr->p2.x)*(tr->p2.y-tr->p0.y))/(tr->p2.x-tr->p0.x);
            *current_AP_triangle_buffer = (AP_package){{{x0, b1, calculate_depth(tr, normal_vec, x0, b1)}, {tr->p1.x, tr->p1.y, calculate_depth(tr, normal_vec, tr->p1.x, tr->p1.y)}, {tr->p2.x, tr->p2.y, calculate_depth(tr, normal_vec, tr->p2.x, tr->p2.y)}}, *normal_vec, color};
            increment_BSP_counter(i);
			current_AP_triangle_buffer++;
            *current_AP_triangle_buffer = (AP_package){{{x0, b1, calculate_depth(tr, normal_vec, x0, b1)}, {x0, b2, calculate_depth(tr, normal_vec, x0, b2)}, {tr->p2.x, tr->p2.y, calculate_depth(tr, normal_vec, tr->p2.x, tr->p2.y)}}, *normal_vec, color};
            increment_BSP_counter(i);
        }
        else if(x1 > tr->p2.x){
			x1--;
            int b1 = tr->p2.y+((x0-tr->p2.x)*(tr->p2.y-tr->p0.y))/(tr->p2.x-tr->p0.x);
            int b2 = tr->p2.y+((x0-tr->p2.x)*(tr->p2.y-tr->p1.y))/(tr->p2.x-tr->p1.x);
            *current_AP_triangle_buffer = (AP_package){{{x0, b1, calculate_depth(tr, normal_vec, x0, b1)}, {x0, b2, calculate_depth(tr, normal_vec, x0, b2)}, {tr->p2.x, tr->p2.y, calculate_depth(tr, normal_vec, tr->p2.x, tr->p2.y)}}, *normal_vec, color};
            increment_BSP_counter(i);
        }
        else if(x0 <= tr->p1.x && x1 > tr->p1.x){
			x1--;
            int b0 = tr->p1.y+((x0-tr->p1.x)*(tr->p1.y-tr->p0.y))/(tr->p1.x-tr->p0.x);
            int b1 = tr->p0.y+((x0-tr->p0.x)*(tr->p2.y-tr->p0.y))/(tr->p2.x-tr->p0.x);
            int b2 = tr->p1.y+((x1-tr->p1.x)*(tr->p2.y-tr->p1.y))/(tr->p2.x-tr->p1.x);
            int b3 = tr->p0.y+((x1-tr->p0.x)*(tr->p2.y-tr->p0.y))/(tr->p2.x-tr->p0.x);
            *current_AP_triangle_buffer = (AP_package){{{x0, b0, calculate_depth(tr, normal_vec, x0, b0)}, {x1, b2, calculate_depth(tr, normal_vec, x1, b2)}, {x1, b3, calculate_depth(tr, normal_vec, x1, b3)}}, *normal_vec, color};
            increment_BSP_counter(i);
			current_AP_triangle_buffer++;
            *current_AP_triangle_buffer = (AP_package){{{x0, b0, calculate_depth(tr, normal_vec, x0, b0)}, {x0, b1, calculate_depth(tr, normal_vec, x0, b1)}, {x1, b3, calculate_depth(tr, normal_vec, x1, b3)}}, *normal_vec, color};
            increment_BSP_counter(i);
			current_AP_triangle_buffer++; 
            *current_AP_triangle_buffer = (AP_package){{{x0, b0, calculate_depth(tr, normal_vec, x0, b0)}, {tr->p1.x, tr->p1.y, calculate_depth(tr, normal_vec, tr->p1.x, tr->p1.y)}, {x1, b2, calculate_depth(tr, normal_vec, x1, b2)}}, *normal_vec, color};
            increment_BSP_counter(i);    
        }
        else if(tr->p1.x >= x1){
			x1--;
            int b0 = tr->p0.y+((x0-tr->p0.x)*(tr->p1.y-tr->p0.y))/(tr->p1.x-tr->p0.x);
            int b1 = tr->p0.y+((x0-tr->p0.x)*(tr->p2.y-tr->p0.y))/(tr->p2.x-tr->p0.x);
            int b2 = tr->p0.y+((x1-tr->p0.x)*(tr->p1.y-tr->p0.y))/(tr->p1.x-tr->p0.x);
            int b3 =  tr->p0.y+((x1-tr->p0.x)*(tr->p2.y-tr->p0.y))/(tr->p2.x-tr->p0.x);
            *current_AP_triangle_buffer = (AP_package){{{x0, b0, calculate_depth(tr, normal_vec, x0, b0)}, {x0, b1, calculate_depth(tr, normal_vec, x0, b1)}, {x1, b3, calculate_depth(tr, normal_vec, x1, b3)}}, *normal_vec, color};
            increment_BSP_counter(i);
			current_AP_triangle_buffer++;
            *current_AP_triangle_buffer = (AP_package){{{x0, b0, calculate_depth(tr, normal_vec, x0, b0)}, {x1, b2, calculate_depth(tr, normal_vec, x1, b2)}, {x1, b3, calculate_depth(tr, normal_vec, x1, b3)}}, *normal_vec, color};
            increment_BSP_counter(i);
        }
        else{
			x1--;
            int b0 = tr->p1.y+((x0-tr->p1.x)*(tr->p2.y-tr->p1.y))/(tr->p2.x-tr->p1.x);
            int b1 = tr->p0.y+((x0-tr->p0.x)*(tr->p2.y-tr->p0.y))/(tr->p2.x-tr->p0.x);
            int b2 = tr->p1.y+((x1-tr->p1.x)*(tr->p2.y-tr->p1.y))/(tr->p2.x-tr->p1.x);
            int b3 = tr->p0.y+((x1-tr->p0.x)*(tr->p2.y-tr->p0.y))/(tr->p2.x-tr->p0.x);
            *current_AP_triangle_buffer = (AP_package){{{x0, b0, calculate_depth(tr, normal_vec, x0, b0)}, {x0, b1, calculate_depth(tr, normal_vec, x0, b1)}, {x1, b3, calculate_depth(tr, normal_vec, x1, b3)}}, *normal_vec, color};
            increment_BSP_counter(i);
			current_AP_triangle_buffer++;
            *current_AP_triangle_buffer = (AP_package){{{x0, b0, calculate_depth(tr, normal_vec, x0, b0)}, {x1, b2, calculate_depth(tr, normal_vec, x1, b2)}, {x1, b3, calculate_depth(tr, normal_vec, x1, b3)}}, *normal_vec, color};
            increment_BSP_counter(i);
        }
    }
}

int valid_color(unsigned char color){return ((color & 0b11110000)==0);}
int valid_plane(unsigned char plane){return (plane<4);}
int valid_coord(int x, int y){return (x>=0 && y>=0 && x<640 && y<480);}
int valid_planar_coord_3D(int x, int y){return (x>=-320 && y>=-240 && x<320 && y<240);}

void project_and_normal(triangle_3d_mem* triangle_addr, triangle_3d *new_tr, float_vector_3d *normal_vec){
	triangle_3d_mem triangle = *triangle_addr;
	float_vector_3d point_0 = {view_matrix.a00*triangle.x0+view_matrix.a01*triangle.y0+view_matrix.a02*triangle.z0+view_matrix.a03, view_matrix.a10*triangle.x0+view_matrix.a11*triangle.y0+view_matrix.a12*triangle.z0+view_matrix.a13, view_matrix.a20*triangle.x0+view_matrix.a21*triangle.y0+view_matrix.a22*triangle.z0+view_matrix.a23};
	float_vector_3d point_1 = {view_matrix.a00*triangle.x1+view_matrix.a01*triangle.y1+view_matrix.a02*triangle.z1+view_matrix.a03, view_matrix.a10*triangle.x1+view_matrix.a11*triangle.y1+view_matrix.a12*triangle.z1+view_matrix.a13, view_matrix.a20*triangle.x1+view_matrix.a21*triangle.y1+view_matrix.a22*triangle.z1+view_matrix.a23};
	float_vector_3d point_2 = {view_matrix.a00*triangle.x2+view_matrix.a01*triangle.y2+view_matrix.a02*triangle.z2+view_matrix.a03, view_matrix.a10*triangle.x2+view_matrix.a11*triangle.y2+view_matrix.a12*triangle.z2+view_matrix.a13, view_matrix.a20*triangle.x2+view_matrix.a21*triangle.y2+view_matrix.a22*triangle.z2+view_matrix.a23};
	
	float normal_x = (point_2.y-point_0.y)*(point_1.z-point_0.z)-(point_2.z-point_0.z)*(point_1.y-point_0.y);
	float normal_y = -(point_2.x-point_0.x)*(point_1.z-point_0.z)+(point_2.z-point_0.z)*(point_1.x-point_0.x);
	float normal_z = (point_2.x-point_0.x)*(point_1.y-point_0.y)-(point_2.y-point_0.y)*(point_1.x-point_0.x);
	float normalization_factor = fast_hypo_3d(normal_x, normal_y, normal_z);

	view_and_project(&point_0, &new_tr->p0);
	view_and_project(&point_1, &new_tr->p1);
	view_and_project(&point_2, &new_tr->p2);

	normal_vec->x = normal_x/normalization_factor;
	normal_vec->y = normal_y/normalization_factor;
	normal_vec->z = normal_z/normalization_factor;
}

void view_and_project(float_vector_3d* point, int_vector_3d* projected){
	projected->x = (int)(point->x/(1-point->z/DIST_TO_SCREEN));
	projected->y = (int)(point->y/(1-point->z/DIST_TO_SCREEN));
	projected->z = (point->z < DIST_TO_SCREEN) ? (int)fast_hypo_3d(point->x, point->y, DIST_TO_SCREEN-point->z) : -1;
}

float fast_hypo_3d(float x, float y, float z){
	//make an initial guess with idea found at https://stackoverflow.com/questions/3506404/fast-hypotenuse-algorithm-for-embedded-processor
	//and further improve using newtons method
	
	//make everything positive
	if(x<0) x*=-1;
	if(y<0) y*=-1;
	if(z<0) z*=-1;

	//x>=y>=z
	float temp_local_float;
	if(y>x){
		temp_local_float = x;
		x = y;
		y = temp_local_float;
	}
	if(z>x){
		temp_local_float = x;
		x = z;
		z = temp_local_float;
	}
	if(z>y){
		temp_local_float = y;
		y = z;
		z = temp_local_float;
	}
	
	float hypo_squared = x*x+y*y+z*z;
	float approximation = 0.4142*z+y;
	approximation = (approximation>x) ? 0.4142*x+approximation : 0.4142*approximation+x;
	//use newtons method twice (even once is already pretty good most of the time)
	approximation = approximation/2.0 + hypo_squared/(2.0*approximation);
	approximation = approximation/2.0 + hypo_squared/(2.0*approximation);
	return approximation;
}

//not the most efficient method, using the method from "drawTriangle3D" which isn't necessarily optimal in this context
//but this should not be a bottleneck
int calculate_depth(triangle_3d* triangle, float_vector_3d* normal_vec, int x_coord, int y_coord){
	float p_factor = (float)triangle->p0.z/fast_hypo_3d((float)triangle->p0.x, (float)triangle->p0.y, DIST_TO_SCREEN);
	float dot_1 = x_coord*normal_vec->x + y_coord*normal_vec->y - DIST_TO_SCREEN*normal_vec->z;
	float dot_2 = p_factor*(triangle->p0.x*normal_vec->x + triangle->p0.y*normal_vec->y - DIST_TO_SCREEN*normal_vec->z);
	if(dot_1!=0){
		return (int)(fast_hypo_3d(x_coord, y_coord, DIST_TO_SCREEN)*(dot_2/dot_1));
	}
	else{
		return 0xffff;
	}
}

void increment_BSP_counter(int i){
	int* BPS_counter = (int*)(AP_buffer_begin+i*0x20000);
	/*__asm__ __volatile__(
		".intel_syntax noprefix;"
		"push eax;"
		".att_syntax;"
	);
	__asm__ __volatile__(
		".intel_syntax noprefix;"
		"lock inc dword ptr [eax];"
		".att_syntax;"
	: "=a"(BPS_counter) : );
	__asm__ __volatile__(
		".intel_syntax noprefix;"
		"pop eax;"
		".att_syntax;"
	);*/
	*BPS_counter += 1;
}

void empty_BSP_counter(int i){
	int* BPS_counter = (int*)(AP_buffer_begin+i*0x20000);
	/*__asm__ __volatile__(
		".intel_syntax noprefix;"
		"push eax;"
		".att_syntax;"
	);
	__asm__ __volatile__(
		".intel_syntax noprefix;"
		"lock and dword ptr [eax], 0;"
		".att_syntax;"
	: "=a"(BPS_counter) : );
		__asm__ __volatile__(
		".intel_syntax noprefix;"
		"pop eax;"
		".att_syntax;"
	);*/
	*BPS_counter = 0;
}

