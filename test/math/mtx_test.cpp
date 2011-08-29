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

#include <unittest++/UnitTest++.h>
#include <blink/math.h>

SUITE(math) {
  //----------------------------------------------------------------------------
  TEST(m4_load_store) {
    const float src[] __attribute__((aligned(16))) = {
      1.0f, 2.0f, 3.0f, 4.0f,
      5.0f, 6.0f, 7.0f, 8.0f,
      9.0f, 10.0f, 11.0f, 12.0f,
      13.0f, 14.0f, 15.0f, 16.0f
    };
    float dst[16] __attribute__((aligned(16))) = { 0.0f };
    BLMatrix4 m;
    bl_m4_load_f16a(&m, src);
    bl_m4_store_f16a(dst, &m);
    CHECK_ARRAY_CLOSE(src, dst, 16, 0.0001f);
  }

  //----------------------------------------------------------------------------
  TEST(m4_make_identity) {
    const float expected[] __attribute__((aligned(16))) = {
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
    float dst[16] __attribute__((aligned(16))) = { 0.0f };
    BLMatrix4 m;
    bl_m4_make_identity(&m);
    bl_m4_store_f16a(dst, &m);
    CHECK_ARRAY_CLOSE(expected, dst, 16, 0.0001f);
  }

  //----------------------------------------------------------------------------
  TEST(m4_make_perspective) {
    const float expected[] __attribute__((aligned(16))) = {
      -0.161087f,   0.0f,       0.0f,     0.0f,
      0.0f,         -0.214245f, 0.0f,     0.0f,
      0.0f,         0.0f,       -1.002f,  -1.0f,
      0.0f,         0.0f,       -0.2002f, 0.0f
    };
    float dst[16] __attribute__((aligned(16))) = { 0.0f };
    BLMatrix4 m;
    bl_m4_make_perspective(&m, 30.0f * 180.0f / 3.14159265f, 1.33f, 0.1f, 100.0f);
    bl_m4_store_f16a(dst, &m);
    CHECK_ARRAY_CLOSE(expected, dst, 16, 0.0001f);
  }

  //----------------------------------------------------------------------------
  TEST(m4_make_rot_x) {
    // column major
    const float expected[] __attribute__((aligned(16))) = {
      1.0f, 0.0f,       0.0f,         0.0f,
      0.0f, cosf(0.1f), -sinf(0.1f),  0.0f,
      0.0f, sinf(0.1f), cosf(0.1f),   0.0f,
      0.0f, 0.0f,       0.0f,         1.0f
    };
    float dst[16] __attribute__((aligned(16))) = { 0.0f };
    BLMatrix4 m;
    bl_m4_make_rot_x(&m, 0.1f);
    bl_m4_store_f16a(dst, &m);
    CHECK_ARRAY_CLOSE(expected, dst, 16, 0.0001f);
  }

  //----------------------------------------------------------------------------
  TEST(m4_make_rot_y) {
    // column major
    const float expected[] __attribute__((aligned(16))) = {
      cosf(0.1f),   0.0f, sinf(0.1f), 0.0f,
      0.0f,         1.0f, 0.0f,       0.0f,
      -sinf(0.1f),  0.0f, cosf(0.1f), 0.0f,
      0.0f,         0.0f, 0.0f,       1.0f
    };
    float dst[16] __attribute__((aligned(16))) = { 0.0f };
    BLMatrix4 m;
    bl_m4_make_rot_y(&m, 0.1f);
    bl_m4_store_f16a(dst, &m);
    CHECK_ARRAY_CLOSE(expected, dst, 16, 0.0001f);
  }

  //----------------------------------------------------------------------------
  TEST(m4_make_rot_z) {
    // column major
    const float expected[] __attribute__((aligned(16))) = {
      cosf(0.1f), -sinf(0.1f),  0.0f, 0.0f,
      sinf(0.1f), cosf(0.1f),   0.0f, 0.0f,
      0.0f,       0.0f,         1.0f, 0.0f,
      0.0f,       0.0f,         0.0f, 1.0f
    };
    float dst[16] __attribute__((aligned(16))) = { 0.0f };
    BLMatrix4 m;
    bl_m4_make_rot_z(&m, 0.1f);
    bl_m4_store_f16a(dst, &m);
    CHECK_ARRAY_CLOSE(expected, dst, 16, 0.0001f);
  }

  //----------------------------------------------------------------------------
  TEST(m4_make_scale) {
    // column major
    const float expected[] __attribute__((aligned(16))) = {
      2.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 3.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 4.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
    float dst[16] __attribute__((aligned(16))) = { 0.0f };
    BLMatrix4 m;
    BLVec s;
    s = bl_vec_set_f(2.0f, 3.0f, 4.0f, 5.0f);
    bl_m4_make_scale(&m, s);
    bl_m4_store_f16a(dst, &m);
    CHECK_ARRAY_CLOSE(expected, dst, 16, 0.0001f);
  }

  //----------------------------------------------------------------------------
  TEST(m4_make_trans) {
    // column major
    const float expected[] __attribute__((aligned(16))) = {
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      2.0f, 3.0f, 4.0f, 1.0f
    };
    float dst[16] __attribute__((aligned(16))) = { 0.0f };
    BLMatrix4 m;
    BLVec t;
    t = bl_vec_set_f(2.0f, 3.0f, 4.0f, 5.0f);
    bl_m4_make_trans(&m, t);
    bl_m4_store_f16a(dst, &m);
    CHECK_ARRAY_CLOSE(expected, dst, 16, 0.0001f);
  }

  //----------------------------------------------------------------------------
  TEST(m4_mul_vec4) {
    // column major
    const float src[] __attribute__((aligned(16))) = {
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      2.0f, 3.0f, 4.0f, 1.0f
    };
    float dst[4] __attribute__((aligned(16))) = { 0.0f };
    BLMatrix4 m;
    BLVec v, r;
    v = bl_vec_set_f(2.0f, 3.0f, 4.0f, 5.0f);
    bl_m4_load_f16a(&m, src);
    r = bl_m4_mul_vec4(&m, v);
    bl_vec_store_f4a(dst, r);
    CHECK_EQUAL(12.0f, dst[0]);
    CHECK_EQUAL(18.0f, dst[1]);
    CHECK_EQUAL(24.0f, dst[2]);
    CHECK_EQUAL(5.0f, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(m4_mul) {
    // column major
    const float expected[] __attribute__((aligned(16))) = {
      5.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 4.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 3.0f, 0.0f,
      2.0f, 3.0f, 4.0f, 1.0f
    };
    float dst[16] __attribute__((aligned(16))) = { 0.0f };
    BLMatrix4 m1, m2, m3;
    bl_m4_make_trans(&m1, bl_vec_set_f(2.0f, 3.0f, 4.0f, 0.0f));
    bl_m4_make_scale(&m2, bl_vec_set_f(5.0f, 4.0f, 3.0f, 0.0f));
    bl_m4_mul(&m3, &m1, &m2);
    bl_m4_store_f16a(dst, &m3);
    CHECK_ARRAY_CLOSE(expected, dst, 16, 0.001f);
  }

  //----------------------------------------------------------------------------
  TEST(m4_transpose) {
    // column major
    const float src[] __attribute__((aligned(16))) = {
      1.0f,   2.0f,   3.0f,   4.0f,
      5.0f,   6.0f,   7.0f,   8.0f,
      9.0f,   10.0f,  11.0f,  12.0f,
      13.0f,  14.0f,  15.0f,  16.0f
    };
    const float expected[] __attribute__((aligned(16))) = {
      1.0f,   5.0f,   9.0f,   13.0f,
      2.0f,   6.0f,   10.0f,  14.0f,
      3.0f,   7.0f,   11.0f,  15.0f,
      4.0f,   8.0f,   12.0f,  16.0f
    };
    float dst[16] __attribute__((aligned(16))) = { 0.0f };
    BLMatrix4 m1, m2;
    bl_m4_load_f16a(&m1, src);
    bl_m4_transpose(&m2, &m1);
    bl_m4_store_f16a(dst, &m2);
    CHECK_ARRAY_CLOSE(expected, dst, 16, 0.001f);
  }
}
