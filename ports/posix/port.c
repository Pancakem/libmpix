/* SPDX-License-Identifier: Apache-2.0 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <mpix/port.h>

uint32_t mpix_port_get_uptime_us(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000 * 1000 + ts.tv_nsec / 1000;
}

void *mpix_port_alloc(size_t size, enum mpix_mem_source mem_source)
{
	(void)mem_source;
	return malloc(size);
}

void mpix_port_free(void *mem, enum mpix_mem_source mem_source)
{
	(void)mem_source;
	free(mem);
}

void mpix_port_printf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

#if CONFIG_MPIX_BENCHMARK_MEMORY
uint32_t mpix_get_free_stack(void)
{
	int ret;
	struct k_thread *current_thread = k_current_get();
	size_t free_stack;

	ret = k_thread_stack_space_get(current_thread, &free_stack);
	if (ret < 0) {
		mpix_port_printf("failed to get free stack\r\n");
	} else {
		mpix_port_printf("stack | free: %u bytes\r\n", free_stack);
	}

	return free_stack;
}

uint32_t mpix_get_heap_usage()
{
	int ret;
	struct sys_memory_stats heap_stats;
	
	ret = sys_heap_runtime_stats_get(mpix_heap, &heap_stats);
	if (ret < 0) {
		mpix_port_printf("failed to get heap stats\r\n");
	} else {
		mpix_port_printf("heap | free: %u bytes, allocated: %u bytes\r\n",
				 heap_stats.free_bytes, heap_stats.allocated_bytes);
	}
}
#endif