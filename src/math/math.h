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
#include <math.h>

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
// Custom assert wrapper so math asserts can be enabled only when desired since
// these functions tend to be in inner loops.
//

//#define BL_MATH_ASSERT_ENABLED
#ifdef BL_MATH_ASSERT_ENABLED
# define BL_MATH_ASSERT(cond)   BL_ASSERT(cond)
# else
# define BL_MATH_ASSERT(cond)   do { BL_UNUSED(cond); } while (0)
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


void bl_m4_load_f16a(BLMatrix4* __restrict dest, const float* __restrict src);
void bl_m4_store_f16a(float* __restrict dest, const BLMatrix4* __restrict m);
void bl_m4_make_identity(BLMatrix4* __restrict dest);
void bl_m4_make_perspective(BLMatrix4* __restrict dest, float fov, float aspect, float z_near, float z_far);
void bl_m4_make_rot_x(BLMatrix4* __restrict dest, float angle);
void bl_m4_make_rot_y(BLMatrix4* __restrict dest, float angle);
void bl_m4_make_rot_z(BLMatrix4* __restrict dest, float angle);
void bl_m4_make_scale(BLMatrix4* __restrict dest, BLVec scale);
void bl_m4_make_trans(BLMatrix4* __restrict dest, BLVec trans);

BLVec bl_m4_mul_vec4(const BLMatrix4* __restrict m, BLVec v);
void bl_m4_mul(BLMatrix4* __restrict dest, const BLMatrix4* __restrict m1, const BLMatrix4* __restrict m2);
void bl_m4_transpose(BLMatrix4* __restrict dest, const BLMatrix4* __restrict m);


//
// SSE implementation
//

#if defined(BL_MATH_USE_SSE)

//------------------------------------------------------------------------------
inline BLVec bl_vec_load_f3(const float* __restrict src) {
  BL_MATH_ASSERT(src && BL_IS_ALIGNED_PTR(src, 4));
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
  BL_MATH_ASSERT(src && BL_IS_ALIGNED_PTR(src, 16));
  // load the extra float anyway since it's guaranteed to be within the same
  // cacheline due to the src being aligned
  return _mm_load_ps(src);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_load_f4a(const float* __restrict src) {
  BL_MATH_ASSERT(src && BL_IS_ALIGNED_PTR(src, 16));
  return _mm_load_ps(src);
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_load_i4(const uint32_t* __restrict src) {
  BL_MATH_ASSERT(src && BL_IS_ALIGNED_PTR(src, 4));
  __m128i r = _mm_loadu_si128((const __m128i*)src);
  return *(__m128*)&r;
}

//------------------------------------------------------------------------------
inline BLVec bl_vec_load_i4a(const uint32_t* __restrict src) {
  BL_MATH_ASSERT(src && BL_IS_ALIGNED_PTR(src, 16));
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
  BL_MATH_ASSERT(dest && BL_IS_ALIGNED_PTR(dest, 4));
  __m128 y, z;
  y   = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));   // y = |y y y y|
  z   = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));   // z = |z z z z|
        _mm_store_ss(dest, v);
        _mm_store_ss(dest + 1, y);
        _mm_store_ss(dest + 2, z);
}

//------------------------------------------------------------------------------
inline void bl_vec_store_f3a(float* __restrict dest, BLVec v) {
  BL_MATH_ASSERT(dest && BL_IS_ALIGNED_PTR(dest, 16));
  // TODO: can i do better?
  bl_vec_store_f3(dest, v);
}

//------------------------------------------------------------------------------
inline void bl_vec_store_f4a(float* __restrict dest, BLVec v) {
  BL_MATH_ASSERT(dest && BL_IS_ALIGNED_PTR(dest, 16));
  _mm_store_ps(dest, v);
}

//------------------------------------------------------------------------------
inline void bl_vec_store_i4(uint32_t* __restrict dest, BLVec v) {
  BL_MATH_ASSERT(dest && BL_IS_ALIGNED_PTR(dest, 4));
  _mm_storeu_si128((__m128i*)dest, *(__m128i*)&v);
}

//------------------------------------------------------------------------------
inline void bl_vec_store_i4a(uint32_t* __restrict dest, BLVec v) {
  BL_MATH_ASSERT(dest && BL_IS_ALIGNED_PTR(dest, 16));
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


//------------------------------------------------------------------------------
inline void bl_m4_load_f16a(BLMatrix4* __restrict dest, const float* __restrict src) {
  BL_MATH_ASSERT(dest && src && BL_IS_ALIGNED_PTR(src, 16));
  dest->c0 = bl_vec_load_f4a(src);
  dest->c1 = bl_vec_load_f4a(src + 4);
  dest->c2 = bl_vec_load_f4a(src + 8);
  dest->c3 = bl_vec_load_f4a(src + 12);
}

//------------------------------------------------------------------------------
inline void bl_m4_store_f16a(float* __restrict dest, const BLMatrix4* __restrict src) {
  BL_MATH_ASSERT(dest && src && BL_IS_ALIGNED_PTR(dest, 16));
  bl_vec_store_f4a(dest, src->c0);
  bl_vec_store_f4a(dest + 4, src->c1);
  bl_vec_store_f4a(dest + 8, src->c2);
  bl_vec_store_f4a(dest + 12, src->c3);
}

//------------------------------------------------------------------------------
inline void bl_m4_make_identity(BLMatrix4* __restrict dest) {
  BL_MATH_ASSERT(dest);
  // result:
  //  |1 0 0 0|
  //  |0 1 0 0|
  //  |0 0 1 0|
  //  |0 0 0 1|
  dest->c0 = bl_vec_make_axis_x();
  dest->c1 = bl_vec_make_axis_y();
  dest->c2 = bl_vec_make_axis_z();
  dest->c3 = bl_vec_make_axis_w();
}

//------------------------------------------------------------------------------
inline void bl_m4_make_perspective(BLMatrix4* __restrict dest, float fov, float aspect, float z_near, float z_far) {
  // result:
  //  |(2n)/(r-l) 0          (r+l)/(r-l)  0
  //  |0          (2n)/(t-b) (t+b)/(t-b)  0
  //  |0          0          -(f+n)/(f-n) (-2fn)/(f-n)
  //  |0          0          -1           0

  // TODO: optimize this into SSE
  float ymax = z_near * tanf(fov * 0.5f);
  float xmax = ymax * aspect;

  float left   = -xmax;
  float right  = xmax;
  float bottom = -ymax;
  float top    = ymax;

  float near2  = 2.0f * z_near;
  float width  = right - left;
  float height = top - bottom;
  float depth  = z_far - z_near;

  __m128 c0, c1, c2, c3;
  c0 = _mm_setr_ps(near2 / width, 0.0f, 0.0f, 0.0f);
  c1 = _mm_setr_ps(0.0f, near2 / height, 0.0f, 0.0f);
  c2 = _mm_setr_ps((right + left) / width, (top + bottom) / height, (-z_far - z_near) / depth, -1.0f);
  c3 = _mm_setr_ps(0.0f, 0.0f, (-near2 * z_far) / depth, 0.0f);

  dest->c0 = c0;
  dest->c1 = c1;
  dest->c2 = c2;
  dest->c3 = c3;
}

//------------------------------------------------------------------------------
inline void bl_m4_make_rot_x(BLMatrix4* __restrict dest, float angle_rads) {
  BL_MATH_ASSERT(dest);
  // result:
  //  |1  0 0 0|
  //  |0  c s 0|
  //  |0 -s c 0|
  //  |0  0 0 1|

  // TODO: do sin/cos in vector registers
  float sin_scalar = sinf(angle_rads);
  float cos_scalar = cosf(angle_rads);

  __m128 s, c, nz, t, c1, c2;
  s   = _mm_set_ss(sin_scalar);                     // s  = |s 0 0 0|
  c   = _mm_set_ss(cos_scalar);                     // c  = |c 0 0 0|
  nz  = _mm_set_ps(1.0f, -1.0f, 1.0f, 1.0f);        // nz = |1 1 -1 1|
  t   = _mm_shuffle_ps(c, s, _MM_SHUFFLE(1,0,0,1)); // t  = |0 c s 0|
  c1  = _mm_mul_ps(t, nz);                          // c1 = |0 c -s 0|
  c2  = _mm_shuffle_ps(s, c, _MM_SHUFFLE(1,0,0,1)); // c2 = |0 s c 0|

  dest->c0 = bl_vec_make_axis_x();
  dest->c1 = c1;
  dest->c2 = c2;
  dest->c3 = bl_vec_make_axis_w();
}

//------------------------------------------------------------------------------
inline void bl_m4_make_rot_y(BLMatrix4* __restrict dest, float angle_rads) {
  BL_MATH_ASSERT(dest);
  // result:
  //  |c 0 -s 0|
  //  |0 1  0 0|
  //  |s 0  c 0|
  //  |0 0  0 1|

  // TODO: do sin/cos in vector registers
  float sin_scalar = sinf(angle_rads);
  float cos_scalar = cosf(angle_rads);

  __m128 s, c, nx, t, c0, c2;
  s   = _mm_set_ss(sin_scalar);                     // s  = |s 0 0 0|
  c   = _mm_set_ss(cos_scalar);                     // c  = |c 0 0 0|
  nx  = _mm_set_ps(1.0f, 1.0f, 1.0f, -1.0f);        // nx = |-1 1 1 1|
  t   = _mm_shuffle_ps(s, c, _MM_SHUFFLE(1,0,1,0)); // t  = |s 0 c 0|
  c0  = _mm_shuffle_ps(c, s, _MM_SHUFFLE(1,0,1,0)); // c0 = |c 0 s 0|
  c2  = _mm_mul_ps(t, nx);                          // c2 = |-s 0 c 0|

  dest->c0 = c0;
  dest->c1 = bl_vec_make_axis_y();
  dest->c2 = c2;
  dest->c3 = bl_vec_make_axis_w();
}

//------------------------------------------------------------------------------
inline void bl_m4_make_rot_z(BLMatrix4* __restrict dest, float angle_rads) {
  BL_MATH_ASSERT(dest);
  // result:
  //  | c s 0 0|
  //  |-s c 0 0|
  //  | 0 0 1 0|
  //  | 0 0 0 1|

  // TODO: do sin/cos in vector registers
  float sin_scalar = sinf(angle_rads);
  float cos_scalar = cosf(angle_rads);

  __m128 s, c, ny, t, c0, c1;
  s   = _mm_set_ss(sin_scalar);                     // s  = |s 0 0 0|
  c   = _mm_set_ss(cos_scalar);                     // c  = |c 0 0 0|
  ny  = _mm_set_ps(1.0f, 1.0f, -1.0f, 1.0f);        // nx = |1 -1 1 1|
  t   = _mm_unpacklo_ps(c, s);                      // t  = |c s 0 0|
  c1  = _mm_unpacklo_ps(s, c);                      // c1 = |s c 0 0|
  c0  = _mm_mul_ps(t, ny);                          // c0 = |c -s 0 0|

  dest->c0 = c0;
  dest->c1 = c1;
  dest->c2 = bl_vec_make_axis_z();
  dest->c3 = bl_vec_make_axis_w();
}

//------------------------------------------------------------------------------
inline void bl_m4_make_scale(BLMatrix4* __restrict dest, BLVec scale) {
  BL_MATH_ASSERT(dest);
  // result:
  //  |sx  0  0 0|
  //  | 0 sy  0 0|
  //  | 0  0 sz 0|
  //  | 0  0  0 1|

  BLVec x, y, z, sx, sy, sz;
  x   = bl_vec_make_axis_x();
  y   = bl_vec_make_axis_y();
  z   = bl_vec_make_axis_z();
  sx  = bl_vec_mul(x, scale);
  sy  = bl_vec_mul(y, scale);
  sz  = bl_vec_mul(z, scale);

  dest->c0 = sx;
  dest->c1 = sy;
  dest->c2 = sz;
  dest->c3 = bl_vec_make_axis_w();
}

//------------------------------------------------------------------------------
inline void bl_m4_make_trans(BLMatrix4* __restrict dest, BLVec trans) {
  BL_MATH_ASSERT(dest);
  // result:
  //  |1 0 0 tx|
  //  |0 1 0 ty|
  //  |0 0 1 tz|
  //  |0 0 0  1|

  __m128 t1, t2, c3;
  t1  = bl_vec_make_axis_w();                             // t1 = |0 0 0 1|
  t2  = _mm_unpackhi_ps(trans, t1);                       // t2 = |z 0 ? 1|
  c3  = _mm_shuffle_ps(trans, t2, _MM_SHUFFLE(3,0,1,0));  // c3 = |x y z 1|

  dest->c0 = bl_vec_make_axis_x();
  dest->c1 = bl_vec_make_axis_y();
  dest->c2 = bl_vec_make_axis_z();
  dest->c3 = c3;
}

//------------------------------------------------------------------------------
inline BLVec bl_m4_mul_vec4(const BLMatrix4* __restrict m, BLVec v) {
  BL_MATH_ASSERT(m);
  // result:
  //  |a e i m|   |x|
  //  |b f j n| * |y|
  //  |c g k o|   |z|
  //  |d h l p|   |w|
  //
  //  = |ax+ey+iz+mw bx+fy+jz+nw cx+gy+kz+ow dx+hy+lz+pw|
  BLVec r, t0, t1, t2, t3, xxxx, yyyy, zzzz, wwww;
  xxxx  = bl_vec_splat_x(v);
  yyyy  = bl_vec_splat_y(v);
  zzzz  = bl_vec_splat_z(v);
  wwww  = bl_vec_splat_w(v);
  r     = bl_vec_mul(m->c0, xxxx);  // r  = |ax bx cx dx|
  t0    = bl_vec_mul(m->c1, yyyy);  // t0 = |ey fy gy hy|
  t1    = bl_vec_mul(m->c2, zzzz);  // t1 = |iz jz kz lz|
  t2    = bl_vec_mul(m->c3, wwww);  // t2 = |mw nw ow pw|
  r     = bl_vec_add(r, t0);        // r  = |ax+ey bx+fy cx+gy dx+hy|
  t3    = bl_vec_add(t1, t2);       // t3 = |iz+mw jz+nw kz+ow lz+pw|
  r     = bl_vec_add(r, t3);        // r  = |ax+ey+iz+mw bx+fy+jz+nw cx+gy+kz+ow dx+hy+lz+pw|
  return r;
}

//------------------------------------------------------------------------------
inline void bl_m4_mul(BLMatrix4* __restrict dest, const BLMatrix4* __restrict m1, const BLMatrix4* __restrict m2) {
  BL_MATH_ASSERT(dest && m1 && m2);
  // result:
  //  |a e i m|   |A E I M|
  //  |b f j n| * |B F J N|
  //  |c g k o|   |C G K O|
  //  |d h l p|   |D H L P|
  //
  //    |aA+eB+iC+mD aE+eF+iG+mH aI+eJ+iK+mL aM+eN+iO+mP|
  //  = |bA+fB+jC+nD bE+fF+jG+nH bI+fJ+jK+nL bM+fN+jO+nP|
  //    |cA+gB+kC+oD cE+gF+kG+oH cI+gJ+kK+oL cM+gN+kO+oP|
  //    |dA+hB+lC+pD dE+hF+lG+pH dI+hJ+lK+pL dM+hN+lO+pP|

  BLVec c0, c1, c2, c3;
  c0 = bl_m4_mul_vec4(m1, m2->c0);  // |aA+eB+iC+mD aE+eF+iG+mH aI+eJ+iK+mL aM+eN+iO+mP|
  c1 = bl_m4_mul_vec4(m1, m2->c1);  // |bA+fB+jC+nD bE+fF+jG+nH bI+fJ+jK+nL bM+fN+jO+nP|
  c2 = bl_m4_mul_vec4(m1, m2->c2);  // |cA+gB+kC+oD cE+gF+kG+oH cI+gJ+kK+oL cM+gN+kO+oP|
  c3 = bl_m4_mul_vec4(m1, m2->c3);  // |dA+hB+lC+pD dE+hF+lG+pH dI+hJ+lK+pL dM+hN+lO+pP|

  dest->c0 = c0;
  dest->c1 = c1;
  dest->c2 = c2;
  dest->c3 = c3;
}

//------------------------------------------------------------------------------
inline void bl_m4_transpose(BLMatrix4* __restrict dest, const BLMatrix4* __restrict m) {
  BL_MATH_ASSERT(dest && m);
  // result:
  //  |a e i m|    |a b c d|
  //  |b f j n| => |e f g h|
  //  |c g k o|    |i j k l|
  //  |d h l p|    |m n o p|

  __m128 t0, t1, t2, t3, c0, c1, c2, c3;
  t0 = _mm_unpacklo_ps(m->c0, m->c1); // t0 = |a e b f|
  t1 = _mm_unpacklo_ps(m->c2, m->c3); // t1 = |i m j n|
  t2 = _mm_unpackhi_ps(m->c0, m->c1); // t2 = |c g d h|
  t3 = _mm_unpackhi_ps(m->c2, m->c3); // t3 = |k o l p|

  c0 = _mm_movelh_ps(t0, t1);         // c0 = |a e i m|
  c1 = _mm_movehl_ps(t1, t0);         // c1 = |b f j n|
  c2 = _mm_movelh_ps(t2, t3);         // c2 = |c g k o|
  c3 = _mm_movehl_ps(t3, t2);         // c3 = |d h l p|

  dest->c0 = c0;
  dest->c1 = c1;
  dest->c2 = c2;
  dest->c3 = c3;
}

#endif

#endif
