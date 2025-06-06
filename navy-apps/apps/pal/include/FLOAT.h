#ifndef __FLOAT_H__
#define __FLOAT_H__

#include "assert.h"

typedef int FLOAT;

static inline int F2int(FLOAT a) {
  return a >> 16;
}

static inline FLOAT int2F(int a) {
  return a << 16;
}

FLOAT F_mul_F(FLOAT, FLOAT);
FLOAT F_div_F(FLOAT, FLOAT);

static inline FLOAT F_mul_int(FLOAT a, int b) {
  return F_mul_F(a, int2F(b));
}

static inline FLOAT F_div_int(FLOAT a, int b) {
  return F_div_F(a, int2F(b));
}

FLOAT f2F(float);
FLOAT Fabs(FLOAT);
FLOAT Fsqrt(FLOAT);
FLOAT Fpow(FLOAT, FLOAT);

#endif

