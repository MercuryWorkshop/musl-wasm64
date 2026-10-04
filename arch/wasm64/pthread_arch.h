/* wasm64: the thread pointer is a wasm global of the module instance
 * (one instance per thread under gex), set by the thread's entry. */
uintptr_t __gex_get_tp(void);
static inline uintptr_t __get_tp()
{
	return __gex_get_tp();
}

#define TLS_ABOVE_TP
#define GAP_ABOVE_TP 0
#define MC_PC gregs[REG_RIP]
