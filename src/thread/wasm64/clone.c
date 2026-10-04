/* wasm64 (gex): a clone starts the child at a fixed guest address that gex
 * binds to __gex_thread_start: a program's AT_ENTRY + 16, or a provider's
 * own (__gex_thread_entry). The child's function, argument and thread pointer
 * travel on top of its stack; the entry takes them from there.
 *
 * A clone without CLONE_THREAD (posix_spawn's CLONE_VM|CLONE_VFORK, or a
 * fork) is a new process under gex: a copy of the address space whose
 * thread starts at the same entry. */
#define _GNU_SOURCE
#include <stdarg.h>
#include <sched.h>
#include <sys/auxv.h>
#include "pthread_impl.h"

unsigned long __gex_get_ctx(void);

/* Where the child starts: overridden by a provider, whose thread entry is
 * not the program's. */
static unsigned long program_thread_entry(void) { return getauxval(AT_ENTRY) + 16; }
weak_alias(program_thread_entry, __gex_thread_entry);

int __clone(int (*func)(void *), void *stack, int flags, void *arg, ...)
{
	va_list ap;
	pid_t *ptid, *ctid;
	void *tls;

	va_start(ap, arg);
	ptid = va_arg(ap, pid_t *);
	tls = va_arg(ap, void *);
	ctid = va_arg(ap, pid_t *);
	va_end(ap);

	unsigned long *sp = (unsigned long *)((uintptr_t)stack & -16);
	sp -= 4;
	sp[0] = (unsigned long)func;
	sp[1] = (unsigned long)arg;
	/* The child's module instance starts with fresh globals: it gets a
	 * thread pointer either way, and a fresh TLS block only for
	 * CLONE_SETTLS (otherwise it shares the caller's, as on Linux). */
	sp[2] = (flags & CLONE_SETTLS) ? (unsigned long)tls : (unsigned long)__get_tp();
	sp[3] = !!(flags & CLONE_SETTLS);

	/* The thread pointer is the entry's to set, so the kernel gets none. */
	flags &= ~CLONE_SETTLS;

	volatile unsigned long long *x = (volatile unsigned long long *)__gex_get_ctx();
	unsigned long long pc_save = x[32];
	x[32] = __gex_thread_entry();
	/* x86-64 order: flags, stack, parent tid, child tid, tls. */
	long r = __syscall(SYS_clone, flags, sp, ptid, ctid, 0);
	x[32] = pc_save;
	return __syscall_ret(r);
}
