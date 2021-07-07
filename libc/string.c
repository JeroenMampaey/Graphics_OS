#include "string.h"

void int_to_hex_ascii(int n, char str[]){
    int i;
    int current;
    i = 0;
    do {
        str[i++] = (n % 16)<10 ? (n % 16 + '0') : (n % 16 - 10 + 'A');
    } while ((n /= 16) > 0);
    str[i] = '\0';

    reverse(str);
}

void int_to_ascii(int n, char str[]) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0) str[i++] = '-';
    str[i] = '\0';

    reverse(str);
}

void hex_to_ascii(int n, char str[]) {  //wtf is this???
    append(str, '0');
    append(str, 'x');
    char zeros = 0;

    int tmp;
    int i;
    for (i = 28; i > 0; i -= 4) {
        tmp = (n >> i) & 0xF;
        if (tmp == 0 && zeros == 0) continue;
        zeros = 1;
        if (tmp > 0xA) append(str, tmp - 0xA + 'a');
        else append(str, tmp + '0');
    }

    tmp = n & 0xF;
    if (tmp >= 0xA) append(str, tmp - 0xA + 'a');
    else append(str, tmp + '0');
}

void reverse(char s[]) {
    int c, i, j;
    for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

int strlen(char s[]) {
    int i = 0;
    while (s[i] != '\0') ++i;
    return i;
}

void append(char s[], char n) {
    int len = strlen(s);
    s[len] = n;
    s[len+1] = '\0';
}

void backspace(char s[]) {
    int len = strlen(s);
    s[len-1] = '\0';
}

int strcmp(char s1[], char s2[]) {
    int i = 0;
    for (; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    return s1[i] - s2[i];
}

int substr_cmp(char s1[], char s2[]){
    int i = 0;
    for(; s1[i] == s2[i]; i++){
        if(s1[i] == '\0') return 0;
    }
    if(s2[i] == '\0') return 0;
    else return s1[i] - s2[i];
}

int ascii_to_hex(char s){
    if(s >= '0' && s <= '9'){
        return s - '0';
    }
    else if(s >= 'A' && s <= 'F'){
        return s - 'A' + 10;
    }
    else if(s >= 'a' && s <= 'f'){
        return s - 'a' + 10;
    }
    else return -1;
}