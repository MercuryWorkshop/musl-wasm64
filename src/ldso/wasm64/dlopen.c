/* wasm64 (gex): dlopen over a registry linked into the program.
 *
 * gex runs a closed world (docs/wine-native.md §3.12): every library a
 * program will dlopen -- Wine's Unix libraries -- is linked into it at
 * build time, each with its exported symbols renamed apart, and the link
 * adds a registry (tools/wine-wasm/dlreg.py) naming each library and its
 * symbols. dlopen finds a library by its file name (any directory), dlsym
 * finds a symbol in it, or in every library for RTLD_DEFAULT. Nothing is
 * loaded at run time. */
#include <dlfcn.h>
#include <string.h>
#include "dynlink.h"

struct __gex_dl_sym { const char *name; void *addr; };
struct __gex_dl_lib { const char *name; const struct __gex_dl_sym *syms; };

/* The program's registry, NULL-terminated; empty unless one is linked. */
static const struct __gex_dl_lib *const empty[] = { 0 };
weak_alias(empty, __gex_dl_libs);
extern const struct __gex_dl_lib *const __gex_dl_libs[];

/* The handle dlopen(NULL) gives: every library. */
hidden const struct __gex_dl_lib __gex_dl_self = { "", 0 };

static const char *base(const char *p)
{
	const char *s = strrchr(p, '/');
	return s ? s + 1 : p;
}

void *dlopen(const char *file, int mode)
{
	if (!file) return (void *)&__gex_dl_self;
	const char *b = base(file);
	for (const struct __gex_dl_lib *const *l = __gex_dl_libs; *l; l++)
		if (!strcmp((*l)->name, b)) return (void *)*l;
	__dl_seterr("%s: not linked into this program", file);
	return 0;
}

