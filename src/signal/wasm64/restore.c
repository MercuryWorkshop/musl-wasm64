/* wasm64: signal handlers are not delivered asynchronously under gex
 * (there is no way to interrupt wasm code); a handler returns to whoever
 * called it, so no sigreturn trampoline is needed. */
void __restore(void) { __builtin_trap(); }
void __restore_rt(void) { __builtin_trap(); }
