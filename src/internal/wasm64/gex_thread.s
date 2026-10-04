# wasm64 (gex): per-thread state as module globals. gex instantiates the
# program's module once per thread, so a module global is a thread's own.
#
#   __gex_ctx   the thread's gex register context (syscalls go through it)
#   __gex_tp    the thread pointer (struct pthread *)
#
# and the two entries gex binds in its function table, both with gex's
# F_body signature (ctx, depth, need, ret) -> i32, neither returning:
#
#   __gex_start          the image's entry (AT_ENTRY): ESP points at argc
#   __gex_thread_start   AT_ENTRY + 16: a clone's child, ESP points at
#                        (func, arg, tls) left by __clone

	.globaltype __stack_pointer, i64

	.globaltype __tls_base, i64

	.globl __gex_set_tls_base
	.type __gex_set_tls_base,@function
__gex_set_tls_base:
	.functype __gex_set_tls_base (i64) -> ()
	local.get 0
	global.set __tls_base
	end_function

	.globaltype __gex_ctx, i64
__gex_ctx:
	.globaltype __gex_tp, i64
__gex_tp:

	.globl __gex_get_ctx
	.type __gex_get_ctx,@function
__gex_get_ctx:
	.functype __gex_get_ctx () -> (i64)
	global.get __gex_ctx
	end_function

	.globl __gex_get_tp
	.type __gex_get_tp,@function
__gex_get_tp:
	.functype __gex_get_tp () -> (i64)
	global.get __gex_tp
	end_function

	.globl __gex_set_tp
	.type __gex_set_tp,@function
__gex_set_tp:
	.functype __gex_set_tp (i64) -> ()
	local.get 0
	global.set __gex_tp
	end_function

	.functype __gex_start_c (i64) -> ()
	.functype __gex_thread_c (i64) -> ()

	.globl __gex_start
	.type __gex_start,@function
__gex_start:
	.functype __gex_start (i32, i32, i32, i64) -> (i32)
	.local i64
	local.get 0
	i64.extend_i32_u
	global.set __gex_ctx
	# rsp: ctx slot x(4)
	local.get 0
	i64.extend_i32_u
	i64.load 32
	local.tee 4
	# the C stack starts below argc, 16-aligned
	i64.const 64
	i64.sub
	i64.const -16
	i64.and
	global.set __stack_pointer
	local.get 4
	call __gex_start_c
	unreachable
	end_function

	.globl __gex_thread_start
	.type __gex_thread_start,@function
__gex_thread_start:
	.functype __gex_thread_start (i32, i32, i32, i64) -> (i32)
	.local i64
	local.get 0
	i64.extend_i32_u
	global.set __gex_ctx
	local.get 0
	i64.extend_i32_u
	i64.load 32
	local.tee 4
	i64.const -16
	i64.and
	i64.const 16
	i64.sub
	global.set __stack_pointer
	local.get 4
	call __gex_thread_c
	unreachable
	end_function

# The C stack pointer, for the syscall stub (it publishes it as the guest
# ESP so a signal frame lands below everything live).
	.globl __gex_get_sp
	.type __gex_get_sp,@function
__gex_get_sp:
	.functype __gex_get_sp () -> (i64)
	global.get __stack_pointer
	end_function

# __gex_swap_sp(sp): make `sp` the C stack pointer, returning the old one
# (signal handlers run on a stack of their own; the call itself is C, in
# gex_signal_frames.c, since llvm-mc cannot assemble a table64 call_indirect).
	.globl __gex_swap_sp
	.type __gex_swap_sp,@function
__gex_swap_sp:
	.functype __gex_swap_sp (i64) -> (i64)
	global.get __stack_pointer
	local.get 0
	global.set __stack_pointer
	end_function

# The provider form (docs/unity-dotnet.md): gex enters an export's adapter
# with the calling guest thread's context; the adapter records it here.
	.globl __gex_set_ctx
	.type __gex_set_ctx,@function
__gex_set_ctx:
	.functype __gex_set_ctx (i64) -> ()
	local.get 0
	global.set __gex_ctx
	end_function
