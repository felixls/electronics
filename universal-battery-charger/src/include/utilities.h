#ifndef UTILITIES_H
#define UTILITIES_H

char *padr(char *d, char *s, int c, char fill);
int strlen (char * str);
char * strcpy (
	char * d, 
	char * s);

void uitoa(unsigned int value, char* string, int radix, char decimal);

void itoa(int value, char* string, int radix);

#endif

