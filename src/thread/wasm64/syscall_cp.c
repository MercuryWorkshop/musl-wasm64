/* wasm64: cancellation points without asynchronous cancellation (a
 * cancel request is seen before the syscall, never during it). */
#include "pthread_impl.h"

hidden long __cancel(void);

hidden long __syscall_cp_asm(volatile int *cancel, syscall_arg_t nr,
	syscall_arg_t u, syscall_arg_t v, syscall_arg_t w,
	syscall_arg_t x, syscall_arg_t y, syscall_arg_t z)
{
	if (*cancel) return __cancel();
	return __syscall(nr, u, v, w, x, y, z);
}
