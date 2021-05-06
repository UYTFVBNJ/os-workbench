#include <spinlock.h>

#define BUDDY_UNIT_SHIFT 9
#define BUDDY_UNIT_SIZE (1 << BUDDY_UNIT_SHIFT)