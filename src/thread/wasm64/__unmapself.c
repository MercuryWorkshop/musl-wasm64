#include "pthread_impl.h"

/* No stack is used after the munmap: the syscall path writes only the
 * thread's register context. */
_Noreturn void __unmapself(void *base, size_t size)
{
	__syscall(SYS_munmap, base, size);
	for (;;) __syscall(SYS_exit, 0);
}
