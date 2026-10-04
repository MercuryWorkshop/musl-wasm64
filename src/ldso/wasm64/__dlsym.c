/* wasm64 (gex): dlsym over the program's registry (see dlopen.c). */
#include <dlfcn.h>
#include <string.h>
#include "dynlink.h"

struct __gex_dl_sym { const char *name; void *addr; };
struct __gex_dl_lib { const char *name; const struct __gex_dl_sym *syms; };
extern const struct __gex_dl_lib *const __gex_dl_libs[];
extern hidden const struct __gex_dl_lib __gex_dl_self;

static void *find(const struct __gex_dl_lib *l, const char *s)
{
	for (const struct __gex_dl_sym *y = l->syms; y && y->name; y++)
		if (!strcmp(y->name, s)) return y->addr;
	return 0;
}

hidden void *__dlsym(void *restrict p, const char *restrict s, void *restrict ra)
{
	if (p && p != RTLD_DEFAULT && p != (void *)&__gex_dl_self) {
		void *a = find(p, s);
		if (a) return a;
	} else {
		for (const struct __gex_dl_lib *const *l = __gex_dl_libs; *l; l++) {
			void *a = find(*l, s);
			if (a) return a;
		}
	}
	__dl_seterr("Symbol not found: %s", s);
	return 0;
}

