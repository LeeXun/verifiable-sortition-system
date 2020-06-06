/**
Copyright (C) 2019 Markku Pulkkinen

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
**/

#ifndef REDUCER_H
#define REDUCER_H

#include <algorithm>
#include <array>
#include <cmath>

#include "ClassGroup.h"

extern "C" {
int has_lzcnt_hard();
unsigned int lzcnt64_soft(unsigned long long x);
unsigned int lzcnt64_hard(unsigned long long x);
}

/** constants utilized in reduction algorithm */
namespace {
const int_fast64_t THRESH{1ul << 31};
const int_fast64_t EXP_THRESH{31};
} 

/**
 * @brief The Reducer class that does custom reduce operation for VDF 
 * repeated squaring algorithm. The implementation is based on
 * Akashnil VDF competition entry and further optimized for speed.
 */
class alignas(64) Reducer {
public:
  /**
   * @brief Reducer - constructs by using reference into cg context.
   */  
  Reducer(ClassGroupContext &ctx_) : ctx(ctx_) {}

  ~Reducer() {}

  /**
   * @brief run - runs reduction algorithm for cg context params
   */  
  inline void run() {
    while (!isReduced()) {
      int_fast64_t a, b, c;
      {
        int_fast64_t a_exp, b_exp, c_exp;
        mpz_get_si_2exp(a, a_exp, ctx.a);
        mpz_get_si_2exp(b, b_exp, ctx.b);
        mpz_get_si_2exp(c, c_exp, ctx.c);
        auto mm = std::minmax({a_exp, b_exp, c_exp});
        if (mm.second - mm.first > EXP_THRESH) {
          reducer();
          continue;
        }
        // Ensure a, b, c are shifted so that a : b : c ratios are same as
        // f.a : f.b : f.c. a, b, c will be used as approximations to f.a,
        // f.b, f.c
        int_fast64_t max_exp(++mm.second); // for safety vs overflow
        a >>= (max_exp - a_exp);
        b >>= (max_exp - b_exp);
        c >>= (max_exp - c_exp);
      }
      {
        int_fast64_t u, v, w, x;
        calc_uvwx(u, v, w, x, a, b, c);

        mpz_mul_si(ctx.faa, ctx.a, u * u);
        mpz_mul_si(ctx.fab, ctx.b, u * w);
        mpz_mul_si(ctx.fac, ctx.c, w * w);

        mpz_mul_si(ctx.fba, ctx.a, u * v << 1);
        mpz_mul_si(ctx.fbb, ctx.b, u * x + v * w);
        mpz_mul_si(ctx.fbc, ctx.c, w * x << 1);

        mpz_mul_si(ctx.fca, ctx.a, v * v);
        mpz_mul_si(ctx.fcb, ctx.b, v * x);
        mpz_mul_si(ctx.fcc, ctx.c, x * x);

        mpz_add(ctx.a, ctx.faa, ctx.fab);
        mpz_add(ctx.a, ctx.a, ctx.fac);

        mpz_add(ctx.b, ctx.fba, ctx.fbb);
        mpz_add(ctx.b, ctx.b, ctx.fbc);

        mpz_add(ctx.c, ctx.fca, ctx.fcb);
        mpz_add(ctx.c, ctx.c, ctx.fcc);
      }
    }
  }

private:

  inline void signed_shift(uint64_t op, int64_t shift, int_fast64_t &r) {
    if (shift > 0)
      r = static_cast<int64_t>(op << shift);
    else if (shift <= -64)
      r = 0;
    else
      r = static_cast<int64_t>(op >> (-shift));
  }

bool bLZCChecked=false;
bool bLZCHasHW=false;

  inline void mpz_get_si_2exp(int_fast64_t &r, int_fast64_t &exp,
                              const mpz_t op) {
    // Return an approximation x of the large mpz_t op by an int64_t and the
    // exponent e adjustment. We must have (x * 2^e) / op = constant
    // approximately.
    int_fast64_t size(static_cast<long>(mpz_size(op)));
    uint_fast64_t last(mpz_getlimbn(op, (size - 1)));

    if(!bLZCChecked)
    {
        bLZCHasHW=has_lzcnt_hard();
        bLZCChecked=true;
    }
    
    int_fast64_t lg2;

    if(bLZCHasHW)
        lg2 = exp = ((63 - lzcnt64_hard(last)) + 1);
    else
        lg2 = exp = ((63 - lzcnt64_soft(last)) + 1);

    signed_shift(last, (63 - exp), r);
    if (size > 1) {
      exp += (size - 1) * 64;
      uint_fast64_t prev(mpz_getlimbn(op, (size - 2)));
      int_fast64_t t;
      signed_shift(prev, -1 - lg2, t);
      r += t;
    }
    if (mpz_sgn(op) < 0)
      r = -r;
  }

  inline bool isReduced() {
    int a_b(mpz_cmpabs(ctx.a, ctx.b));
    int c_b(mpz_cmpabs(ctx.c, ctx.b));
    if (a_b < 0 || c_b < 0)
      return false;

    int a_c(mpz_cmp(ctx.a, ctx.c));
    if (a_c > 0) {
      mpz_swap(ctx.a, ctx.c);
      mpz_neg(ctx.b, ctx.b);
    } else if (a_c == 0 && mpz_sgn(ctx.b) < 0) {
      mpz_neg(ctx.b, ctx.b);
    }
    return true;
  }

  inline void reducer() {
    // (c + b)/2c == (1 + (b/c))/2 -> s
    mpz_mdiv(ctx.r, ctx.b, ctx.c);
    mpz_add_ui(ctx.r, ctx.r, 1);
    mpz_div_2exp(ctx.s, ctx.r, 1);
    // cs -> m
    mpz_mul(ctx.m, ctx.c, ctx.s);
    // 2cs -> r
    mpz_mul_2exp(ctx.r, ctx.m, 1);
    // (cs - b) -> m
    mpz_sub(ctx.m, ctx.m, ctx.b);

    // new b = -b + 2cs
    mpz_sub(ctx.b, ctx.r, ctx.b);
    // new a = c, c = a
    mpz_swap(ctx.a, ctx.c);
    // new c = c + cs^2 - bs ( == c + (s * ( cs - b)))
    mpz_addmul(ctx.c, ctx.s, ctx.m);
  }

  inline void calc_uvwx(int_fast64_t &u, int_fast64_t &v, int_fast64_t &w,
                        int_fast64_t &x, int_fast64_t &a, int_fast64_t &b,
                        int_fast64_t &c) {
    // We must be very careful about overflow in the following steps
    int below_threshold;
    int_fast64_t u_{1}, v_{0}, w_{0}, x_{1};
    int_fast64_t a_, b_, s;
    do {
      u = u_;
      v = v_;
      w = w_;
      x = x_;

      s = b >= 0 ? (b+c) / (c<<1) : - (-b+c) / (c<<1);

      a_ = a;
      b_ = b;
      // cs = c * s;

      // a = c
      a = c;
      // b = -b + 2cs
      b = -b + (c * s << 1);
      // c = a + cs^2 - bs
      c = a_ - s * (b_ - c * s);

      u_ = v;
      v_ = -u + s * v;
      w_ = x;
      x_ = -w + s * x;

      // The condition (abs(v_) | abs(x_)) <= THRESH protects against
      // overflow
      below_threshold = (abs(v_) | abs(x_)) <= THRESH ? 1 : 0;
    } while (below_threshold && a > c && c > 0);

    if (below_threshold) {
      u = u_;
      v = v_;
      w = w_;
      x = x_;
    }
  }

  ClassGroupContext &ctx;
};

/*
Count leading zero bits. Choice of public domain or MIT-0.

David Reid - mackron@gmail.com

The lzcnt32 and lzcnt64 functions count the number of leading zero bits in a given 32- or 64-bit variable. When the input variable is 0, the
total size in bits will be returned (32 for lzcnt32 and 64 for lzcnt64).

For x86/64 platforms, this will use the LZCNT instruction if available. On ARM it will try the CLZ instruction. If these are unavailable it
will fall back to compiler-specific built-ins. If these are unavailable it'll fall back to the generic implementation.

Functions
---------
lzcnt32_generic
lzcnt64_generic
    Generic implementation.

lzcnt32_msvc_bsr
lzcnt64_msvc_bsr
    MSVC built-in implementation using _BitScanReverse/64(). Note that lzcnt64_mscv_bsr() is only available when compiling as 64-bit.

lzcnt32_gcc_builtin
lzcnt64_gcc_builtin
    GCC/Clang built-in implementation using __builtin_clzl/l(). Note that lzcnt64_gcc_builtin() is only available when compiling as 64-bit.

lzcnt32_msvc_x86
lzcnt64_msvc_x64
    MSVC implementation using the __lzcnt and __lzcnt64 intrinsic. This should emit a LZCNT instruction. Note that these are only available
    when targeting x86/64. lzcnt64_msvc_x64() is only available when compiling as 64-bit.

lzcnt32_gcc_x86
lzcnt64_gcc_x64
    GCC/Clang inline assembly implementation. This will emit the LZCNT instruction. Note that these are only available when targeting x86/x64
    and when compiled using a compiler that supports GCC style inline assembly.

lzcnt32_gcc_arm
lzcnt64_gcc_arm
    GCC/Clang inline assembly implementation. This will emit the CLZ instruction. Note that these are only available when targeting ARM
    architecture version 5 and above and when compiled using a compiler that supports GCC style inline assembly.

lzcnt32_hard
lzcnt64_hard
    High level helper for calling an hardware implementation. This will choose either lzcnt32_msvc_x86()/lzcnt64_msvc_x64() or lzcnt32_gcc_x86()/
    lzcnt64_gcc_x64() depending on the environment. Note that this is only available when targeting x86/64. lzcnt64_hard() is only available
    when compiling as 64-bit. You should only call this if has_lzcnt_hard() returns non-zero.

lzcnt32_soft
lzcnt64_soft
    High level helper for calling the best software implementation available for the current build environment.

lzcnt32
lzcnt64
    High level helper for calling either a hardware or software implementation depending on the build environment. This will always favor a
    hardware implementation. Do not call this in high performance code. The reason for this is that each it will call has_lzcnt_hard() each
    time which may be too fine grained for your purposes. You may be better off calling has_lzcnt_hard() once at a higher level.

has_lzcnt_hard
    Determines whether or not a hardware implementation of lzcnt is available. Use this to know whether or not you can call lzcnt32/64_hard().
    Note that this calls CPUID for each, so you may want to cache the result. Use HAS_LZCNT32/64_HARD to check for compile-time support.
*/
#if defined(_MSC_VER)
#include <intrin.h>
#endif

#if defined(__i386) || defined(_M_IX86)
    #define ARCH_X86
#elif defined(__x86_64__) || defined(_M_X64)
    #define ARCH_X64
#elif (defined(__arm__) && defined(__ARM_ARCH) && __ARM_ARCH >= 5) || (defined(_M_ARM) && _M_ARM >= 5) || defined(__ARM_FEATURE_CLZ) /* ARM (Architecture Version 5) */
    #define ARCH_ARM
#endif

#if defined(_WIN64) || defined(_LP64) || defined(__LP64__)
    #define ARCH_64BIT
#else
    #define ARCH_32BIT
#endif

#if defined(ARCH_X86) || defined(ARCH_X64)
    /* x86/64 */
    #if defined(_MSC_VER) && _MSC_VER >= 1500
        #define HAS_LZCNT32_HARD
        #if defined(ARCH_64BIT)
            #define HAS_LZCNT64_HARD
        #endif
    #elif defined(__GNUC__) || defined(__clang__)
        #define HAS_LZCNT32_HARD
        #if defined(ARCH_64BIT)
            #define HAS_LZCNT64_HARD
        #endif
    #endif
#elif defined(ARCH_ARM)
    /* ARM */
    #if defined(__GNUC__) || defined(__clang__)
        #define HAS_LZCNT32_HARD
        #if defined(ARCH_64BIT)
            #define HAS_LZCNT64_HARD
        #endif
    #endif
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1500 && (defined(ARCH_X86) || defined(ARCH_X64)) && !defined(__clang__)
    #define HAS_LZCNT_INTRINSIC
#elif (defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)))
    #define HAS_LZCNT_INTRINSIC
#elif defined(__clang__)
    #if defined(__has_builtin)
        #if __has_builtin(__builtin_clzll) || __has_builtin(__builtin_clzl)
            #define HAS_LZCNT_INTRINSIC
        #endif
    #endif
#endif

unsigned int lzcnt32_generic(unsigned int x)
{
    unsigned int n;
    static unsigned int clz_table_4[] = {
        0,
        4,
        3, 3,
        2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 1
    };

    if (x == 0) {
        return sizeof(x)*8;
    }

    n = clz_table_4[x >> (sizeof(x)*8 - 4)];
    if (n == 0) {
        if ((x & 0xFFFF0000) == 0) { n  = 16; x <<= 16; }
        if ((x & 0xFF000000) == 0) { n += 8;  x <<= 8;  }
        if ((x & 0xF0000000) == 0) { n += 4;  x <<= 4;  }
        n += clz_table_4[x >> (sizeof(x)*8 - 4)];
    }

    return n - 1;
}

unsigned int lzcnt64_generic(unsigned long long x)
{
    unsigned int n;
    static unsigned int clz_table_4[] = {
        0,
        4,
        3, 3,
        2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 1
    };

    if (x == 0) {
        return sizeof(x)*8;
    }

    n = clz_table_4[x >> (sizeof(x)*8 - 4)];
    if (n == 0) {
        if ((x & ((unsigned long long)0xFFFFFFFF << 32)) == 0) { n  = 32; x <<= 32; }
        if ((x & ((unsigned long long)0xFFFF0000 << 32)) == 0) { n += 16; x <<= 16; }
        if ((x & ((unsigned long long)0xFF000000 << 32)) == 0) { n += 8;  x <<= 8;  }
        if ((x & ((unsigned long long)0xF0000000 << 32)) == 0) { n += 4;  x <<= 4;  }
        n += clz_table_4[x >> (sizeof(x)*8 - 4)];
    }

    return n - 1;
}

/* Generic compiler specific intrinsics. */
#if defined(_MSC_VER)
unsigned int lzcnt32_msvc_bsr(unsigned int x)
{
    unsigned long n;

    if (x == 0) {
        return 32;
    }

    _BitScanReverse(&n, x);

    return 32 - n - 1;
}

/* _BitScanReverse64() is only available on 64-bit builds. */
#if defined(ARCH_64BIT)
unsigned int lzcnt64_msvc_bsr(unsigned long long x)
{
    unsigned long n;

    if (x == 0) {
        return 64;
    }

    _BitScanReverse64(&n, x);

    return 64 - n - 1;
}
#endif  /* ARCH_64BIT */
#elif (defined(__GNUC__) || defined(__clang__)) && defined(HAS_LZCNT_INTRINSIC)
unsigned int lzcnt32_gcc_builtin(unsigned int x)
{
    if (x == 0) {
        return 32;
    }

    return (unsigned int)__builtin_clzl((unsigned long)x);
}

unsigned int lzcnt64_gcc_builtin(unsigned long long x)
{
    if (x == 0) {
        return 64;
    }

    return (unsigned int)__builtin_clzll(x);
}
#endif

int has_lzcnt_hard()
{
#if defined(ARCH_X86) || defined(ARCH_X64)
    int info[4] = {0};

    #if defined(_MSC_VER)
        __cpuid(info, 0x80000001);
    #elif defined(__GNUC__) || defined(__clang__)
        /*
        It looks like the -fPIC option uses the ebx register which GCC complains about. We can work around this by just using a different register, the
        specific register of which I'm letting the compiler decide on. The "k" prefix is used to specify a 32-bit register. The {...} syntax is for
        supporting different assembly dialects.

        What's basically happening is that we're saving and restoring the ebx register manually.
        */
        #if defined(ARCH_X86) && defined(__PIC__)
            __asm__ __volatile__ (
                "xchg{l} {%%}ebx, %k1;"
                "cpuid;"
                "xchg{l} {%%}ebx, %k1;"
                : "=a"(info[0]), "=&r"(info[1]), "=c"(info[2]), "=d"(info[3]) : "a"(0x80000001), "c"(0)
            );
        #else
            __asm__ __volatile__ (
                "cpuid" : "=a"(info[0]), "=b"(info[1]), "=c"(info[2]), "=d"(info[3]) : "a"(0x80000001), "c"(0)
            );
        #endif
    #endif

    return (info[2] & (1 << 5)) != 0;
#elif defined(ARCH_ARM)
    return 1;   /* The CLZ instruction is available starting from ARM architecture version 5. Our ARCH_ARM #define is only defined when targeting version 5 at compile time. */
#else
    return 0;   /* Hardware LZCNT is only supported in x86/64 and ARM for now. */
#endif
}

/* Intrinsics and inline-assembly. x86/64 has a hardware LZCNT instruction. You can only call these if has_lzcnt_hard() returns true. */
#if defined(HAS_LZCNT32_HARD)
    #if defined(ARCH_X86) || defined(ARCH_X64)
        #if defined(_MSC_VER) && !defined(__clang__)
            unsigned int lzcnt32_msvc_x86(unsigned int x)
            {
                return __lzcnt(x);
            }
        #elif defined(__GNUC__) || defined(__clang__)
            unsigned int lzcnt32_gcc_x86(unsigned int x)
            {
                /*
                att:   lzcntl [out], [in]
                intel: lzcnt  [in], [out]
                */
                unsigned int r;
                __asm__ __volatile__ (
                    "lzcnt{l %1, %0| %0, %1}" : "=r"(r) : "r"(x)
                );

                return r;
            }
        #endif
    #endif
    #if defined(ARCH_ARM)
        #if defined(__GNUC__) || defined(__clang__)
            unsigned int lzcnt32_gcc_arm(unsigned int x)
            {
                unsigned int r;
                __asm__ __volatile__ (
                #if defined(ARCH_32BIT)
                    "clz %[out], %[in]" : [out]"=r"(r) : [in]"r"(x)
                #else
                    "clz %w[out], %w[in]" : [out]"=r"(r) : [in]"r"(x)
                #endif
                );
                
                return r;
            }
        #endif
    #endif

    unsigned int lzcnt32_hard(unsigned int x)
    {
        #if defined(ARCH_X86) || defined(ARCH_X64)
            #if defined(_MSC_VER) && !defined(__clang__)
                return lzcnt32_msvc_x86(x);
            #elif defined(__GNUC__) || defined(__clang__)
                return lzcnt32_gcc_x86(x);
            #else
                #error "This compiler does not support the lzcnt intrinsic."
            #endif
        #elif defined(ARCH_ARM)
            #if defined(__GNUC__) || defined(__clang__)
                return lzcnt32_gcc_arm(x);
            #else
                #error "This compiler does not support the clz intrinsic."
            #endif
        #else
            #error "The build target does not support a native instruction."
        #endif
    }
#endif

#if defined(HAS_LZCNT64_HARD)
    #if defined(ARCH_X86) || defined(ARCH_X64)
        #if defined(_MSC_VER) && !defined(__clang__)
            unsigned int lzcnt64_msvc_x64(unsigned long long x)
            {
                return (unsigned int)__lzcnt64(x);
            }
        #elif defined(__GNUC__) || defined(__clang__)
            unsigned int lzcnt64_gcc_x64(unsigned long long x)
            {
                /*
                att:   lzcnt [out], [in]
                intel: lzcnt [in], [out]
                */
                unsigned long long r;
                __asm__ __volatile__ (
                    "lzcnt{ %1, %0| %0, %1}" : "=r"(r) : "r"(x)
                );

                return r;
            }
        #endif
    #endif
    #if defined(ARCH_ARM)
        #if defined(__GNUC__) || defined(__clang__)
            unsigned int lzcnt64_gcc_arm(unsigned long long x)
            {
                unsigned long long r;
                __asm__ __volatile__ (
                    "clz %[out], %[in]" : [out]"=r"(r) : [in]"r"(x)
                );
                
                return r;
            }
        #endif
    #endif

    unsigned int lzcnt64_hard(unsigned long long x)
    {
    #if defined(ARCH_X64)
        #if defined(_MSC_VER) && !defined(__clang__)
            return lzcnt64_msvc_x64(x);
        #elif defined(__GNUC__) || defined(__clang__)
            return lzcnt64_gcc_x64(x);
        #else
            #error "This compiler does not support the lzcnt intrinsic."
        #endif
    #elif defined(ARCH_ARM) && defined(ARCH_64BIT)
        #if defined(__GNUC__) || defined(__clang__)
            return lzcnt64_gcc_arm(x);
        #else
            #error "This compiler does not support the clz intrinsic."
        #endif
    #else
        #error "The build target does not support a native instruction."
    #endif
    }
#else
// For wasm
unsigned int lzcnt64_hard(unsigned long long x)
{
  return 0;
}
#endif


unsigned int lzcnt32_soft(unsigned int x)
{
#if defined(_MSC_VER)
    return lzcnt32_msvc_bsr(x);
#elif defined(HAS_LZCNT_INTRINSIC)
    return lzcnt32_gcc_builtin(x);
#else
    return lzcnt32_generic(x);
#endif
}

unsigned int lzcnt64_soft(unsigned long long x)
{
#if defined(ARCH_64BIT)
    #if defined(_MSC_VER)
        return lzcnt64_msvc_bsr(x);
    #elif defined(HAS_LZCNT_INTRINSIC)
        return lzcnt64_gcc_builtin(x);
    #else
        return lzcnt64_generic(x);
    #endif
#else
    return lzcnt64_generic(x);
#endif
}


unsigned int lzcnt32(unsigned int x)
{
#if defined(HAS_LZCNT32_HARD)
    if (has_lzcnt_hard()) {
        return lzcnt32_hard(x);
    } else
#endif
    {
        return lzcnt32_soft(x);
    }
}

unsigned int lzcnt64(unsigned int x)
{
#if defined(HAS_LZCNT64_HARD)
    if (has_lzcnt_hard()) {
        return lzcnt64_hard(x);
    } else
#endif
    {
        return lzcnt64_soft(x);
    }
}

/*
#include <stdio.h>
int do_test32(unsigned int (* lzcnt32proc)(unsigned int), const char* procName)
{
    int testResult = 0;
    unsigned int i;
    
    printf("%s: ", procName);
    for (i = 0; i <= 32; ++i) {
        unsigned int x = (0x80000000 >> i);
        unsigned int r;
        
        // Need a special case for the i=32 case because shifting by more than 31 is undefined. 
        if (i == 32) {
            x = 0;
        }
        
        r = lzcnt32proc(x);
        if (r != i) {
            printf("\n  Failed: x=%u r=%u", x, r);
            testResult = -1;
        }
    }
    
    if (testResult == 0) {
        printf("Passed");
    }
    printf("\n");
    
    return testResult;
}

int do_test64(unsigned int (* lzcnt64proc)(unsigned long long), const char* procName)
{
    int testResult = 0;
    unsigned int i;
    
    printf("%s: ", procName);
    for (i = 0; i <= 64; ++i) {
        unsigned long long x = (0x8000000000000000ULL >> i);
        unsigned int r;
        
        // Need a special case for the i=64 case because shifting by more than 63 is undefined. 
        if (i == 64) {
            x = 0;
        }
        
        r = lzcnt64proc(x);
        if (r != i) {
            printf("\n  Failed: x=%llu r=%u", x, r);
            testResult = -1;
        }
    }
    
    if (testResult == 0) {
        printf("Passed");
    }
    printf("\n");
    
    return testResult;
}

int main(int argc, char** argv)
{
    int exitCode = 0;

    // has_lzcnt_hard() 
#if defined(_WIN64) || defined(_LP64) || defined(__LP64__)
    printf("64 bit\n");
#else
    printf("32 bit\n");
#endif
    printf("has_lzcnt_hard()=%d\n", has_lzcnt_hard());


    // lzcnt32 
    
    // Hardware. 
#if defined(HAS_LZCNT32_HARD)
    if (has_lzcnt_hard()) {
    #if defined(ARCH_X86) || defined(ARCH_X64)
        #if defined(_MSC_VER) && !defined(__clang__)
            if (do_test32(lzcnt32_msvc_x86, "lzcnt32_msvc_x86") != 0) {
                exitCode = -1;
            }
        #elif defined(__GNUC__) || defined(__clang__)
            if (do_test32(lzcnt32_gcc_x86, "lzcnt32_gcc_x86") != 0) {
                exitCode = -1;
            }
        #endif
    #endif
    #if defined(ARCH_ARM)
        #if defined(__GNUC__) || defined(__clang__)
            if (do_test32(lzcnt32_gcc_arm, "lzcnt32_gcc_arm") != 0) {
                exitCode = -1;
            }
        #endif
    #endif
    }
#endif

    // Software 
#if defined(_MSC_VER)
    if (do_test32(lzcnt32_msvc_bsr, "lzcnt32_msvc_bsr") != 0) {
        exitCode = -1;
    }
#elif defined(HAS_LZCNT_INTRINSIC)
    if (do_test32(lzcnt32_gcc_builtin, "lzcnt32_gcc_builtin") != 0) {
        exitCode = -1;
    }
#endif
    if (do_test32(lzcnt32_generic, "lzcnt32_generic") != 0) {
        exitCode = -1;
    }


    // lzcnt64 
    
    // Hardware. 
#if defined(HAS_LZCNT64_HARD)
    if (has_lzcnt_hard()) {
    #if defined(ARCH_X64)
        #if defined(_MSC_VER) && !defined(__clang__)
            if (do_test64(lzcnt64_msvc_x64, "lzcnt64_msvc_x64") != 0) {
                exitCode = -1;
            }
        #elif defined(__GNUC__) || defined(__clang__)
            if (do_test64(lzcnt64_gcc_x64, "lzcnt64_gcc_x64") != 0) {
                exitCode = -1;
            }
        #endif
    #endif
    #if defined(ARCH_ARM)
        #if defined(__GNUC__) || defined(__clang__)
            if (do_test64(lzcnt64_gcc_arm, "lzcnt64_gcc_arm") != 0) {
                exitCode = -1;
            }
        #endif
    #endif
    }
#endif

    // Software 
#if defined(ARCH_64BIT)
    #if defined(_MSC_VER)
        if (do_test64(lzcnt64_msvc_bsr, "lzcnt64_msvc_bsr") != 0) {
            exitCode = -1;
        }
    #elif defined(HAS_LZCNT_INTRINSIC)
        if (do_test64(lzcnt64_gcc_builtin, "lzcnt64_gcc_builtin") != 0) {
            exitCode = -1;
        }
    #endif
#endif
    if (do_test64(lzcnt64_generic, "lzcnt64_generic") != 0) {
        exitCode = -1;
    }
    

    (void)argc;
    (void)argv;
    return exitCode;
}
*/

#endif // REDUCER_H
