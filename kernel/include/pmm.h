#include <spinlock.h>

#ifdef TEST
#include <pmm_test.h>
#endif

#define BUDDY_UNIT_SHIFT 12
#define BUDDY_UNIT_SIZE (1 << BUDDY_UNIT_SHIFT)