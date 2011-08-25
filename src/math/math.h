// Copyright (c) 2011, Ben Scott.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#pragma once
#ifndef BL_MATH_H
#define BL_MATH_H

#include <base/base.h>

//
// Select the implementation
//

#if defined(BL_ARCH_IA32) || defined(BL_ARCH_X64)
# define BL_MATH_USE_SSE
#elif defined(BL_ARCH_PPC) || defined(BL_ARCH_PPC64)
# define BL_MATH_USE_ALTIVEC
#else
# error Unsupported architecture
#endif


//
// types
//

#if defined(BL_MATH_USE_SSE)
# include <xmmintrin.h>

typedef __m128 BLVec;

#elif defined(BL_MATH_USE_ALTIVEC)
#else
# error Unsupported architecture
#endif

// AOS 3x3 matrix
struct BLMatrix3 {
  BLVec c0;
  BLVec c1;
  BLVec c2;
};

// AOS 4x4 matrix
struct BLMatrix4 {
  BLVec c0;
  BLVec c1;
  BLVec c2;
  BLVec c3;
};

// AOS 3x4 affine matrix
struct BLMatrixT {
  BLVec c0;
  BLVec c1;
  BLVec c2;
  BLVec c3;
};


//
// api
//

BLVec bl_vec_load_f3(const float* __restrict src);
BLVec bl_vec_load_f3a(const float* __restrict src);
BLVec bl_vec_load_f4a(const float* __restrict src);
BLVec bl_vec_load_i4a(const uint32_t* __restrict src);
BLVec bl_vec_set_f(float x, float y, float z, float w);
BLVec bl_vec_set_i(uint32_t x, uint32_t y, uint32_t z, uint32_t w);
BLVec bl_vec_splat_scalar(float scalar);
void bl_vec_store_f3(float* __restrict dest, BLVec v);
void bl_vec_store_f3a(float* __restrict dest, BLVec v);
void bl_vec_store_f4a(float* __restrict dest, BLVec v);
void bl_vec_store_i4a(uint32_t* __restrict dest, BLVec v);

BLVec bl_vec_make_zero();
BLVec bl_vec_make_axis_x();
BLVec bl_vec_make_axis_y();
BLVec bl_vec_make_axis_z();
BLVec bl_vec_make_axis_w();
BLVec bl_vec_splat_x(BLVec v);
BLVec bl_vec_splat_y(BLVec v);
BLVec bl_vec_splat_z(BLVec v);
BLVec bl_vec_splat_w(BLVec v);

BLVec bl_vec_add(BLVec v1, BLVec v2);
BLVec bl_vec_and(BLVec v1, BLVec v2);
BLVec bl_vec_cross3(BLVec v1, BLVec v2);
BLVec bl_vec_dot3(BLVec v1, BLVec v2);
BLVec bl_vec_dot4(BLVec v1, BLVec v2);
BLVec bl_vec_length3(BLVec v);
BLVec bl_vec_length4(BLVec v);
BLVec bl_vec_length_sq3(BLVec v);
BLVec bl_vec_length_sq4(BLVec v);
BLVec bl_vec_lerp(BLVec v1, BLVec v2, BLVec t);
BLVec bl_vec_madd(BLVec v1, BLVec v2, BLVec v3);
BLVec bl_vec_max(BLVec v1, BLVec v2);
BLVec bl_vec_min(BLVec v1, BLVec v2);
BLVec bl_vec_mul(BLVec v1, BLVec v2);
BLVec bl_vec_neg(BLVec v);
BLVec bl_vec_or(BLVec v1, BLVec v2);
BLVec bl_vec_recip(BLVec v);
BLVec bl_vec_rsqrt(BLVec v);
BLVec bl_vec_sub(BLVec v1, BLVec v2);
BLVec bl_vec_sqrt(BLVec v);

BLVec bl_vec_cmp_eq(BLVec v1, BLVec v2);
BLVec bl_vec_cmp_ge(BLVec v1, BLVec v2);
BLVec bl_vec_cmp_gt(BLVec v1, BLVec v2);
BLVec bl_vec_cmp_le(BLVec v1, BLVec v2);
BLVec bl_vec_cmp_lt(BLVec v1, BLVec v2);
BLVec bl_vec_cmp_neq(BLVec v1, BLVec v2);


//
// SSE implementation
//

#if defined(BL_MATH_USE_SSE)

//------------------------------------------------------------------------------
inline BLVec bl_vec_load_f3(const float* __restrict src) {
  BL_ASSERT(src && BL_IS_ALIGNED_PTR(src, 4));
  __m128 x, y, z, xy, r;
  x   = _mm_load_ss(src);         // x  = |x ? ? ?|
  y   = _mm_load_ss(src + 1);     // y  = |y ? ? ?|
  z   = _mm_load_ss(src + 2);     // z  = |z ? ? ?|
  xy  = _mm_unpacklo_ps(x, y);    // xy = |x y ? ?|
  r   = _mm_movelh_ps(xy, z);     // r  = |x y z ?|
  return r;
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_load_f3a(const float* __restrict src) {
  BL_ASSERT(src && BL_IS_ALIGNED_PTR(src, 16));
  // load the extra float anyway since it's guaranteed to be within the same
  // cacheline due to the src being aligned
  return _mm_load_ps(src);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_load_f4a(const float* __restrict src) {
  BL_ASSERT(src && BL_IS_ALIGNED_PTR(src, 16));
  return _mm_load_ps(src);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_load_i4(const uint32_t* __restrict src) {
  BL_ASSERT(src && BL_IS_ALIGNED_PTR(src, 4));
  __m128i r = _mm_loadu_si128((const __m128i*)src);
  return *(__m128*)&r;
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_load_i4a(const uint32_t* __restrict src) {
  BL_ASSERT(src && BL_IS_ALIGNED_PTR(src, 16));
  __m128i r = _mm_load_si128((const __m128i*)src);
  return *(__m128*)&r;
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_set_f(float x, float y, float z, float w) {
  return _mm_set_ps(w, z, y, x);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_set_i(uint32_t x, uint32_t y, uint32_t z, uint32_t w) {
  return _mm_set_epi32(w, z, y, x);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_splat_scalar(float scalar) {
  return _mm_set_ps(scalar, scalar, scalar, scalar);
}

//------------------------------------------------------------------------------
inline void bl_vec_store_f3(float* __restrict dest, BLVec v) {
  BL_ASSERT(dest && BL_IS_ALIGNED_PTR(dest, 4));
  __m128 y, z;
  y   = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));   // y = |y y y y|
  z   = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));   // z = |z z z z|
        _mm_store_ss(dest, v);
        _mm_store_ss(dest + 1, y);
        _mm_store_ss(dest + 2, z);
}

//------------------------------------------------------------------------------
inline void bl_vec_store_f3a(float* __restrict dest, BLVec v) {
  BL_ASSERT(dest && BL_IS_ALIGNED_PTR(dest, 16));
  // TODO: can i do better?
  bl_vec_store_f3(dest, v);
}

//------------------------------------------------------------------------------
inline void bl_vec_store_f4a(float* __restrict dest, BLVec v) {
  BL_ASSERT(dest && BL_IS_ALIGNED_PTR(dest, 16));
  _mm_store_ps(dest, v);
}

//------------------------------------------------------------------------------
inline void bl_vec_store_i4(uint32_t* __restrict dest, BLVec v) {
  BL_ASSERT(dest && BL_IS_ALIGNED_PTR(dest, 4));
  _mm_storeu_si128((__m128i*)dest, *(__m128i*)&v);
}

//------------------------------------------------------------------------------
inline void bl_vec_store_i4a(uint32_t* __restrict dest, BLVec v) {
  BL_ASSERT(dest && BL_IS_ALIGNED_PTR(dest, 16));
  _mm_store_si128((__m128i*)dest, *(__m128i*)&v);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_make_zero() {
  return _mm_setzero_ps();
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_make_axis_x() {
  return _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_make_axis_y() {
  return _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_make_axis_z() {
  return _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_make_axis_w() {
  return _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_splat_x(BLVec v) {
  return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_splat_y(BLVec v) {
  return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_splat_z(BLVec v) {
  return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_splat_w(BLVec v) {
  return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3,3,3,3));
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_add(BLVec v1, BLVec v2) {
  return _mm_add_ps(v1, v2);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_and(BLVec v1, BLVec v2) {
  return _mm_and_ps(v1, v2);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_cross3(BLVec v1, BLVec v2) {
  // |bC-cB cA-aC aB-bA ?| = |a b c ?| x |A B C ?|
  __m128 t1, t2, t3, t4, lt, rt, r;
  t1  = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3,0,2,1)); // t1 = |b c a ?|
  t2  = _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3,1,0,2)); // t2 = |C A B ?|
  t3  = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3,1,0,2)); // t3 = |c a b ?|
  t4  = _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3,0,2,1)); // t4 = |B C A ?|
  lt  = _mm_mul_ps(t1, t2);                           // lt = |bC cA aB ?|
  rt  = _mm_mul_ps(t3, t4);                           // rt = |cB aC bA ?|
  r   = _mm_sub_ps(lt, rt);                           // r  = |bC-cB cA-aC aB-bA ?|
  return r;
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_dot3(BLVec v1, BLVec v2) {
  // |aA+bB+cC ? ? ?| = |a b c ?| * |A B C ?|
  __m128 t1, t2, t3, t4, r;
  t1  = _mm_mul_ps(v1, v2);                           // t1 = |aA bB cC ?|
  t2  = _mm_shuffle_ps(t1, t1, _MM_SHUFFLE(3,3,2,1)); // t2 = |bB cC ? ?|
  t3  = _mm_shuffle_ps(t2, t2, _MM_SHUFFLE(3,3,3,1)); // t3 = |cC ? ? ?|
  t4  = _mm_add_ps(t1, t2);                           // t4 = |aA+bB ? ? ?|
  r   = _mm_add_ps(t4, t3);                           // r  = |aA+bB+cC ? ? ?|
  return r;
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_dot4(BLVec v1, BLVec v2) {
  // |aA+bB+cC+dD ? ? ?| = |a b c d| * |A B C D|
  __m128 t1, t2, t3, t4, r;
  t1  = _mm_mul_ps(v1, v2);                           // t1 = |aA bB cC dD|
  t2  = _mm_shuffle_ps(t1, t1, _MM_SHUFFLE(3,3,3,2)); // t2 = |cC dD ? ?|
  t3  = _mm_add_ps(t1, t2);                           // t3 = |aA+cC bB+dD ? ?|
  t4  = _mm_shuffle_ps(t3, t3, _MM_SHUFFLE(3,3,3,1)); // t4 = |bB+dD ? ? ?|
  r   = _mm_add_ps(t3, t4);                           // r  = |aA+bB+cC+dD ? ? ?|
  return r;
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_length3(BLVec v) {
  __m128 t, r;
  t   = bl_vec_length_sq3(v);
  r   = _mm_sqrt_ps(t);
  return r;
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_length4(BLVec v) {
  __m128 t, r;
  t   = bl_vec_length_sq4(v);
  r   = _mm_sqrt_ps(t);
  return r;
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_length_sq3(BLVec v) {
  return bl_vec_dot3(v, v);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_length_sq4(BLVec v) {
  return bl_vec_dot4(v, v);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_lerp(BLVec v1, BLVec v2, BLVec t) {
  // result = v1 + t(v2 - v1)
  __m128 d, r;
  d   = _mm_sub_ps(v2, v1);
  r   = bl_vec_madd(t, d, v1);
  return r;
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_madd(BLVec v1, BLVec v2, BLVec v3) {
  __m128 t, r;
  t   = _mm_mul_ps(v1, v2);
  r   = _mm_add_ps(t, v3);
  return r;
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_max(BLVec v1, BLVec v2) {
  return _mm_max_ps(v1, v2);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_min(BLVec v1, BLVec v2) {
  return _mm_min_ps(v1, v2);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_mul(BLVec v1, BLVec v2) {
  return _mm_mul_ps(v1, v2);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_neg(BLVec v) {
  __m128 z, r;
  z   = _mm_setzero_ps();
  r   = _mm_sub_ps(z, v);
  return r;
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_or(BLVec v1, BLVec v2) {
  return _mm_or_ps(v1, v2);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_recip(BLVec v) {
  return _mm_rcp_ps(v);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_rsqrt(BLVec v) {
  return _mm_rsqrt_ps(v);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_sub(BLVec v1, BLVec v2) {
  return _mm_sub_ps(v1, v2);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_sqrt(BLVec v) {
  __m128 t, r;
  t   = _mm_rsqrt_ps(v);
  r   = _mm_rcp_ps(t);
  return r;
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_cmp_eq(BLVec v1, BLVec v2) {
  return _mm_cmpeq_ps(v1, v2);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_cmp_ge(BLVec v1, BLVec v2) {
  return _mm_cmpge_ps(v1, v2);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_cmp_gt(BLVec v1, BLVec v2) {
  return _mm_cmpgt_ps(v1, v2);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_cmp_le(BLVec v1, BLVec v2) {
  return _mm_cmple_ps(v1, v2);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_cmp_lt(BLVec v1, BLVec v2) {
  return _mm_cmplt_ps(v1, v2);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_cmp_neq(BLVec v1, BLVec v2) {
  return _mm_cmpneq_ps(v1, v2);
}

#endif

#endif
