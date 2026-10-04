/* wasm64 under gex: a syscall is the x86-64 Linux ABI, issued through gex's
 * `gex.syscall(ctx, pc)` import. The number and the arguments go into the
 * calling thread's register context (ctx slots are 8 bytes, numbered as x86
 * encodes registers: rax 0, rcx 1, rdx 2, rbx 3, rsp 4, rbp 5, rsi 6, rdi 7,
 * r8..r15 8..15; PC is slot 32) and the result comes back in rax.
 *
 * The context may be an x86-64 guest thread's own (a provider running in
 * place of a guest library: docs/unity-dotnet.md), whose registers the
 * guest still owns, so every slot a syscall touches is put back afterwards
 * except rax, which the provider's return overwrites anyway. */

#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)

unsigned long __gex_get_ctx(void);
__attribute__((__import_module__("gex"), __import_name__("syscall")))
void __gex_syscall_import(int, long long);
void __gex_signal_frames(volatile unsigned long long *, unsigned long long);

/* brk is the guest program's (a provider sharing its process must not
 * move the program break under the guest's malloc): it fails, and musl's
 * malloc takes mmap instead. */
#define __GEX_SYS_BRK 12

static inline long __gex_sc(long n, long a, long b, long c, long d, long e, long f)
{
	if (n == __GEX_SYS_BRK) return -12; /* ENOMEM */
	volatile unsigned long long *x = (volatile unsigned long long *)__gex_get_ctx();
	unsigned long long pc = x[32];
	unsigned long long s_rcx = x[1], s_rdx = x[2], s_rsi = x[6], s_rdi = x[7];
	unsigned long long s_r8 = x[8], s_r9 = x[9], s_r10 = x[10], s_r11 = x[11];
	x[0] = (unsigned long)n;
	x[7] = (unsigned long)a;
	x[6] = (unsigned long)b;
	x[2] = (unsigned long)c;
	x[10] = (unsigned long)d;
	x[8] = (unsigned long)e;
	x[9] = (unsigned long)f;
	__gex_syscall_import((int)(unsigned long)x, (long long)pc);
	if (__builtin_expect(x[32] != pc, 0))
		__gex_signal_frames(x, pc);
	long r = (long)x[0];
	x[1] = s_rcx; x[2] = s_rdx; x[6] = s_rsi; x[7] = s_rdi;
	x[8] = s_r8; x[9] = s_r9; x[10] = s_r10; x[11] = s_r11;
	return r;
}

static inline long __syscall0(long n)
{ return __gex_sc(n, 0, 0, 0, 0, 0, 0); }
static inline long __syscall1(long n, long a1)
{ return __gex_sc(n, a1, 0, 0, 0, 0, 0); }
static inline long __syscall2(long n, long a1, long a2)
{ return __gex_sc(n, a1, a2, 0, 0, 0, 0); }
static inline long __syscall3(long n, long a1, long a2, long a3)
{ return __gex_sc(n, a1, a2, a3, 0, 0, 0); }
static inline long __syscall4(long n, long a1, long a2, long a3, long a4)
{ return __gex_sc(n, a1, a2, a3, a4, 0, 0); }
static inline long __syscall5(long n, long a1, long a2, long a3, long a4, long a5)
{ return __gex_sc(n, a1, a2, a3, a4, a5, 0); }
static inline long __syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{ return __gex_sc(n, a1, a2, a3, a4, a5, a6); }

#define IPC_64 0
