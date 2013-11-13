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

#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "accessory.h"
#include "uart.h"

#define ACCESSORY_BUFFER_SIZE 1024

static void print_buffer(unsigned char *buffer, int size, int type);

int main(void) {
	accessory_device *ad = NULL;

	accessory_init();

	while (1) {
		puts("");
		puts("Looking for accessory device...");
		while (1) {
			ad = accessory_get_device();
			if (ad != NULL) {
				printf(" - Found Android device with ID=%04x:%04x now connected as ID=%04x:%04x, version %d\n", ad->vendor_id, ad->product_id, ad->aoa_vendor_id, ad->aoa_product_id, ad->aoa_version);
				break;
			}
			usleep(500000);
		}

		puts("");
		puts("Capture and show data flow coming from Android device...");
		unsigned char *buffer = malloc(ACCESSORY_BUFFER_SIZE);
		if (buffer == NULL)
			return EXIT_FAILURE;

		if (uart_open("/dev/ttyUSB0", B115200, 0) < 0) {
			perror("Unable to open serial port");
			return EXIT_FAILURE;
		}

		while (1) {
			int cnt = accessory_receive_data(ad, buffer, ACCESSORY_BUFFER_SIZE - 1);
			if (cnt > 0) {
				print_buffer(buffer, cnt, 0);
				uart_send_buffer(buffer, cnt);
			} else if (cnt == LIBUSB_ERROR_NO_DEVICE) {
				puts("AOA device disconnected !");
				break;
			}

			cnt = uart_receive_buffer_timout(buffer, ACCESSORY_BUFFER_SIZE, 1);
			if (cnt > 0) {
				accessory_send_data(ad, buffer, cnt);
				print_buffer(buffer, cnt, 1);
			}

			usleep(1000);
		}
		free(buffer);
	}


	accessory_free_device(ad);
	accessory_finalize();

	return EXIT_SUCCESS;
}

#define COLOR_RED		"\e[31m"
#define COLOR_BLUE		"\e[34m"
#define COLOR_GREEN 	"\e[32m"
#define COLOR_RESET 	"\033[0m"

static void print_buffer(unsigned char *buffer, int size, int type) {
	int i;

	if (type == 0)
		printf(COLOR_BLUE);
	else
		printf(COLOR_RED);
	for (i = 0; i < size; i++) {
		if (type == 0)
			printf("%02X ", buffer[i]);
		else
			printf("%02X ", buffer[i]);
	}
	printf(COLOR_RESET "\n");

	fflush(stdout);
}
