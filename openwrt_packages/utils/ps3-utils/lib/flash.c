/*
 *  PS3 flash memory os area.
 *
 *  Copyright (C) 2006 Sony Computer Entertainment Inc.
 *  Copyright 2006 Sony Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "ps3-flash.h"

#if defined(DEBUG)
#define DBG os_area_log
#else
static inline int __attribute__ ((format (printf, 1, 2))) DBG(
	__attribute__((unused)) const char *fmt, ...) {return 0;}
#endif

static FILE *osa_log;

/**
 * os_area_set_log_stream - Set the message logging stream.
 * @stream: A file stream to log messages to.
 *
 * The default log stream is stdout.  Returns the old
 * stream.
 */

FILE *os_area_set_log_stream(FILE *log)
{
	FILE *old = osa_log ? osa_log : stdout;

	assert(log);

	fflush(log);
	fflush(old);
		
	osa_log = log;
	return old;
}

int os_area_log(const char *fmt, ...)
{
	int ret;
	va_list ap;
	FILE *f;

	f = osa_log ? osa_log : stdout;

	va_start(ap, fmt);
	ret = vfprintf(f, fmt, ap);
	va_end(ap);
	fflush(f);

	return ret;
}

/**
 * os_area_header_read - Read the firmware defined header.
 */

int os_area_header_read(struct os_area_header *h, FILE *dev)
{
	int result;
	size_t bytes;

	if (!dev) {
		DBG("%s:%d: bad stream\n", __func__, __LINE__);
		return -1;
	}

	result = fseek(dev, 0, SEEK_SET);

	if (result) {
		os_area_log("%s: seek error: os_area_header: %s\n", __func__,
			strerror(errno));
		return result;
	}

	bytes = fread(h, 1, sizeof(struct os_area_header), dev);

	if (bytes < sizeof(struct os_area_header)) {
		os_area_log("%s: read error: os_area_header: %s\n", __func__,
			strerror(errno));
		return -1;
	}

	result = os_area_header_verify(h);

	if (result) {
		os_area_log("%s: invalid os_area_header\n", __func__);
		return -1;
	}

	return 0;
}

/**
 * os_area_header_write - Write the firmware defined header.
 */

int os_area_header_write(const struct os_area_header *h, FILE *dev)
{
	int result;
	size_t bytes;

	if (!dev) {
		DBG("%s:%d: bad stream\n", __func__, __LINE__);
		return -1;
	}

	result = os_area_header_verify(h);

	if (result) {
		os_area_log("%s: invalid os_area_header\n", __func__);
		return -1;
	}

	result = fseek(dev, 0, SEEK_SET);

	if (result) {
		os_area_log("%s: seek error: os_area_header: %s\n", __func__,
			strerror(errno));
		return result;
	}

	bytes = fwrite(h, 1, sizeof(struct os_area_header), dev);

	if (bytes < sizeof(struct os_area_header)) {
		os_area_log("%s: fwrite error: os_area_header: %s\n", __func__,
			strerror(errno));
		return -1;
	}

	return 0;
}

/**
 * os_area_params_read - Read the firmware defined params.
 */

int os_area_params_read(struct os_area_params *p, FILE *dev)
{
	int result;
	size_t bytes;

	if (!dev) {
		DBG("%s:%d: bad stream\n", __func__, __LINE__);
		return -1;
	}

	result = fseek(dev, OS_AREA_SEGMENT_SIZE, SEEK_SET);

	if (result) {
		os_area_log("%s: seek error: os_area_params: %s\n", __func__,
			strerror(errno));
		return result;
	}

	bytes = fread(p, 1, sizeof(struct os_area_params), dev);

	if (bytes < sizeof(struct os_area_params)) {
		os_area_log("%s: read error: os_area_params: %s\n", __func__,
			strerror(errno));
		return -1;
	}

	return 0;
}

/**
 * os_area_params_write - Write the firmware defined params.
 */

int os_area_params_write(const struct os_area_params *p, FILE *dev)
{
	int result;
	size_t bytes;

	if (!dev) {
		DBG("%s:%d: bad stream\n", __func__, __LINE__);
		return -1;
	}

	result = fseek(dev, OS_AREA_SEGMENT_SIZE, SEEK_SET);

	if (result) {
		os_area_log("%s: seek error: os_area_params: %s\n", __func__,
			strerror(errno));
		return result;
	}

	bytes = fwrite(p, 1, sizeof(struct os_area_params), dev);

	if (bytes < sizeof(struct os_area_params)) {
		os_area_log("%s: fwrite error: os_area_params: %s\n", __func__,
			strerror(errno));
		return -1;
	}

	return 0;
}

/**
 * os_area_image_write - Write the other os boot image.
 */

int os_area_image_write(FILE *image, struct os_area_header *h, FILE *dev)
{
	int result;
	size_t flash_size;
	size_t image_size;
	size_t count;
	void* buf;

	result = fseek(dev, 0, SEEK_END);

	if (result) {
		os_area_log("%s: fseek dev failed: %s\n", __func__,
			strerror(errno));
		return -1;
	}

	flash_size = ftell(dev) - h->ldr_area_offset * OS_AREA_SEGMENT_SIZE;

	DBG("%s:%d: flash_size %lxh\n", __func__, __LINE__,
		(unsigned long)flash_size);

	result = fseek(image, 0, SEEK_END);

	if (result) {
		os_area_log("%s: fseek image failed: %s\n", __func__,
			strerror(errno));
		return -1;
	}

	image_size = ftell(image);

	DBG("%s:%d: image_size %lxh\n", __func__, __LINE__,
		(unsigned long)image_size);

	if (image_size > flash_size) {
		os_area_log("%s: image too big (%lxh / %lxh bytes).\n",
			__func__, (unsigned long)image_size,
			(unsigned long)flash_size);
		return -1;
	}

	buf = malloc(image_size);

	if (!buf) {
		DBG("%s: malloc failed: %s\n", __func__, strerror(errno));
		return -1;
	}

	rewind(image);
	count = fread(buf, image_size, 1, image);

	if (count != 1) {
		os_area_log("%s: fread failed: %s\n", __func__,
			strerror(errno));
		return -1;
	}

	result = fseek(dev, h->ldr_area_offset * OS_AREA_SEGMENT_SIZE,
		SEEK_SET);

	if (result) {
		os_area_log("%s: seek error ldr_area_offset: %s\n", __func__,
			strerror(errno));
		return -1;
	}

	count = fwrite(buf, image_size, 1, dev);

	if (count != 1) {
		os_area_log("%s: fwrite error image: %s\n", __func__,
			strerror(errno));
		return -1;
	}

	result = os_area_set_ldr_size(h, dev, image_size);

	if (result) {
		os_area_log("%s: os_area_set_ldr_size failed: %s\n", __func__,
			strerror(errno));
		return -1;
	}

	os_area_log("wrote image to flash (%lxh / %lxh bytes).\n",
		(unsigned long)image_size, (unsigned long)flash_size);

	free(buf);

	return result;
}

/**
 * os_area_header_verify - Verify the firmware defined header.
 */

int os_area_header_verify(const struct os_area_header *h)
{
	if (memcmp(h->magic_num, "cell_ext_os_area", 16)) {
		os_area_log("%s: magic_num failed\n", __func__);
		return -1;
	}

	if (h->hdr_version < 1) {
		os_area_log("%s: hdr_version failed\n", __func__);
		return -1;
	}

	if (h->db_area_offset > h->ldr_area_offset) {
		os_area_log("%s: offsets failed\n", __func__);
		return -1;
	}

	return 0;
}

/**
 * os_area_set_ldr_format - Set the data format of the other os boot image.
 * @value: Specifies the format of the os boot image, one of enum ldr_format.
 */

int os_area_set_ldr_format(struct os_area_header *h, FILE *dev,
	enum os_area_ldr_format value)
{
	int result;

	DBG("%s:%d: %u(%xh) -> %u(%xh)\n", __func__, __LINE__,
		(unsigned int)h->ldr_format, (unsigned int)h->ldr_format,
		value, value);

	if (h->ldr_format == (uint32_t)value)
		return 0;

	h->ldr_format = (uint32_t)value ? HEADER_LDR_FORMAT_GZIP
		: HEADER_LDR_FORMAT_RAW; /* for now just flatten to gzip */

	os_area_header_write(h, dev);
	result = os_area_header_read(h, dev);

	if (result)
		os_area_log("%s: os_area_set_ldr_format failed\n", __func__);

	return result;
}

/**
 * os_area_set_ldr_size - Set the size in bytes of the other os boot image.
 * @value: Size in bytes.
 */

int os_area_set_ldr_size(struct os_area_header *h, FILE *dev,
	unsigned int value)
{
	int result;

	DBG("%s:%d: %u(%xh) -> %u(%xh)\n", __func__, __LINE__,
		(unsigned int)h->ldr_size, (unsigned int)h->ldr_size,
		value, value);

	if (h->ldr_size == (uint32_t)value)
		return 0;

	h->ldr_size = (uint32_t)value;

	os_area_header_write(h, dev);
	result = os_area_header_read(h, dev);

	if (result)
		os_area_log("%s: os_area_set_ldr_format failed\n", __func__);

	return result;
}

/**
 * os_area_set_boot_flag - Set the system boot flag.
 * @value: Boot flag value, one of enum os_area_boot_flag.
 */

int os_area_set_boot_flag(struct os_area_params *p, FILE *dev,
	enum os_area_boot_flag value)
{
	int result;

	DBG("%s:%d: %u(%xh) -> %u(%xh)\n", __func__, __LINE__,
		(unsigned int)p->boot_flag, (unsigned int)p->boot_flag,
		value, value);

	if (p->boot_flag == (uint32_t)value)
		return 0;

	p->boot_flag = (uint32_t)value ? PARAM_BOOT_FLAG_OTHER_OS
		: PARAM_BOOT_FLAG_GAME_OS; /* for now just flatten to otheros */

	os_area_params_write(p, dev);
	result = os_area_params_read(p, dev);

	if (result)
		os_area_log("%s: os_area_set_boot_flag failed\n", __func__);

	return result;
}

void os_area_header_dump(const struct os_area_header *h, const char *header,
	int id)
{
	os_area_log("%s:%d: h.magic_num:      '%s'\n",
		header, id,
		h->magic_num);
	os_area_log("%s:%d: h.hdr_version:     %u\n",
		header, id,
		h->hdr_version);
	os_area_log("%s:%d: h.db_area_offset:  %u\n",
		header, id,
		h->db_area_offset);
	os_area_log("%s:%d: h.ldr_area_offset: %u\n",
		header, id,
		h->ldr_area_offset);
	os_area_log("%s:%d: h.ldr_format:      %u (%s)\n",
		header, id,
		h->ldr_format, (h->ldr_format ? "gzip" : "raw"));
	os_area_log("%s:%d: h.ldr_size:        %u (%xh)\n",
		header, id,
		h->ldr_size, h->ldr_size);
}

void os_area_params_dump(const struct os_area_params *p, const char *header,
	int id)
{
	os_area_log("%s:%d: p.boot_flag:       %u (%s)\n",
		header, id,
		p->boot_flag, (p->boot_flag ? "other-os" : "game-os"));
	os_area_log("%s:%d: p.num_params:      %u\n",
		header, id,
		p->num_params);
	os_area_log("%s:%d: p.rtc_diff         %lld\n",
		header, id,
		(long long)p->rtc_diff);
	os_area_log("%s:%d: p.av_multi_out     %u\n",
		header, id,
		p->av_multi_out);
	os_area_log("%s:%d: p.ctrl_button:     %u\n",
		header, id,
		p->ctrl_button);
	os_area_log("%s:%d: p.static_ip_addr:  %u.%u.%u.%u\n",
		header, id,
		p->static_ip_addr[0], p->static_ip_addr[1],
		p->static_ip_addr[2], p->static_ip_addr[3]);
	os_area_log("%s:%d: p.network_mask:    %u.%u.%u.%u\n",
		header, id,
		p->network_mask[0], p->network_mask[1],
		p->network_mask[2], p->network_mask[3]);
	os_area_log("%s:%d: p.default_gateway: %u.%u.%u.%u\n",
		header, id,
		p->default_gateway[0], p->default_gateway[1],
		p->default_gateway[2], p->default_gateway[3]);
	os_area_log("%s:%d: p.dns_primary:     %u.%u.%u.%u\n",
		header, id,
		p->dns_primary[0], p->dns_primary[1],
		p->dns_primary[2], p->dns_primary[3]);
	os_area_log("%s:%d: p.dns_secondary:   %u.%u.%u.%u\n",
		header, id,
		p->dns_secondary[0], p->dns_secondary[1],
		p->dns_secondary[2], p->dns_secondary[3]);
}
