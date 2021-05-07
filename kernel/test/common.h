#ifndef COMMON
#define COMMON

// TEST
// #define HEAP_XFT 24
#define HEAP_XFT 29
// #define HEAP_XFT 14
#define HEAP_SIZE (1 << HEAP_XFT)

#define USED(num) (0x66660000 | num)

//
#define MAX_CPU 8

//
#include <kernel.h>
#include <threads.h>

//
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#endif