#ifndef KEYBOARD_H
#define KEYBOARD_H
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

int getkey() {
	int character;
	struct termios orig_term_attr;
	struct termios new_term_attr;

	tcgetattr(fileno(stdin), &orig_term_attr);
	memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
	new_term_attr.c_lflag &= ~(ECHO|ICANON);
	new_term_attr.c_cc[VTIME] = 0;
	new_term_attr.c_cc[VMIN] = 1;
	tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

	character = fgetc(stdin);

	tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

	return character;
}

#endif
