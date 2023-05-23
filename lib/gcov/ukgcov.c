/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the MIT License (the "License", see COPYING.md).
 * You may not use this file except in compliance with the License.
 */
/*
 * The code in this file was derived from gcc-13-onlinedocs:
 * Source: https://gcc.gnu.org/onlinedocs/gcc/Freestanding-Environments.html
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <uk/essentials.h>
#include <uk/gcov.h>

#ifdef GCOV_OPT_OUTPUT_BINARY_MEMORY
static unsigned char __used *gcov_output_buffer;
static size_t __used gcov_output_buffer_pos;
static size_t gcov_output_buffer_size = INITIAL_BUFFER_SIZE;
#endif

/* Gcov symbols: (Source) https://github.com/gcc-mirror/gcc/blob/master/libgcc/gcov.h */
struct gcov_info;
extern void __gcov_reset(void);
extern void __gcov_dump(void);
extern void __gcov_info_to_gcda(const struct gcov_info *__info,
			void (*__filename_fn)(const char *, void *),
			void (*__dump_fn)(const void *, unsigned, void *),
			void *(*__allocate_fn)(unsigned, void *),
			void *__arg);
extern void __gcov_filename_to_gcfn(const char *__filename,
			void (*__dump_fn)(const void *, unsigned, void *),
			void *__arg);

/* The start and end symbols are provided by the linker script.  We use the
 * array notation to avoid issues with a potential small-data area.
 */
extern const struct gcov_info *const __gcov_info_start[];
extern const struct gcov_info *const __gcov_info_end[];

#ifdef GCOV_OPT_OUTPUT_BINARY_MEMORY
static void dump_memory(const void *d, unsigned n, void *arg __unused)
{
	const unsigned char *c = d;

	if (gcov_output_buffer_pos + n >= gcov_output_buffer_size) {
		unsigned char *temp_buffer = (unsigned char *)realloc(gcov_output_buffer, gcov_output_buffer_size <<= 1);

		if (temp_buffer == NULL) {
			free(gcov_output_buffer);
			return;
		}
		gcov_output_buffer = temp_buffer;
	}
	memcpy(gcov_output_buffer + gcov_output_buffer_pos, c, n);
	gcov_output_buffer_pos += n;
}
#endif

#ifdef GCOV_OPT_OUTPUT_BINARY_FILE
static void dump_file(const void *d, unsigned n, void *arg)
{
	FILE *file = arg;
	const unsigned char *c = d;

	fwrite(c, n, 1, file);
}
#endif

#ifdef GCOV_OPT_OUTPUT_SERIAL_HEXDUMP
static inline unsigned char *encode(unsigned char c, unsigned char buf[2])
{
	buf[0] = c % 16 + 'a';
	buf[1] = (c / 16) % 16 + 'a';
	return buf;
}

static void dump_serial(const void *d, unsigned n, void *arg __unused)
{
	const unsigned char *c = d;
	unsigned char buf[2];

	for (unsigned i = 0; i < n; ++i) {
		char *encoded = encode(c[i], buf);
		putchar(encoded[0]);
		putchar(encoded[1]);
	}
}
#endif

/* This function shall produce a reliable in order byte stream to transfer the
 * gcov information from the target to the host system.
 */
static void dump(const void *d, unsigned n, void *arg)
{
#ifdef GCOV_OPT_OUTPUT_BINARY_MEMORY
	dump_memory(d, n, arg);
#elif defined(GCOV_OPT_OUTPUT_BINARY_FILE)
	dump_file(d, n, arg);
#elif defined(GCOV_OPT_OUTPUT_SERIAL_HEXDUMP)
	dump_serial(d, n, arg);
#endif /* GCOV_OPT_OUTPUT_BINARY_MEMORY */
}

/* The filename is serialized to a gcfn data stream by the
 * __gcov_filename_to_gcfn() function.  The gcfn data is used by the
 * "merge-stream" subcommand of the "gcov-tool" to figure out the filename
 * associated with the gcov information.
 */
static void filename(const char *f, void *arg)
{
	__gcov_filename_to_gcfn(f, dump, arg);
}

/* The __gcov_info_to_gcda() function may have to allocate memory under
 * certain conditions.
 */
static void *allocate(unsigned length, void *arg __unused)
{
	return malloc(length);
}

#ifdef GCOV_OPT_OUTPUT_BINARY_MEMORY
static int gcov_dump_info_memory(void)
{
	const struct gcov_info *const *info = __gcov_info_start;
	const struct gcov_info *const *end = __gcov_info_end;

	gcov_output_buffer = (unsigned char *)malloc(gcov_output_buffer_size);
	if (gcov_output_buffer == NULL)
		return -ENOMEM;
	while (info != end) {
		__gcov_info_to_gcda(*info, filename, dump, allocate, NULL);
		++info;
	}
	return 0;
}
#endif

#ifdef GCOV_OPT_OUTPUT_BINARY_FILE
static int gcov_dump_info_file(void)
{
	const struct gcov_info *const *info = __gcov_info_start;
	const struct gcov_info *const *end = __gcov_info_end;

	FILE *file = fopen(GCOV_OUTPUT_BINARY_FILENAME, "w");

	if (file == NULL)
		return -ENOENT;

	while (info != end) {
		__gcov_info_to_gcda(*info, filename, dump, allocate, (void *)file);
		++info;
	}
	fclose(file);
	return 0;
}
#endif

#ifdef GCOV_OPT_OUTPUT_SERIAL_HEXDUMP
static int gcov_dump_info_serial(void)
{
	const struct gcov_info *const *info = __gcov_info_start;
	const struct gcov_info *const *end = __gcov_info_end;

	puts("\n");
	puts("GCOV_DUMP_INFO_SERIAL:");
	while (info != end) {
		__gcov_info_to_gcda(*info, filename, dump, allocate, NULL);
		putchar('\n');
		++info;
	}
	puts("GCOV_DUMP_INFO_SERIAL_END\n");
	return 0;
}
#endif

int gcov_dump_info (void)
{
#ifdef GCOV_OPT_OUTPUT_BINARY_MEMORY
	return gcov_dump_info_memory();
#elif defined(GCOV_OPT_OUTPUT_BINARY_FILE)
	return gcov_dump_info_file();
#elif defined(GCOV_OPT_OUTPUT_SERIAL_HEXDUMP)
	return gcov_dump_info_serial();
#endif

	/* Should never reach this point */
	return -1;
}
