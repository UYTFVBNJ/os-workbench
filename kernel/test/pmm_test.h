// #define HEAP_XFT 24
#define HEAP_XFT 29
// #define HEAP_XFT 14
#define HEAP_SIZE (1 << HEAP_XFT)
#define USED(num) (0x66660000 | num)

void pmm_test_paint(void *addr, size_t size, char key);
void pmm_test_check(void *addr, size_t size, char key);