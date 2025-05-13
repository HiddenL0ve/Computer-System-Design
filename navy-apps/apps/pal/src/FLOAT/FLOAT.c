#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  //assert(0);
  //return 0;
  return (a * b) >> 16;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
    FLOAT dividend = Fabs(a);
    FLOAT divisor = Fabs(b);
    // 执行浮点数的除法操作，得到初始结果
    FLOAT res = dividend / divisor;
    dividend = dividend % divisor;

    // 长除法
    for (int i = 0; i < 16; i++) {
        dividend <<= 1;  // 将被除数左移1位
        res <<= 1;  // 将结果左移1位
        // 如果被除数大于等于除数
        if (dividend >= divisor) { 
            dividend -= divisor;
            res++;
        }
    }
    // 如果a和b的符号不同
    if (((a ^ b) & 0x80000000) == 0x80000000) {
        res = -res;
    }
    return res;
}

  union float_ {
    struct {
      uint32_t sign : 1;
      uint32_t exp : 8;
      uint32_t man : 23;
    };
    uint32_t val;
  };

FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */

  union float_ f;
  f.val = *((uint32_t*)(void*)&a);

  int exp = f.exp - 127;
  FLOAT result = 0;
  int mov = 7 - exp;
  if (mov >= 0)
    result = (f.man | (1 << 23)) >> mov;
  else
    result = (f.man | (1 << 23)) << (-mov);

  return f.sign == 0 ? result : -result;
}

FLOAT Fabs(FLOAT a) {
  return (a > 0) ? a : -a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}
