/* wasm64 (gex): the program's entry. gex enters __gex_start (gex_thread.s),
 * which sets up the thread's globals and C stack and calls this with the
 * initial stack pointer, as any _start_c gets it. */
#include <features.h>
#include "libc.h"

/* clang renames main on wasm: main(void) is __original_main and
 * main(int, char **) is __main_argc_argv. */
int __original_main(void);
weak int __main_argc_argv(int argc, char **argv) { return __original_main(); }
static int main_shim(int argc, char **argv, char **envp) { return __main_argc_argv(argc, argv); }
weak void _init();
weak void _fini();
int __libc_start_main(int (*)(), int, char **,
	void (*)(), void(*)(), void(*)());

hidden void __gex_start_c(long *p)
{
	int argc = p[0];
	char **argv = (void *)(p+1);
	__libc_start_main((int (*)())main_shim, argc, argv, _init, _fini, 0);
}

/* Both entries in the module's table, where gex finds them by name. */
int __gex_start(int, int, int, long long);
int __gex_thread_start(int, int, int, long long);
__attribute__((used)) void *const __gex_entries[] = { __gex_start, __gex_thread_start };
