#include "editor_program.h"

//private functions
void move_buffer(char* to_change, char prev_char);
void shrink_buffer(char* to_change);


void kernel_to_editor(){
    clear_screen();
    //buffer is 0x28CD0 bytes big, allowing for 2089 lines of ASCII char's
    clear_dwords_asm(0x1000000, 0xA334);
    set_write_color(0b00000111);
    put_on_screen((char*)BUFFER_BEGIN, 0);
    current_mem_addr = 0;
    current_char = *((char*)BUFFER_BEGIN);
    current_mem_position = 0;
    print_char_at('<', current_mem_position);
}

void editor_loop_callback(){
    __asm__ __volatile__(
        ".intel_syntax noprefix;"
        "hlt;"
        ".att_syntax;"
    );
}

//TODO: cleanup
void editor_keyboard_callback(unsigned char scancode){
    if(scancode == (unsigned char)75){   //leftkey
        if(current_mem_addr == 0) return;
        current_mem_addr--;
        char* cursor_char = (char*)BUFFER_BEGIN + current_mem_addr;
        if(current_mem_position==0){
            if(*cursor_char == '\n'){
                int counter = 1;
                while(((int)(cursor_char-counter) >= BUFFER_BEGIN) && *(cursor_char-counter)!='\n' && counter<80) counter++;
                counter--;
                scroll_down();
                //put_on_screen(cursor_char-counter, current_mem_position);
                current_mem_position += counter;
            }
            else{
                current_mem_position = 79;
                scroll_down();
            }
        }
        else if((current_mem_position % 80) == 0){
            if(*cursor_char == '\n'){
                int counter = 1;
                while(((int)(cursor_char-counter) >= BUFFER_BEGIN) && *(cursor_char-counter)!='\n' && counter<80) counter++;
                counter--;
                current_mem_position -= 16*80;
                current_mem_position += counter;
            }
            else{
                current_mem_position -= 15*80;
                current_mem_position--;
            }
        }
        else{
            current_mem_position--;
        }
    }
    else if(scancode == (unsigned char)77){   //rightkey
        char* cursor_char = (char*)BUFFER_BEGIN + current_mem_addr;
        if(*cursor_char == '\0') return;
    }
    else if(scancode > SC_MAX) return;
    else if(scancode == BACKSPACE){
        if(current_mem_addr == 0) return;
        current_mem_addr--;
        char* to_change = (char*)BUFFER_BEGIN + current_mem_addr;
        char prev_char = *to_change;
        shrink_buffer(to_change);
        if(*to_change == '\0'){
            if(current_mem_position==0){
                if(prev_char == '\n'){
                    int counter = 1;
                    while(((int)(to_change-counter) >= BUFFER_BEGIN) && *(to_change-counter)!='\n' && counter<80) counter++;
                    counter--;
                    for(int i=0; i<counter; i++){
                        print_char_at(*(to_change-counter+i), i);
                    }
                    print_char_at('<', counter);
                    current_mem_position += counter;
                }
                else{
                    for(int i=0; i<79; i++){
                        print_char_at(*(to_change-79+i), i);
                    }
                    print_char_at(79, '<');
                    current_mem_position = 79;
                }
            }
            else if((current_mem_position % 80) == 0){
                if(prev_char == '\n'){
                    int counter = 1;
                    while(((int)(to_change-counter) >= BUFFER_BEGIN) && *(to_change-counter)!='\n' && counter<80) counter++;
                    counter--;
                    print_char_at(' ', current_mem_position);
                    current_mem_position -= 16*80;
                    for(int i=0; i<counter; i++){
                        print_char_at(*(to_change-counter+i), current_mem_position+i);
                    }
                    print_char_at('<', current_mem_position+counter);
                    current_mem_position += counter;
                }
                else{
                    print_char_at(' ', current_mem_position);
                    current_mem_position -= 15*80;
                    current_mem_position--;
                    print_char_at('<', current_mem_position);
                }
            }
            else{
                print_char_at(' ', current_mem_position);
                current_mem_position--;
                print_char_at('<', current_mem_position);
            }
        }
        else{
            //TODO
        }
    }
    else if(scancode == ENTER){
        char* to_change = (char*)BUFFER_BEGIN + current_mem_addr;
        char prev_char = *to_change;
        *to_change = '\n';
        to_change++;
        print_char_at(' ', current_mem_position);
        current_mem_position += (80 - (current_mem_position % 80));
        current_mem_position += 15*80;
        current_mem_addr++;
        move_buffer(to_change, prev_char);
        if(current_mem_position == 480*80){
            scroll_up();
            current_mem_position -= 16*80;
            put_on_screen(to_change, current_mem_position);
        }
        else if(prev_char != '\0'){
            //TODO
        }
        print_char_at('<', current_mem_position);
    }
    else{
        char letter = sc_ascii[(int)scancode];
        print_char_at(letter, current_mem_position);
        char* to_change = (char*)BUFFER_BEGIN + current_mem_addr;
        char prev_char = *to_change;
        *to_change = letter;
        to_change++;
        current_mem_position++;
        current_mem_position = ((current_mem_position % 80) == 0) ? current_mem_position+15*80 : current_mem_position;
        current_mem_addr++;
        move_buffer(to_change, prev_char);
        if(current_mem_position == 480*80){
            scroll_up();
            current_mem_position -= 16*80;
            put_on_screen(to_change, current_mem_position);            
        }
        else if(prev_char != '\0'){
            //TODO
        }
        print_char_at('<', current_mem_position);
    }
}

void editor_timer_callback(){
    //DO NOTHING
}

void move_buffer(char* to_change, char prev_char){
    int addr_offset = current_mem_addr;
    char temp_char;
    while((temp_char = *to_change) != '\0'){
        *to_change = prev_char;
        prev_char = temp_char;
        to_change++;
    }
    *to_change = prev_char;
}

void shrink_buffer(char* to_change){
    while(*to_change != '\0'){
        *to_change = *(to_change+1);
        to_change++;
    }
}