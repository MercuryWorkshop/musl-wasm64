/* wasm64 has no ELF dynamic linker; dlopen is the host's (gex). */
#define LDSO_ARCH "wasm64"
#define CRTJMP(pc,sp) __builtin_trap()
#define GETFUNCSYM(fp, sym, got) (*(fp) = (void *)sym)
