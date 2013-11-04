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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/param.h>

#include "uart.h"

static void uart_set_blocking(int fd, int should_block);
static int uart_set_interface_attribs(int fd, int speed, int parity);

static int fd = -1;

int uart_open(const char *device_name, int speed, int parity) {

	fd = open(device_name, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0)
		return -1;

	if (uart_set_interface_attribs(fd, speed, parity) < 0)
		return -1;

	// set no blocking mode
	uart_set_blocking(fd, 0);

	return 0;
}

void uart_close() {
	if (fd != 0)
		close(fd);
}

void uart_send_buffer(void *buffer, size_t size) {
	write(fd, buffer, size);
}

void uart_receive_buffer(void* buffer, size_t size) {
	read(fd, buffer, size);
}

int uart_receive_buffer_timout(void* buffer, size_t size, int timeout) {
	struct timeval to = { (timeout * 2) / 1000, ((timeout * 2) % 1000) * 1000 };
	int ret, bytes_read = 0;
	fd_set read_fds;

	FD_ZERO(&read_fds);
	FD_SET(fd, &read_fds);

	while (bytes_read < size && select(fd + 1, &read_fds, NULL, NULL, &to) == 1) {
		ret = read(fd, buffer + bytes_read, size - bytes_read);
		if (ret < 0 && errno != EAGAIN) {
			perror("serial, read:");
			return 1;
		}
		bytes_read += MAX(ret, 0);
	}
	return bytes_read;
}

void uart_set_blocking(int fd, int should_block) {
	struct termios tty;

	memset(&tty, 0, sizeof tty);

	if (tcgetattr(fd, &tty) != 0) {
		return;
	}

	tty.c_cc[VMIN] = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	if (tcsetattr(fd, TCSANOW, &tty) != 0)
		return;
}

int uart_set_interface_attribs(int fd, int speed, int parity) {
	struct termios tty;

	memset(&tty, 0, sizeof tty);

	if (tcgetattr(fd, &tty) != 0) {
		return -1;
	}

	cfsetospeed(&tty, speed);
	cfsetispeed(&tty, speed);

	// 8-bit chars
	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;

	// disable IGNBRK for mismatched speed tests; otherwise receive break as \000 chars
	tty.c_iflag &= ~IGNBRK;			// ignore break signal
	tty.c_lflag = 0;	 			// no signaling chars, no echo, no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN] = 0;            	// read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	// shut off xon/xoff ctrl
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);

	// ignore modem controls, enable reading
	tty.c_cflag |= (CLOCAL | CREAD);

	// shut off parity
	tty.c_cflag &= ~(PARENB | PARODD);
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		return -1;
	}

	return 0;
}
