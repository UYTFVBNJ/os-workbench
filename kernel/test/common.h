#ifndef COMMON
#define COMMON

//
// #define HEAP_XFT 24
#define HEAP_XFT 29
#define HEAP_SIZE (1 << HEAP_XFT)

#define USED(num) (0x66660000 | num)

//
#include <kernel.h>

//
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#endif