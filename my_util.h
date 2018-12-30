/** @file my_util.h
 ** @author Justin Brown
 ** @description cs-162 Final Project
 ** @date June 4th, 2013
**/

#ifndef MY_UTIL_H
#define MY_UTIL_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void my_itoa(int n, int base, char **s);
char dtohc(int n); // digit to hex char
int my_isdigit(int n);
int get_size(int n, int base);
int my_strlen(char *s);
void my_reverse(char s[]);

#endif // MY_UTIL_H
