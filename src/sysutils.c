/*
 * The MIT License (MIT)
 * Copyright (c) 2013 Silverio Diquigiovanni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>

int getkey() {
	int character;
	struct termios old_term_attr;
	struct termios new_term_attr;

	// set the terminal to raw mode
	tcgetattr(fileno(stdin), &old_term_attr);
	memcpy(&new_term_attr, &old_term_attr, sizeof(struct termios));
	new_term_attr.c_lflag &= ~(ECHO | ICANON);
	new_term_attr.c_cc[VTIME] = 0;
	new_term_attr.c_cc[VMIN] = 0;
	tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

	// read a character from the stdin stream without blocking
	// returns EOF (-1) if no character is available
	character = fgetc(stdin);

	// restore the original terminal attributes
	tcsetattr(fileno(stdin), TCSANOW, &old_term_attr);

	return character;
}

int kbhit(void) {
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(0, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &newt);
	oldf = fcntl(0, F_GETFL, 0);
	fcntl(0, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(0, TCSANOW, &oldt);
	fcntl(0, F_SETFL, oldf);

	if (ch != EOF) {
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}
