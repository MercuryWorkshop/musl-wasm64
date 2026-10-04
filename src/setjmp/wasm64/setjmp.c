/* wasm64: setjmp/longjmp need wasm exception handling (clang's
 * -mllvm -wasm-enable-sjlj lowers them onto __wasm_setjmp/__wasm_longjmp).
 * Until gex's runtimes have it (wasmtime 27 natively does not), setjmp
 * returns 0 and longjmp stops the program. */
#include <setjmp.h>
#include <stdlib.h>

int setjmp(jmp_buf env) { return 0; }
int _setjmp(jmp_buf env) { return 0; }
_Noreturn void longjmp(jmp_buf env, int val) { abort(); }
_Noreturn void _longjmp(jmp_buf env, int val) { abort(); }
