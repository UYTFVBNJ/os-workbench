#include <spinlock.h>

#define BUDDY_UNIT_SHIFT 12
#define BUDDY_UNIT_SIZE (1 << BUDDY_UNIT_SHIFT)