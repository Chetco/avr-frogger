/** @file my_util.c
 ** @author Justin Brown
 ** @description cs-162 Final Project
 ** @date June 4th, 2013
**/

#include "my_util.h"

void my_itoa(int n, int base, char **s)
{ // convert n of base to char array
  // assume char array is of correct size

  // same structure from K&R
  int i, sign;

  if ((sign = n) < 0) //record sign
    n = -n;
  else
    sign = 0;

  i = 0;
  if(base <= 10)
  {
    do { // generate digits in reverse order
      (*s)[i++] = n % base + '0';
    } while ((n /= base) > 0);
  }
  else
  {
    do {
      int temp = n % base;
      if(temp > 9)
        (*s)[i++] = dtohc(temp);
      else
        (*s)[i++] = n % base + '0';
    } while((n /= base) > 0);
  }

  if (sign)
    (*s)[i++] = '-';

  (*s)[i] = '\0';
  my_reverse((*s));
}

char dtohc(int n)
{ // take n, output the corresponding hex character
  if(my_isdigit(n))
  {
    return n + '0';
  }
  else
  {
    switch(n)
    {
      case 10:
        return 'a';
      case 11:
        return 'b';
      case 12:
        return 'c';
      case 13:
        return 'd';
      case 14:
        return 'e';
      case 15:
        return 'f';
      default:
        //printf("ERROR\n");
        //exit(1);
		return '\a';
    }
  }
}

int my_isdigit(int n)
{ // is it digit?
  return (n >= '0' && n <= '9') ? 1 : 0; // cake : death
}

int get_size(int num, int base)
{
  int size = 1; // account for null terminator
	float temp = num; // allow floating point division

	if (num < 0)
  	{ // ensure positive
    	temp = -temp;
    	size++; // account for sign
 	}

	while (temp >= 1)
    {
    	temp /= (float)base;
		size++;
	}
	return size;
}

int my_strlen(char *s)
{ // The C Programming Language, K&R : returns a string's length
  int n;
  for (n = 0; *s != '\0'; s++)
    n++;
  return n;
}

void my_reverse(char s[])
{ // The C Programming Language, K&R : reversed a string
  int c, i, j;
  for (i = 0, j = my_strlen(s)-1; i < j; i++, j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}

