/* SPDX-License-Identifier: Apache-2.0 */

#include <stdarg.h>
#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/video.h>
#include <zephyr/drivers/video-controls.h>
#include <zephyr/sys/heap_listener.h>

#include <mpix/port.h>

K_HEAP_DEFINE(mpix_heap, CONFIG_MPIX_HEAP_SIZE);

void *mpix_port_alloc(size_t size, enum mpix_mem_source mem_source)
{
	(void)mem_source;
	return k_heap_alloc(&mpix_heap, size, K_NO_WAIT);
}

void mpix_port_free(void *mem, enum mpix_mem_source mem_source)
{
	(void)mem_source;
	return k_heap_free(&mpix_heap, mem);
}

uint32_t mpix_port_get_uptime_us(void)
{
	return k_cycle_get_64() * 1000 * 1000 / sys_clock_hw_cycles_per_sec();
}

void mpix_port_printf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

#if CONFIG_MPIX_BENCHMARK_MEMORY
struct mpix_benchmark_entry *benchmark_entry = NULL;

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

static void mpix_on_heap_alloc(uintptr_t heap_id, void *mem, size_t bytes)
{
	(void)mem;
	(void)heap_id;
	benchmark_entry->heap_bytes += bytes;
}

HEAP_LISTENER_ALLOC_DEFINE(mpix_heap_alloc_listener, HEAP_ID_FROM_POINTER(&mpix_heap),
			   mpix_on_heap_alloc);

static void mpix_on_heap_free(uintptr_t heap_id, void *mem, size_t bytes)
{
	(void)mem;
	(void)heap_id;
}

HEAP_LISTENER_FREE_DEFINE(mpix_heap_free_listener, HEAP_ID_FROM_POINTER(&mpix_heap),
			  mpix_on_heap_free);

void mpix_start_heap_track(struct mpix_benchmark_entry *entry)
{
	heap_listener_register(&mpix_heap_alloc_listener);
	benchmark_entry = entry;
}

void mpix_end_heap_track(void)
{
	heap_listener_unregister(&mpix_heap_alloc_listener);
	benchmark_entry = NULL;
}
#endif
