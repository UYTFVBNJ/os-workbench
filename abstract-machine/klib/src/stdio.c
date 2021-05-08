#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

const char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

#define M__vsprintf()                                            \
  size_t fmtp = 0, outp = 0, bufp = 0;                           \
  char buf[25];                                                  \
                                                                 \
  char v_char;                                                   \
  int v_int;                                                     \
  uint32_t v_uint32_t;                                           \
  uint32_t mask;                                                 \
  char *v_str;                                                   \
                                                                 \
  for (fmtp = 0; fmt[fmtp] != '\0'; fmtp++) switch (fmt[fmtp]) { \
      case '%':                                                  \
        fmtp++;                                                  \
        switch (fmt[fmtp]) {                                     \
          case 'c':                                              \
            v_char = va_arg(ap, int);                            \
            opt(v_char);                                         \
            break;                                               \
          case 'd':                                              \
            v_int = va_arg(ap, int);                             \
            bufp = 0;                                            \
                                                                 \
            if (v_int < 0) {                                     \
              v_int = -v_int;                                    \
              opt('-');                                          \
            }                                                    \
                                                                 \
            if (v_int == 0) {                                    \
              opt('0');                                          \
            }                                                    \
                                                                 \
            while (v_int) {                                      \
              buf[bufp++] = v_int % 10 + '0';                    \
              v_int /= 10;                                       \
            }                                                    \
                                                                 \
            while (bufp) {                                       \
              bufp--;                                            \
              opt(buf[bufp]);                                    \
            }                                                    \
                                                                 \
            break;                                               \
          case 's':                                              \
            v_str = va_arg(ap, char *);                          \
            size_t i;                                            \
                                                                 \
            for (i = 0; v_str[i] != '\0'; i++) opt(v_str[i]);    \
                                                                 \
            break;                                               \
          case 'p':                                              \
            mask = 0xf0000000;                                   \
            v_uint32_t = va_arg(ap, uint32_t);                   \
                                                                 \
            opt('0');                                            \
            opt('x');                                            \
                                                                 \
            for (int i = 7; i >= 0; i--, mask >>= 4) {           \
              opt(hex[(v_uint32_t & mask) >> (i * 4)]);          \
            }                                                    \
                                                                 \
            break;                                               \
          default:                                               \
            printf("Not implemented!");                          \
            return -1;                                           \
            break;                                               \
        }                                                        \
        break;                                                   \
                                                                 \
      case '\\':                                                 \
        fmtp++;                                                  \
        switch (fmt[fmtp]) {                                     \
          case 'n':                                              \
            opt('\n');                                           \
            break;                                               \
          default:                                               \
            printf("Not implemented!");                          \
            return -1;                                           \
            break;                                               \
        }                                                        \
        break;                                                   \
      default:                                                   \
        opt(fmt[fmtp]);                                          \
        break;                                                   \
    }                                                            \
                                                                 \
  // bufp--; opt(buf[bufp]) cannot be written as buf[--bufp] because of
  // vsnprintf uses if
int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

#define opt(arg) putch(arg)

  M__vsprintf();

#undef opt

  va_end(ap);

  return outp;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
#define opt(arg) out[outp++] = arg

  M__vsprintf() out[outp] = '\0';

#undef opt

  return outp;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list vl;
  va_start(vl, fmt);

  int outp = vsprintf(out, fmt, vl);

  va_end(vl);

  return outp;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list vl;
  va_start(vl, fmt);

  int outp = vsnprintf(out, n, fmt, vl);

  va_end(vl);

  return outp;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
#define opt(arg) \
  if (outp < n - 1) out[outp++] = arg

  M__vsprintf() out[outp] = '\0';

#undef opt

  return outp;
}

#endif
