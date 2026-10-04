/* wasm64 (gex): libc for a provider, a library of wasm that runs in place
 * of a guest library inside an x86-64 guest process (noita-wasm
 * docs/unity-dotnet.md). Nothing here owns the process: the guest's own
 * libc started it, its threads were created by the guest, and its
 * environment is on the guest's initial stack. So:
 *
 *   __gex_provider_init()    once per process, from whichever thread enters
 *                            first: libc's own state (TLS layout, page size,
 *                            the environment from /proc/self/environ), and
 *                            this thread attached;
 *   __gex_provider_attach()  once per guest thread (its module instance):
 *                            a struct pthread and a TLS block of its own.
 *
 * A thread the provider creates itself (pthread_create) arrives through
 * __gex_thread_start like a program's, already attached. */
#define SYSCALL_NO_TLS 1
#include <elf.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "pthread_impl.h"
#include "libc.h"
#include "syscall.h"
#include "atomic.h"

void __init_libc(char **, char *);
extern char **__environ;
void *__copy_tls(unsigned char *);
int __init_tp(void *);
void __wasm_init_tls(void *);

static volatile int init_state; /* 0 none, 1 running, 2 done */

/* Read /proc/self/environ into a fresh environment (never freed). */
static void load_environ(void)
{
	int fd = __sys_open("/proc/self/environ", O_RDONLY | O_CLOEXEC);
	if (fd < 0) return;
	size_t cap = 1 << 16, len = 0;
	char *buf = malloc(cap);
	for (;;) {
		if (!buf) { __syscall(SYS_close, fd); return; }
		long n = __syscall(SYS_read, fd, buf + len, cap - len - 1);
		if (n <= 0) break;
		len += n;
		if (len + 1 >= cap) buf = realloc(buf, cap *= 2);
	}
	__syscall(SYS_close, fd);
	buf[len] = 0;
	size_t count = 0;
	for (size_t i = 0; i < len; i += strlen(buf + i) + 1) count++;
	char **env = malloc((count + 1) * sizeof *env);
	if (!env) return;
	size_t k = 0;
	for (size_t i = 0; i < len; i += strlen(buf + i) + 1) env[k++] = buf + i;
	env[k] = 0;
	__environ = env;
}

void __gex_provider_init(void)
{
	if (a_cas(&init_state, 0, 1) != 0) {
		while (init_state != 2) a_spin();
		return;
	}
	/* An empty environment and the auxiliary values libc needs; this
	 * thread gets the first TLS block (static_init_tls). */
	static size_t fake[] = { 0, AT_PAGESZ, 4096, AT_UID, 0, AT_EUID, 0, AT_GID, 0, AT_EGID, 0, 0, 0 };
	__init_libc((char **)fake, "gex-provider");
	load_environ();
	a_store(&init_state, 2);
}

void __gex_provider_attach(void)
{
	if (init_state != 2) {
		__gex_provider_init();
		if (__get_tp()) return; /* this thread did the init */
	}
	void *mem = (void *)__syscall(SYS_mmap, 0, libc.tls_size, PROT_READ|PROT_WRITE,
		MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	if ((unsigned long)mem > -4096UL) a_crash();
	pthread_t td = __copy_tls(mem);
	td->self = td;
	__set_thread_area(TP_ADJ(td));
	td->detach_state = DT_DETACHED;
	td->tid = __syscall(SYS_gettid);
	td->locale = &libc.global_locale;
	td->robust_list.head = &td->robust_list.head;
	td->sysinfo = __sysinfo;
	td->next = td->prev = td;
	td->stack = 0;
	td->stack_size = 0;
	__wasm_init_tls(td->dtv);
}
