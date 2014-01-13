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

#ifndef ACCESSORY_H_
#define ACCESSORY_H_

#include <stdint.h>

typedef struct {
	uint16_t vendor_id;
	uint16_t product_id;
	int aoa_version;
	uint16_t aoa_vendor_id;
	uint16_t aoa_product_id;
	uint8_t aoa_endpoint_in;
	uint8_t aoa_endpoint_out;
	int was_interface_claimed;
	int was_kernel_driver_detached;
	struct libusb_device_handle *handle;
} accessory_device;

void accessory_finalize();
accessory_device *accessory_get_device();
accessory_device *accessory_get_device_with_vid_pid(uint16_t vendor_id, uint16_t product_id);
void accessory_free_device(accessory_device *ad);
int accessory_init();
int accessory_get_endpoints(accessory_device *ad);
int accessory_receive_data(accessory_device *ad, unsigned char *buffer, int buffer_size);
void accessory_send_data(accessory_device *ad, unsigned char *buffer, int size);

#endif /* ACCESSORY_H_ */
