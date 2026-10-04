/* wasm64 (gex): thread-local storage is wasm-ld's. Each thread's module
 * instance has its own __tls_base global; __wasm_init_tls(block) copies the
 * .tdata image into the block and points __tls_base at it. musl only lays
 * the block out beside struct pthread; the thread itself initialises it
 * (wasm32 __clone's entry for new threads, static_init_tls for the main
 * one). td->dtv holds the block's address (no dynamic TLS: tls_cnt is 0). */
#define SYSCALL_NO_TLS 1
#include <limits.h>
#include <sys/mman.h>
#include <string.h>
#include <stddef.h>
#include "pthread_impl.h"
#include "libc.h"
#include "atomic.h"
#include "syscall.h"

void __wasm_init_tls(void *);

volatile int __thread_list_lock;

int __init_tp(void *p)
{
	pthread_t td = p;
	td->self = td;
	int r = __set_thread_area(TP_ADJ(p));
	if (r < 0) return -1;
	if (!r) libc.can_do_threads = 1;
	td->detach_state = DT_JOINABLE;
	td->tid = __syscall(SYS_set_tid_address, &__thread_list_lock);
	td->locale = &libc.global_locale;
	td->robust_list.head = &td->robust_list.head;
	td->sysinfo = __sysinfo;
	td->next = td->prev = td;
	return 0;
}

static struct builtin_tls {
	char c;
	struct pthread pt;
	void *space[64];
} builtin_tls[1];
#define MIN_TLS_ALIGN offsetof(struct builtin_tls, pt)

static size_t tls_block_size, tls_block_align;

void *__copy_tls(unsigned char *mem)
{
	pthread_t td;
	unsigned char *block;

	mem += -(uintptr_t)mem & (libc.tls_align-1);
	td = (pthread_t)mem;
	block = mem + sizeof(struct pthread);
	block += -(uintptr_t)block & (tls_block_align-1);
	td->dtv = (uintptr_t *)block;
	return td;
}

static void static_init_tls(size_t *aux)
{
	void *mem;

	tls_block_size = __builtin_wasm_tls_size();
	tls_block_align = __builtin_wasm_tls_align();
	if (tls_block_align < sizeof(void *)) tls_block_align = sizeof(void *);

	libc.tls_align = tls_block_align < MIN_TLS_ALIGN ? MIN_TLS_ALIGN : tls_block_align;
	libc.tls_cnt = 0;
	libc.tls_head = 0;
	libc.tls_size = sizeof(struct pthread) + tls_block_size
		+ tls_block_align + libc.tls_align
		+ MIN_TLS_ALIGN-1 & -MIN_TLS_ALIGN;

	if (libc.tls_size > sizeof builtin_tls) {
		mem = (void *)__syscall(
			SYS_mmap,
			0, libc.tls_size, PROT_READ|PROT_WRITE,
			MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	} else {
		mem = builtin_tls;
	}

	pthread_t td = __copy_tls(mem);
	if (__init_tp(td) < 0)
		a_crash();
	__wasm_init_tls(td->dtv);
}

weak_alias(static_init_tls, __init_tls);
