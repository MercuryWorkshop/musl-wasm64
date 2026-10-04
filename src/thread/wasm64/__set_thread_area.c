#include <stdint.h>
void __gex_set_tp(uintptr_t);

int __set_thread_area(void *p)
{
	__gex_set_tp((uintptr_t)p);
	return 0;
}
