/* wasm32: setjmp/longjmp are lowered by clang (-mllvm -wasm-enable-sjlj)
 * onto wasm exception handling; the buffer holds the runtime's state. */
typedef unsigned long __jmp_buf[8];
