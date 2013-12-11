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

/**
 * get asynchronous key value (not blocking) without put pressed char in stdio
 */
int getasynckey() {
	struct termios old_termios;
	struct termios new_termios;

	int ch;
	int old_fcntl;

	tcgetattr(fileno(stdin), &old_termios);
	new_termios = old_termios;
	new_termios.c_lflag &= ~(ICANON | ECHO);
	new_termios.c_cc[VTIME] = 0;
	new_termios.c_cc[VMIN] = 0;
	tcsetattr(fileno(stdin), TCSANOW, &new_termios);

	old_fcntl = fcntl(0, F_GETFL, 0);
	fcntl(0, F_SETFL, old_fcntl | O_NONBLOCK);

	ch = getchar();

	tcsetattr(fileno(stdin), TCSANOW, &old_termios);
	fcntl(0, F_SETFL, old_fcntl);

	return ch;
}
