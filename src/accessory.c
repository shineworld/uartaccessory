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

#include "accessory.h"

#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "usb_ch9.h"

#define USB_ACCESSORY_VENDOR_ID 		0x18D1
#define USB_ACCESSORY_PRODUCT_ID 		0x2D00
#define USB_ACCESSORY_ADB_PRODUCT_ID	0x2D01

#define ACCESSORY_STRING_MANUFACTURER	0
#define ACCESSORY_STRING_MODEL			1
#define ACCESSORY_STRING_DESCRIPTION	2
#define ACCESSORY_STRING_VERSION		3
#define ACCESSORY_STRING_URI			4
#define ACCESSORY_STRING_SERIAL		5

#define ACCESSORY_GET_PROTOCOL			51
#define ACCESSORY_SEND_STRING			52
#define ACCESSORY_START				53
#define ACCESSORY_REGISTER_HID			54
#define ACCESSORY_UNREGISTER_HID		55
#define ACCESSORY_SET_HID_REPORT_DESC	56
#define ACCESSORY_SEND_HID_EVENT		57
#define ACCESSORY_SET_AUDIO_MODE		58

#define ACCESSORY_MANUFACTURER			"SHINE"
#define ACCESSORY_MODEL				"Android Accessory Emulator"
#define ACCESSORY_DESCRIPTION			"SHINE Android Accessory Emulator"
#define ACCESSORY_VERSION				"1.0"
#define ACCESSORY_URI					"https://github.com/shineworld/uartaccessory"
#define ACCESSORY_SERIAL				"SHINE Emulator"

#define VOID_ID						0xFFFF
#define FDTI_ID						0x0403
#define VMWARE_ID						0x0E0F

#define CONNECT_TRIES					15
#define CONNECT_TRIES_MS_DELAY			1000

#define LIBUSB_VERBOSE_LEVEL			0

#define ENDPOINT_BULK_IN 				0x81
#define ENDPOINT_BULK_OUT 				0x04

static int accessory_setup(accessory_device *ad);

static libusb_context *ctx = NULL;

void accessory_finalize() {
	if (ctx != NULL) {
		libusb_exit(ctx);
		ctx = NULL;
	}
}

accessory_device *accessory_get_device() {
	return accessory_get_device_with_vid_pid(VOID_ID, VOID_ID);
}

accessory_device *accessory_get_device_with_vid_pid(uint16_t vendor_id, uint16_t product_id) {
	if (ctx == NULL)
		return NULL;

	if (vendor_id != VOID_ID && product_id != VOID_ID) {
		accessory_device *ad = malloc(sizeof(accessory_device));
		memset(ad, 0, sizeof(accessory_device));

		ad->product_id = product_id;
		ad->vendor_id = vendor_id;

		ad->handle = libusb_open_device_with_vid_pid(ctx, ad->vendor_id, ad->product_id);
		if (ad->handle == NULL) {
			accessory_free_device(ad);
			return NULL;
		}

		// check whether a kernel driver is attached to interface #0. If so, we'll need to detach it.
		if (libusb_kernel_driver_active(ad->handle, 0)) {
			int res = libusb_detach_kernel_driver(ad->handle, 0);
			if (res == 0) {
				ad->was_kernel_driver_detached = 1;
			} else {
				accessory_free_device(ad);
				return NULL;
			}
		}

		ad->was_interface_claimed = libusb_claim_interface(ad->handle, 0);
		if (ad->was_interface_claimed != 0) {
			accessory_free_device(ad);
			return NULL;
		}

		if (accessory_setup(ad) < 0) {
			accessory_free_device(ad);
			return NULL;
		}

		return ad;
	} else {
		int i;
		ssize_t cnt;
		libusb_device **devs;

		cnt = libusb_get_device_list(ctx, &devs);
		if (cnt <= 0) {
			return NULL;
		}

		for (i = 0; i < cnt; i++) {
			struct libusb_device_descriptor desc;

			int r = libusb_get_device_descriptor(devs[i], &desc);
			if (r < 0)
				continue;

			if (desc.bDeviceClass != 0 || desc.idVendor == VMWARE_ID || desc.idVendor == FDTI_ID)
				continue;

			accessory_device *ad = malloc(sizeof(accessory_device));
			memset(ad, 0, sizeof(accessory_device));

			ad->vendor_id = desc.idVendor;
			ad->product_id = desc.idProduct;

			ad->handle = libusb_open_device_with_vid_pid(ctx, ad->vendor_id, ad->product_id);
			if (ad->handle == NULL) {
				accessory_free_device(ad);
				continue;
			}

			// check whether a kernel driver is attached to interface #0. If so, we'll need to detach it.
			if (libusb_kernel_driver_active(ad->handle, 0) == 1) {
				if (libusb_detach_kernel_driver(ad->handle, 0) == 0) {
					ad->was_kernel_driver_detached = 1;
				} else {
					accessory_free_device(ad);
					continue;
				}
			}

			ad->was_interface_claimed = libusb_claim_interface(ad->handle, 0);
			if (ad->was_interface_claimed != 0) {
				accessory_free_device(ad);
				continue;
			}

			if (accessory_setup(ad) < 0) {
				accessory_free_device(ad);
				continue;
			} else {
				libusb_free_device_list(devs, 1);
				return ad;
			}
		}

		libusb_free_device_list(devs, 1);
		return NULL;
	}
}

void accessory_free_device(accessory_device *ad) {
	if (ctx == NULL)
		return;

	if (ad != NULL) {
		if (ad->was_interface_claimed)
			libusb_release_interface(ad->handle, 0);

		if (ad->was_kernel_driver_detached)
			libusb_attach_kernel_driver(ad->handle, 0);

		if (ad->handle)
			libusb_close(ad->handle);

		free(ad);
	}
}

int accessory_init() {
	if (ctx != NULL)
		return -1;

	int r = libusb_init(&ctx);
	if (r < 0) {
		return -1;
	}

	libusb_set_debug(ctx, LIBUSB_VERBOSE_LEVEL);

	return 0;
}

static int accessory_setup(accessory_device *ad) {
	int res, tries;
	unsigned char buffer[2];

	if (ctx == NULL || ad == NULL)
		return -1;

	if (ad->vendor_id == USB_ACCESSORY_VENDOR_ID) {
		if (ad->product_id == USB_ACCESSORY_PRODUCT_ID || ad->product_id == USB_ACCESSORY_ADB_PRODUCT_ID) {
			ad->aoa_vendor_id = ad->vendor_id;
			ad->aoa_product_id = ad->product_id;
			return 0;
		}
	}

	res = libusb_control_transfer(ad->handle, USB_DIR_IN | USB_TYPE_VENDOR, ACCESSORY_GET_PROTOCOL, 0, 0, buffer, 2, 0);
	if (res < 0)
		return -1;

	ad->aoa_version = buffer[1] << 8 | buffer[0];

	usleep(1000);

	res = libusb_control_transfer(ad->handle, USB_DIR_OUT | USB_TYPE_VENDOR, ACCESSORY_SEND_STRING, 0, 0, (unsigned char *) ACCESSORY_MANUFACTURER, strlen(ACCESSORY_MANUFACTURER), 0);
	if (res < 0)
		return -1;
	res = libusb_control_transfer(ad->handle, USB_DIR_OUT | USB_TYPE_VENDOR, ACCESSORY_SEND_STRING, 0, 1, (unsigned char *) ACCESSORY_MODEL, strlen(ACCESSORY_MODEL) + 1, 0);
	if (res < 0)
		return -1;
	res = libusb_control_transfer(ad->handle, USB_DIR_OUT | USB_TYPE_VENDOR, ACCESSORY_SEND_STRING, 0, 2, (unsigned char *) ACCESSORY_DESCRIPTION, strlen(ACCESSORY_DESCRIPTION) + 1, 0);
	if (res < 0)
		return -1;
	res = libusb_control_transfer(ad->handle, USB_DIR_OUT | USB_TYPE_VENDOR, ACCESSORY_SEND_STRING, 0, 3, (unsigned char *) ACCESSORY_VERSION, strlen(ACCESSORY_VERSION) + 1, 0);
	if (res < 0)
		return -1;
	res = libusb_control_transfer(ad->handle, USB_DIR_OUT | USB_TYPE_VENDOR, ACCESSORY_SEND_STRING, 0, 4, (unsigned char *) ACCESSORY_URI, strlen(ACCESSORY_URI) + 1, 0);
	if (res < 0)
		return -1;
	res = libusb_control_transfer(ad->handle, USB_DIR_OUT | USB_TYPE_VENDOR, ACCESSORY_SEND_STRING, 0, 5, (unsigned char *) ACCESSORY_SERIAL, strlen(ACCESSORY_SERIAL) + 1, 0);
	if (res < 0)
		return -1;

	res = libusb_control_transfer(ad->handle, USB_DIR_OUT | USB_TYPE_VENDOR, ACCESSORY_START, 0, 0, NULL, 0, 0);
	if (res < 0) {
		//	Cause problems with some ANDROID devices running on Rockchip & AllWinner kernels we need to ignore
		//	a response error (-ESHUTDOWN)(-108) and continue with accessory device claim operations.

		// return -1;
	}

	usleep(1000);

	libusb_release_interface(ad->handle, 0);
	ad->was_interface_claimed = 0;
	libusb_close(ad->handle);
	ad->handle = 0;

	usleep(1000);

	tries = 0;
	while (1) {
		ad->handle = libusb_open_device_with_vid_pid(ctx, USB_ACCESSORY_VENDOR_ID, USB_ACCESSORY_PRODUCT_ID);
		if (ad->handle != NULL) {
			ad->aoa_vendor_id = USB_ACCESSORY_VENDOR_ID;
			ad->aoa_product_id = USB_ACCESSORY_PRODUCT_ID;
			break;
		}

		usleep(1000);

		ad->handle = libusb_open_device_with_vid_pid(ctx, USB_ACCESSORY_VENDOR_ID, USB_ACCESSORY_ADB_PRODUCT_ID);
		if (ad->handle != NULL) {
			ad->aoa_vendor_id = USB_ACCESSORY_VENDOR_ID;
			ad->aoa_product_id = USB_ACCESSORY_ADB_PRODUCT_ID;
			break;
		}

		tries++;
		if (tries >= CONNECT_TRIES)
			return -1;

		usleep(CONNECT_TRIES_MS_DELAY * 1000);
	}

	ad->was_interface_claimed = libusb_claim_interface(ad->handle, 0);
	if (ad->was_interface_claimed != 0)
		return -1;

	return 0;
}

/**
 * THERE IS AN ISSUE in libusb_bulk_transfer, or at least I thought there is. If you get the return value LIBUSB_ERROR_TIMEOUT
 * you should find in transfered the amount of received data before the TIMEOUT, but if you get the same size of required "size"
 * this mean "you haven't got any real data", so you have to discard it.
 */
int accessory_receive_data(accessory_device *ad, unsigned char *buffer, int buffer_size) {
	const static int TIMEOUT = 2;
	int transferred = 0;

	int r = libusb_bulk_transfer(ad->handle, ENDPOINT_BULK_IN, buffer, buffer_size, &transferred, TIMEOUT);
	if (r != 0 && r != LIBUSB_ERROR_TIMEOUT)
		return r;
	// fix upon described issue
	if (r == LIBUSB_ERROR_TIMEOUT && transferred == buffer_size)
		return r;
	return transferred;
}

void accessory_send_data(accessory_device *ad, unsigned char *buffer, int size) {
	const static int PACKET_BULK_LEN = 512;
	const static int MAX_TRIES = 5;
	const static int TIMEOUT = 2;
	int transferred = 0;
	int to_send = 0;
	int tries = 0;
	int sent = 0;

	while (sent < size) {
		to_send = size - sent;
		if (to_send > PACKET_BULK_LEN)
			to_send = PACKET_BULK_LEN;
		int r = libusb_bulk_transfer(ad->handle, (ENDPOINT_BULK_OUT), &buffer[sent], to_send, &transferred, TIMEOUT);
		if (r != 0 && r != LIBUSB_ERROR_TIMEOUT)
			break;
		if (transferred > 0) {
			sent += transferred;
			tries = 0;
			continue;
		}
		if (++tries >= MAX_TRIES)
			break;
	}

	return;
}
