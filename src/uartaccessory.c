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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "accessory.h"

int main(void) {
	accessory_device *ad = NULL;

	accessory_init();
	puts("Looking for accessory device... (press CTRL+C to quit)");
	while (1) {
		ad = accessory_get_device();
		if (ad != NULL) {
			printf("\nFound AOA ready device with ID %04x:%04x and AOA version %d connected how ID %04x:%04x\n\n",
					ad->vendor_id,
					ad->product_id,
					ad->aoa_version,
					ad->aoa_vendor_id,
					ad->aoa_product_id);
			break;
		}
		usleep(500000);
	}
	accessory_free_device(ad);
	accessory_finalize();

/*
	int i;
	char buffer[1024];

	if (uart_open("/dev/ttyUSB0", 115200, 0) < 0) {
		perror("Unable to open serial port");
		return EXIT_FAILURE;
	}

	for (i = 0; i < 1000; i++) {
	uart_send_buffer("ciao silverio", 13);
	uart_receive_buffer_timout(buffer, 5, 1);
	}

	uart_close();
*/

	return EXIT_SUCCESS;
}
