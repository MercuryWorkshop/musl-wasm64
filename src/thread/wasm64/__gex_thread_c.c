#include "pthread_impl.h"

void __wasm_init_tls(void *);
void __gex_set_tp(uintptr_t);
void __gex_set_tls_base(void *);

/* A provider's thread runs on the C stack gex gives each guest thread, not
 * the one pthread_create mapped (src/env/wasm64/__gex_provider.c), so that
 * is the stack pthread_getattr_np reports. */
static int no_stack(unsigned long *lo, unsigned long *size) { return -1; }
weak_alias(no_stack, __gex_thread_stack);

/* A clone's child (see wasm32 __clone and gex_thread.s). */
_Noreturn hidden void __gex_thread_c(unsigned long *sp)
{
	int (*func)(void *) = (int (*)(void *))sp[0];
	void *arg = (void *)sp[1];
	/* sp[2] is the thread pointer (TP_ADJ of the thread's struct). */
	__gex_set_tp(sp[2]);
	pthread_t td = __pthread_self();
	unsigned long lo, size;
	if (__gex_thread_stack(&lo, &size) == 0) {
		td->stack = (void *)(lo + size);
		td->stack_size = size;
	}
	if (sp[3]) __wasm_init_tls(td->dtv);
	else __gex_set_tls_base(td->dtv);
	int r = func(arg);
	for (;;) __syscall(SYS_exit, r);
}
