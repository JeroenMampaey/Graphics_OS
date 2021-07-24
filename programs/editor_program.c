#include "editor_program.h"

//private functions
void move_buffer(char* to_change, char prev_char);
void shrink_buffer(char* to_change);


void kernel_to_editor(){
    clear_screen();
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

//this code is not pretty but it is NOT lazy and appears to work
void editor_keyboard_callback(unsigned char scancode){
    if(scancode == (unsigned char)72){   //upkey
        //TODO: scrolling
        if(current_mem_addr == 0) return;
        char* cursor_char = (char*)BUFFER_BEGIN + current_mem_addr;
        if(*cursor_char=='\n' || *cursor_char=='\0') print_char_at(' ', current_mem_position);
        else print_char_at(*cursor_char, current_mem_position);
        int found = 0;
        int offset = current_mem_position;
        while(cursor_char >= (char*)BUFFER_BEGIN){
            cursor_char--;
            if(*cursor_char=='\n' && found==0){
                found = 1;
            }
            else if(*cursor_char=='\n' && found==1){
                found = 2;
                break;
            }
            offset = ((offset % 80) == 0) ? offset-15*80 : offset;
            offset--;
        }
        if(found == 1){
            current_mem_addr = 0;
            current_mem_position = 0;
        }
        else if(found == 2){
            cursor_char++;
            offset = offset - (offset % 80);
            current_mem_position = offset;
            current_mem_addr = (int)(cursor_char-(char*)BUFFER_BEGIN);
        }
        print_char_at('<', current_mem_position);
    }
    else if(scancode == (unsigned char)80){   //downkey
        //TODO: scrolling
        char* cursor_char = (char*)BUFFER_BEGIN + current_mem_addr;
        if(*cursor_char == '\0') return;
        if(*cursor_char=='\n' || *cursor_char=='\0') print_char_at(' ', current_mem_position);
        else print_char_at(*cursor_char, current_mem_position);
        int offset = current_mem_position;
        while(*cursor_char != '\0'){
            if(*cursor_char=='\n'){
                cursor_char++;
                offset = offset - (offset % 80);
                offset += 16*80;
                current_mem_position = offset;
                current_mem_addr = (int)(cursor_char-(char*)BUFFER_BEGIN);
                break;
            }
            offset++;
            offset = ((offset % 80) == 0) ? offset+15*80 : offset;
            cursor_char++;
        }
        print_char_at('<', current_mem_position);
    }
    else if(scancode == (unsigned char)75){   //leftkey
        if(current_mem_addr == 0) return;
        char* cursor_char = (char*)BUFFER_BEGIN + current_mem_addr;
        if(*cursor_char == '\n' || *cursor_char == '\0') print_char_at(' ', current_mem_position);
        else print_char_at(*cursor_char, current_mem_position);
        current_mem_addr--;
        cursor_char--;
        if(current_mem_position==0){
            if(*cursor_char == '\n'){
                int counter = 1;
                while((int)(cursor_char-counter)>=BUFFER_BEGIN && *(cursor_char-counter)!='\n') counter++;
                counter--;
                counter = counter % 80;
                scroll_down();
                for(int i=0; i<counter; i++){
                    print_char_at(*(cursor_char-counter+i), current_mem_position+i);
                }
                current_mem_position += counter;
            }
            else{
                current_mem_position = 79;
                scroll_down();
                for(int i=0; i<80;i++){
                    print_char_at(*(cursor_char-i), current_mem_position-i);
                }
            }
        }
        else if((current_mem_position % 80) == 0){
            if(*cursor_char == '\n'){
                int counter = 1;
                while((int)(cursor_char-counter)>=BUFFER_BEGIN && *(cursor_char-counter)!='\n') counter++;
                counter--;
                counter = counter % 80;
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
        print_char_at('<', current_mem_position);
    }
    else if(scancode == (unsigned char)77){   //rightkey
        char* cursor_char = (char*)BUFFER_BEGIN + current_mem_addr;
        if(*cursor_char == '\0') return;

        if(*cursor_char == '\n'){
            print_char_at(' ', current_mem_position);
            current_mem_position += (80 - (current_mem_position % 80));
            current_mem_position += 15*80;
            if(current_mem_position == 480*80){
                scroll_up();
                current_mem_position -= 16*80;
                put_on_screen(cursor_char+1, current_mem_position);
            }
        }
        else{
            print_char_at(*cursor_char, current_mem_position);
            current_mem_position++;
            if((current_mem_position % 80) == 0){
                current_mem_position += 15*80;
                if(current_mem_position == 480*80){
                    scroll_up();
                    current_mem_position -= 16*80;
                    put_on_screen(cursor_char+1, current_mem_position);
                }
            }
        }
        current_mem_addr++;
        print_char_at('<', current_mem_position);
    }
    else if(scancode > SC_MAX) return;
    else if(scancode == BACKSPACE){
        if(current_mem_addr == 0) return;
        current_mem_addr--;
        char* to_change = (char*)BUFFER_BEGIN + current_mem_addr;
        char prev_char = *to_change;
        shrink_buffer(to_change);
        int counter_original = 0;
        int counter_new = 0;
        int line_counter_original = (prev_char=='\n') ? 1 : 0;
        int line_counter_new = 0;
        int line_counter_new_under = 1;
        int original_mem_position = current_mem_position; 
        char* to_change_original = to_change-1;
        while(*to_change_original!='\n' && (int)to_change_original>=BUFFER_BEGIN){
            counter_original++;
            counter_new++;
            if((counter_new % 80) == 0){
                line_counter_original++;
                line_counter_new++;
            }
            to_change_original--;
        }
        to_change_original = to_change;
        if(prev_char=='\n') counter_original = 0;
        else{
            counter_original++;
            if((counter_original % 80)==0) line_counter_original++;
        }
        if((current_mem_position % 80)==0 && current_mem_position!=0) current_mem_position -= 16*80;
        current_mem_position -= (current_mem_position % 80);
        int offset = current_mem_position;
        current_mem_position += (counter_new % 80);
        while(*to_change_original!='\n' && *to_change_original!='\0'){
            counter_original++;
            counter_new++;
            if((counter_original % 80)==0) line_counter_original++;
            if((counter_new % 80)==0){
                line_counter_new++;
                line_counter_new_under++;
            }
            to_change_original++;
        }
        if(line_counter_new==line_counter_original){
            if(original_mem_position==0) scroll_down();
            if(line_counter_new_under>30) line_counter_new_under=30;
            clear_dwords_asm((int)VIDEO_MEMORY+offset, (int)(16*80*line_counter_new_under/4));
            for(int i=0; i<(current_mem_position % 80); i++){
                print_char_at(*(to_change-1-i), current_mem_position-1-i);
            }
            offset = current_mem_position;
            while(*to_change!='\n' && *to_change!='\0'){
                print_char_at(*to_change, offset);
                offset++;
                offset = ((offset % 80) == 0) ? offset+15*80 : offset;
                to_change++;
            }
        }
        else{
            if(line_counter_new_under>30) line_counter_new_under=30;
            if(original_mem_position!=0){
                set_read_color(0);
                if(offset!=480*80-16*80) move_dwords_asm((int)(VIDEO_MEMORY+offset+16*80), (int)(VIDEO_MEMORY+offset), (int)((464*80-offset)/4));
            }
            clear_dwords_asm((int)VIDEO_MEMORY+offset, (int)(16*80*line_counter_new_under/4));
            for(int i=0; i<(current_mem_position % 80); i++){
                print_char_at(*(to_change-1-i), current_mem_position-1-i);
            }
            offset = current_mem_position;
            while(*to_change!='\n' && *to_change!='\0'){
                print_char_at(*to_change, offset);
                offset++;
                offset = ((offset % 80) == 0) ? offset+15*80 : offset;
                to_change++;
            }
            if(original_mem_position!=0){
                if(*to_change=='\n'){
                    while(offset!=464*80 && *to_change!='\0'){
                        if(*to_change=='\n'){
                            offset += (80 - (offset % 80));
                            offset += 15*80;
                        }
                        else{
                            offset++;
                            offset = ((offset % 80)==0) ? offset+15*80 : offset;
                        }
                        to_change++;
                    }
                    clear_dwords_asm((int)VIDEO_MEMORY+480*80-16*80, (int)(16*80/4));
                    for(int i=0; i<80 && *to_change!='\0' && *to_change!='\n'; i++, offset++, to_change++){
                        print_char_at(*to_change, offset);
                    }
                }
                else if(current_mem_position >= 480*80-16*2*80){
                    clear_dwords_asm((int)VIDEO_MEMORY+480*80-16*80, (int)(16*80/4));
                }
            }
        }
        print_char_at('<', current_mem_position);
    }
    else if(scancode == ENTER){
        char* to_change = (char*)BUFFER_BEGIN + current_mem_addr;
        char prev_char = *to_change;
        *to_change = '\n';
        to_change++;
        print_char_at(' ', current_mem_position);
        current_mem_addr++;
        move_buffer(to_change, prev_char);
        if(current_mem_position+16*80 >= 480*80){
            current_mem_position++;
            while((current_mem_position % 80)!=0){
                print_char_at(' ', current_mem_position);
                current_mem_position++;
            }
            scroll_up();
            current_mem_position -= 80;
            put_on_screen(to_change, current_mem_position);
        }
        else{
            int offset = current_mem_position+1;
            char* to_change_original = to_change-2;
            int counter_original = 0;
            int counter_new = 0;
            int line_counter_original = 0;
            int line_counter_new = 1;
            int line_counter_new_under = 1;
            current_mem_position += (80 - (current_mem_position % 80));
            current_mem_position += 15*80;
            while((offset % 80)!=0){
                print_char_at(' ', offset);
                offset++;
            }
            offset += 15*80;
            while(*to_change_original!='\n' && (int)to_change_original>=BUFFER_BEGIN){
                counter_original++;
                if((counter_original % 80) == 0){
                    line_counter_original++;
                    line_counter_new++;
                }
                to_change_original--;
            }
            to_change_original = to_change;
            while(*to_change_original!='\n' && *to_change_original!='\0'){
                counter_original++;
                counter_new++;
                if((counter_original % 80)==0) line_counter_original++;
                if((counter_new % 80)==0){
                    line_counter_new++;
                    line_counter_new_under++;
                }
                to_change_original++;
            }
            if(line_counter_new==line_counter_original){
                if(line_counter_new_under>30) line_counter_new_under=30;
                clear_dwords_asm((int)VIDEO_MEMORY+offset, (int)(16*80*line_counter_new_under/4));
                while(*to_change!='\n' && *to_change!='\0'){
                    print_char_at(*to_change, offset);
                    to_change++;
                    offset++;
                    offset = ((offset % 80) == 0) ? offset+15*80 : offset;
                }
            }
            else{
                set_read_color(0);
                if(offset!=480*80-16*80) move_dwords_reverse_asm((int)(VIDEO_MEMORY+0x90FC), (int)(VIDEO_MEMORY+0x95FC), (int)((464*80-offset)/4));
                if(line_counter_new_under>30) line_counter_new_under=30;
                clear_dwords_asm((int)VIDEO_MEMORY+offset, (int)(16*80*line_counter_new_under/4));
                while(*to_change!='\n' && *to_change!='\0'){
                    print_char_at(*to_change, offset);
                    to_change++;
                    offset++;
                    offset = ((offset % 80) == 0) ? offset+15*80 : offset;
                }
            }
        }
        print_char_at('<', current_mem_position);
    }
    else if(scancode == (unsigned char)1){    //ESC-key
        current_program = 0;
        editor_to_kernel();
        clear_screen();
        printk("\n");
	    printk("> ");
    }
    else{
        char letter = sc_ascii[(int)scancode];
        print_char_at(letter, current_mem_position);
        char* to_change = (char*)BUFFER_BEGIN + current_mem_addr;
        char prev_char = *to_change;
        *to_change = letter;
        to_change++;
        int offset = current_mem_position;
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
            while(prev_char!='\n' && prev_char!='\0' && offset<480*80){
                offset++;
                offset = ((offset % 80) == 0) ? offset+15*80 : offset;
                print_char_at(prev_char, offset);
                to_change++;
                prev_char = *to_change;
            }
            if(prev_char=='\n' && (offset % 80)==79 && offset!=480*80-15*80-1){
                offset++;
                offset = ((offset % 80) == 0) ? offset+15*80 : offset;
                set_read_color(0);
                if(offset!=480*80-16*80) move_dwords_reverse_asm((int)(VIDEO_MEMORY+0x90FC), (int)(VIDEO_MEMORY+0x95FC), (int)((464*80-offset)/4));
                clear_dwords_asm((int)VIDEO_MEMORY+offset, (int)(16*80/4));
            }
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