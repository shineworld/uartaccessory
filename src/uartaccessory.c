/*
 * The MIT License (MIT)
 * Copyright (c) 2013 Silverio Diquigiovanni <shineworld.software@gmail.com>
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

#include <getopt.h>
#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "accessory.h"
#include "sysutils.h"
#include "uart.h"

#define ACCESSORY_MODE_BUFFER_SIZE 16384

#define ARRAY_LEN(x)    ( sizeof( x ) / sizeof( x[ 0 ]))

static void print_buffer(unsigned char *buffer, int size, int type);

static int option_quiet = 0;
static int option_colors = 0;
static int option_no_reply = 0;
static int option_closed_loop = 0;
static const char *option_baud = "115200";
static const char *option_port = "/dev/ttyUSB0";

typedef struct
{
	speed_t speed;
	unsigned baud_rate;
} baud_info;

baud_info baud_table[] = {
		{ B50, 50 },
		{ B75, 75 },
		{ B110, 110 },
		{ B134, 134 },
		{ B150, 150 },
		{ B200, 200 },
		{ B300, 300 },
		{ B600, 600 },
		{ B1200, 1200 },
		{ B1800, 1800 },
		{ B2400, 2400 },
		{ B4800, 4800 },
		{ B9600, 9600 },
		{ B19200, 19200 },
		{ B38400, 38400 },
		{ B57600, 57600 },
		{ B115200, 115200 },
		{ B230400, 230400 }
};

int main(int argc, char *argv[]) {

	static struct option long_options[] = {
			{ "uart-port", required_argument, 0, 'p' },
			{ "baud-rate", required_argument, 0, 'b' },
			{ "closed-loop", no_argument, 0, 'c' },
			{ "no-reply", no_argument, 0, 'n' },
			{ "colors", no_argument, 0, 'j' },
			{ "quiet", no_argument, 0, 'q' },
			{ "help", no_argument, 0, 'h' },
			{ 0, 0, 0, 0 }
	};

	int option;
	int option_index = 0;

	while ((option = getopt_long(argc, argv, "p:b:cnjqh", long_options, &option_index)) != -1) {
		switch (option) {
		case 0:
			break;
		case 'p':
			if (optarg) {
				option_port = optarg;
			}
			break;
		case 'b':
			if (optarg) {
				option_baud = optarg;
			}
			break;
		case 'c':
			option_closed_loop = 1;
			break;
		case 'n':
			option_no_reply = 1;
			break;
		case 'j':
			option_colors = 1;
			break;
		case 'q':
			option_quiet = 1;
			break;
		case 'h':
			puts("Usage: uartaccessory [options]");
			puts("Options:");
			puts("  -h, --help               Display this information");
			puts("  -p, --uart-port          Set the uart port. Example: use -p /dev/ttyUSB3 or simply port number -p 3. Default is /dev/ttyUSB0");
			puts("  -b, --baud-rate          Set the baud rate. Example: use -b 57600. Standard values are permitted. Default is 115200");
			puts("  -c, --closed-loop        Enable closed loop mode where accessory TX/RX are logically coupled and ttyUSBx disabled");
			puts("  -n, --no-reply           Disable sending of RX data packets (reply to request)");
			puts("  -j, --colors             Enable colors to show TX vs RX packets");
			puts("  -q, --quiet              Quiet mode");
			return EXIT_SUCCESS;
		}
	}

	accessory_device *ad = NULL;

	accessory_init();

	int need_quit = 0;

	while (need_quit == 0) {
		puts("");
		puts("Looking for accessory device... Press Q to quit");
		while (1) {
			int key = getasynckey();
			if (key == 'Q' || key == 'q') {
				need_quit = 1;
				break;
			}

			ad = accessory_get_device();
			if (ad != NULL) {
				printf(" - Found Android device with ID=%04x:%04x now connected as ID=%04x:%04x, version %d\n", ad->vendor_id, ad->product_id, ad->aoa_vendor_id, ad->aoa_product_id, ad->aoa_version);
				break;
			}
			usleep(500000);
		}
		if (need_quit != 0)
			break;

		puts("");
		puts("Capture and show data flow coming from Android device... Press Q to quit");

		unsigned char *buffer = malloc(ACCESSORY_MODE_BUFFER_SIZE);
		if (buffer == NULL)
			return EXIT_FAILURE;

		if (option_closed_loop == 0) {

			char device_name[256];
			if (option_port[0] == '/')
				strcpy(device_name, option_port);
			else {
				strcpy(device_name, "/dev/ttyUSB");
				strcpy(&device_name[11], option_port);
			}

			int i;
			speed_t baud_rate = B0;
			int requested_baud = atoi(option_baud);
			for (i = 0; i < ARRAY_LEN(baud_table); i++) {
				if (baud_table[i].baud_rate == requested_baud) {
					baud_rate = baud_table[i].speed;
					break;
				}
			}
			if (baud_rate == B0) {
				fprintf(stderr, "Unrecognized baud rate: '%s'\n", option_baud);
				exit(1);
			}

			if (uart_open(device_name, baud_rate, 0) < 0) {
				char message[256];
				snprintf(message, sizeof message, "Unable to open serial port %s", device_name);
				perror(message);
				return EXIT_FAILURE;
			}
		}

		while (1) {
			int key = getasynckey();
			if (key == 'Q' || key == 'q') {
				need_quit = 1;
				break;
			}

			int cnt = accessory_receive_data(ad, buffer, ACCESSORY_MODE_BUFFER_SIZE);
			if (cnt > 0) {
				print_buffer(buffer, cnt, 0);
				if (option_closed_loop == 0)
					uart_send_buffer(buffer, cnt);
				else
				if (option_no_reply == 0) {
					accessory_send_data(ad, buffer, cnt);
					print_buffer(buffer, cnt, 1);
				}
			} else if (cnt == LIBUSB_ERROR_NO_DEVICE) {
				puts("AOA device disconnected !");
				break;
			}

			if (option_no_reply == 0 && option_closed_loop == 0) {
				cnt = uart_receive_buffer_timout(buffer, ACCESSORY_MODE_BUFFER_SIZE, 0);
				if (cnt > 0) {
					accessory_send_data(ad, buffer, cnt);
					print_buffer(buffer, cnt, 1);
				}
			}
		}
		free(buffer);
	}

	accessory_free_device(ad);
	accessory_finalize();
	uart_close();

	return EXIT_SUCCESS;
}

#define COLOR_RED      "\e[31m"
#define COLOR_BLUE     "\e[34m"
#define COLOR_GREEN    "\e[32m"
#define COLOR_RESET    "\033[0m"

static void print_buffer(unsigned char *buffer, int size, int type) {
	int i;

	if (option_quiet)
		return;

	if (option_colors) {
		if (type == 0)
			printf(COLOR_BLUE);
		else
			printf(COLOR_RED);
	}
	for (i = 0; i < size; i++) {
		if (type == 0)
			printf("%02X ", buffer[i]);
		else
			printf("%02X ", buffer[i]);
	}
	if (option_colors)
		printf(COLOR_RESET "\n");

	fflush(stdout);
}
