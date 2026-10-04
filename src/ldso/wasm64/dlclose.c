/* wasm64 (gex): nothing is loaded, so nothing is unloaded. */
#include <dlfcn.h>

int dlclose(void *p)
{
	return 0;
}
