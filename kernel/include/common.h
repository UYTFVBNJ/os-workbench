#ifndef COMMON
#define COMMON

#include <kernel.h>
#include <klib-macros.h>
#include <klib.h>

#define TRACE_F
// #define L1_TEST
// #define L2_TEST
// #define TEST_LOG

#define MAX_CPU 16

#ifdef TRACE_F
  #define TRACE_ENTRY printf("[trace] %s:entry\n", __func__)
  #define TRACE_EXIT  printf("[trace] %s:exit\n", __func__)
  #define TRACE_EXIT_with(ret)                  \
    printf("[trace] %s:exit\n", __func__); \
    return ret
#else
  #define TRACE_ENTRY ((void)0)
  #define TRACE_EXIT  ((void)0)
  #define TRACE_EXIT(ret) return ret
#endif

#endif