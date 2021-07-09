#ifndef STRINGS_H
#define STRINGS_H

void int_to_ascii(int n, char str[]);
void int_to_hex_ascii(int n, char str[]);
void reverse(char s[]);
int strlen(char s[]);
void backspace(char s[]);
void append(char s[], char n);
int strcmp(char s1[], char s2[]);
int substr_cmp(char s1[], char s2[]);  //check if the first characters from s1 are equal to s2
int ascii_to_hex(char s);              //transform a hexadecimal number in char format to actual hexadecimal number (e.g 'A' becomes 0xA)

#endif