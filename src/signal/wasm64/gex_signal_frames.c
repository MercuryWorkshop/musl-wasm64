/* wasm64 (gex): a signal handled at a syscall's return. gex-kernel's x86-64
 * personality leaves an x86-64 signal frame on the guest stack and PC at the
 * handler, which is guest (x86-64) code. Running it needs the caller's view
 * of gex's function table, so a provider (docs/unity-dotnet.md) supplies
 * the real loop; this default stops loudly. */
#include "syscall.h"

static void no_frames(volatile unsigned long long *x, unsigned long long pc)
{
	static const char m[] = "musl wasm64: a signal handler at a syscall's return, and no __gex_signal_frames to run it\n";
	x[32] = pc;
	__syscall(SYS_write, 2, m, sizeof m - 1);
	__builtin_trap();
}
weak_alias(no_frames, __gex_signal_frames);
