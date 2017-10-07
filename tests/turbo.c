/* Copyright (c) 2016-7 Google Inc.
 * Barret Rhoden <brho@cs.berkeley.edu>
 * See LICENSE for details.
 *
 * Usage: turbo [reset]
 *
 * This will print the turbo ratio since the last reset for each core.
 *
 * x86 only (TODO) */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <parlib/sysinfo.h>
#include <parlib/arch/arch.h>
#include <ros/arch/msr-index.h>

static void check_for_msrs(void)
{
	uint32_t ecx;

	parlib_cpuid(0x6, 0, NULL, NULL, &ecx, NULL);
	if (!(ecx & (1 << 0))) {
		fprintf(stderr, "cpuid says no MPERF and APERF\n");
		exit(-1);
	}
}

static void reset_msrs(int fd)
{
	uint64_t reset_val = 0;
	ssize_t ret;

	ret = pwrite(fd, &reset_val, sizeof(reset_val), MSR_IA32_MPERF);
	if (ret < 0) {
		perror("pwrite mperf");
		exit(-1);
	}
	ret = pwrite(fd, &reset_val, sizeof(reset_val), MSR_IA32_APERF);
	if (ret < 0) {
		perror("pwrite aperf");
		exit(-1);
	}
}

int main(int argc, char **argv)
{
	uint64_t *mperf_buf, *aperf_buf;
	size_t buf_sz;
	ssize_t ret;
	int fd;
	int num_cores = get_num_pcores();
	bool reset = FALSE;

	check_for_msrs();

	if (argc == 2) {
		if (strcmp("reset", argv[1])) {
			printf("Usage: %s [reset]\n", argv[0]);
			exit(-1);
		}
		reset = TRUE;
	}

	fd = open("#arch/msr", O_RDWR);
	if (fd < 0) {
		perror("open");
		exit(-1);
	}
	if (reset) {
		reset_msrs(fd);
		exit(0);
	}

	buf_sz = num_cores * sizeof(uint64_t);
	mperf_buf = malloc(buf_sz);
	aperf_buf = malloc(buf_sz);
	assert(mperf_buf && aperf_buf);
	/* ideally these reads happen with no interference/interrupts in between */
	ret = pread(fd, mperf_buf, buf_sz, MSR_IA32_MPERF);
	if (ret < 0) {
		perror("pread mperf");
		exit(-1);
	}
	ret = pread(fd, aperf_buf, buf_sz, MSR_IA32_APERF);
	if (ret < 0) {
		perror("pread aperf");
		exit(-1);
	}
	for (int i = 0; i < num_cores; i++)
		printf("Core %3d: %4f%\n", i, 100.0 * aperf_buf[i] / mperf_buf[i]);
	return 0;
}
