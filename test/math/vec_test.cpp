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
#include <blink/vecmath.h>

SUITE(math) {
  //----------------------------------------------------------------------------
  TEST(vec_load_store_f3) {
    const float src[] = { 2.0f, 3.0f, 4.0f };
    float dst[] = { 0.0f, 0.0f, 0.0f };
    BLVec v = bl_vec_load_f3(src);
    bl_vec_store_f3(dst, v);
    CHECK_EQUAL(2.0f, dst[0]);
    CHECK_EQUAL(3.0f, dst[1]);
    CHECK_EQUAL(4.0f, dst[2]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_load_store_f3a) {
    const float src[] __attribute__((aligned(16))) = { 2.0f, 3.0f, 4.0f };
    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f };
    BLVec v = bl_vec_load_f3a(src);
    bl_vec_store_f3a(dst, v);
    CHECK_EQUAL(2.0f, dst[0]);
    CHECK_EQUAL(3.0f, dst[1]);
    CHECK_EQUAL(4.0f, dst[2]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_load_store_f4a) {
    const float src[] __attribute__((aligned(16))) = { 2.0f, 3.0f, 4.0f, 5.0f };
    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
    BLVec v = bl_vec_load_f4a(src);
    bl_vec_store_f4a(dst, v);
    CHECK_EQUAL(2.0f, dst[0]);
    CHECK_EQUAL(3.0f, dst[1]);
    CHECK_EQUAL(4.0f, dst[2]);
    CHECK_EQUAL(5.0f, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_load_store_i4) {
    const uint32_t src[] __attribute__((aligned(16))) = { 2, 3, 4, 5 };
    uint32_t dst[] __attribute__((aligned(16))) = { 0, 0, 0, 0 };
    BLVec v = bl_vec_load_i4(src);
    bl_vec_store_i4(dst, v);
    CHECK_EQUAL(2, dst[0]);
    CHECK_EQUAL(3, dst[1]);
    CHECK_EQUAL(4, dst[2]);
    CHECK_EQUAL(5, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_load_store_i4a) {
    const uint32_t src[] __attribute__((aligned(16))) = { 2, 3, 4, 5 };
    uint32_t dst[] __attribute__((aligned(16))) = { 0, 0, 0, 0 };
    BLVec v = bl_vec_load_i4a(src);
    bl_vec_store_i4a(dst, v);
    CHECK_EQUAL(2, dst[0]);
    CHECK_EQUAL(3, dst[1]);
    CHECK_EQUAL(4, dst[2]);
    CHECK_EQUAL(5, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_set) {
    BLVec v = bl_vec_set_f(2.0f, 3.0f, 4.0f, 5.0f);
    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
    bl_vec_store_f4a(dst, v);
    CHECK_EQUAL(2.0f, dst[0]);
    CHECK_EQUAL(3.0f, dst[1]);
    CHECK_EQUAL(4.0f, dst[2]);
    CHECK_EQUAL(5.0f, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_splat_scalar) {
    BLVec v = bl_vec_splat_scalar(3.0f);
    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
    bl_vec_store_f4a(dst, v);
    CHECK_EQUAL(3.0f, dst[0]);
    CHECK_EQUAL(3.0f, dst[1]);
    CHECK_EQUAL(3.0f, dst[2]);
    CHECK_EQUAL(3.0f, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_make_zero) {
    BLVec v = bl_vec_make_zero();
    float dst[] __attribute__((aligned(16))) = { 2.0f, 2.0f, 2.0f, 2.0f };
    bl_vec_store_f4a(dst, v);
    CHECK_EQUAL(0.0f, dst[0]);
    CHECK_EQUAL(0.0f, dst[1]);
    CHECK_EQUAL(0.0f, dst[2]);
    CHECK_EQUAL(0.0f, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_make_axis_x) {
    BLVec v = bl_vec_make_axis_x();
    float dst[] __attribute__((aligned(16))) = { 2.0f, 2.0f, 2.0f, 2.0f };
    bl_vec_store_f4a(dst, v);
    CHECK_EQUAL(1.0f, dst[0]);
    CHECK_EQUAL(0.0f, dst[1]);
    CHECK_EQUAL(0.0f, dst[2]);
    CHECK_EQUAL(0.0f, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_make_axis_y) {
    BLVec v = bl_vec_make_axis_y();
    float dst[] __attribute__((aligned(16))) = { 2.0f, 2.0f, 2.0f, 2.0f };
    bl_vec_store_f4a(dst, v);
    CHECK_EQUAL(0.0f, dst[0]);
    CHECK_EQUAL(1.0f, dst[1]);
    CHECK_EQUAL(0.0f, dst[2]);
    CHECK_EQUAL(0.0f, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_make_axis_z) {
    BLVec v = bl_vec_make_axis_z();
    float dst[] __attribute__((aligned(16))) = { 2.0f, 2.0f, 2.0f, 2.0f };
    bl_vec_store_f4a(dst, v);
    CHECK_EQUAL(0.0f, dst[0]);
    CHECK_EQUAL(0.0f, dst[1]);
    CHECK_EQUAL(1.0f, dst[2]);
    CHECK_EQUAL(0.0f, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_make_axis_w) {
    BLVec v = bl_vec_make_axis_w();
    float dst[] __attribute__((aligned(16))) = { 2.0f, 2.0f, 2.0f, 2.0f };
    bl_vec_store_f4a(dst, v);
    CHECK_EQUAL(0.0f, dst[0]);
    CHECK_EQUAL(0.0f, dst[1]);
    CHECK_EQUAL(0.0f, dst[2]);
    CHECK_EQUAL(1.0f, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_make_splat_x) {
    BLVec v = bl_vec_set_f(2.0f, 3.0f, 4.0f, 5.0f);
    v = bl_vec_splat_x(v);
    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
    bl_vec_store_f4a(dst, v);
    CHECK_EQUAL(2.0f, dst[0]);
    CHECK_EQUAL(2.0f, dst[1]);
    CHECK_EQUAL(2.0f, dst[2]);
    CHECK_EQUAL(2.0f, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_make_splat_y) {
    BLVec v = bl_vec_set_f(2.0f, 3.0f, 4.0f, 5.0f);
    v = bl_vec_splat_y(v);
    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
    bl_vec_store_f4a(dst, v);
    CHECK_EQUAL(3.0f, dst[0]);
    CHECK_EQUAL(3.0f, dst[1]);
    CHECK_EQUAL(3.0f, dst[2]);
    CHECK_EQUAL(3.0f, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_make_splat_z) {
    BLVec v = bl_vec_set_f(2.0f, 3.0f, 4.0f, 5.0f);
    v = bl_vec_splat_z(v);
    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
    bl_vec_store_f4a(dst, v);
    CHECK_EQUAL(4.0f, dst[0]);
    CHECK_EQUAL(4.0f, dst[1]);
    CHECK_EQUAL(4.0f, dst[2]);
    CHECK_EQUAL(4.0f, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_make_splat_w) {
    BLVec v = bl_vec_set_f(2.0f, 3.0f, 4.0f, 5.0f);
    v = bl_vec_splat_w(v);
    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
    bl_vec_store_f4a(dst, v);
    CHECK_EQUAL(5.0f, dst[0]);
    CHECK_EQUAL(5.0f, dst[1]);
    CHECK_EQUAL(5.0f, dst[2]);
    CHECK_EQUAL(5.0f, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_add) {
    BLVec a, b, c;
    a = bl_vec_set_f(2.0f, 3.0f, -4.0f, 0.0f);
    b = bl_vec_set_f(1.0f, -2.0f, 0.0f, 4.0f);
    c = bl_vec_add(a, b);

    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
    bl_vec_store_f4a(dst, c);
    CHECK_EQUAL(3.0f, dst[0]);
    CHECK_EQUAL(1.0f, dst[1]);
    CHECK_EQUAL(-4.0f, dst[2]);
    CHECK_EQUAL(4.0f, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_and) {
    BLVec a, b, c;
    a = bl_vec_set_i(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
    b = bl_vec_set_i(0x12000000, 0x00340000, 0x00005600, 0x00000078);
    c = bl_vec_and(a, b);

    uint32_t dst[] __attribute__((aligned(16))) = { 0, 0, 0, 0 };
    bl_vec_store_i4a(dst, c);
    CHECK_EQUAL(0x12000000, dst[0]);
    CHECK_EQUAL(0x00340000, dst[1]);
    CHECK_EQUAL(0x00005600, dst[2]);
    CHECK_EQUAL(0x00000078, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_cross3) {
    BLVec a, b, c;
    a = bl_vec_set_f(1.0f, 0.0f, 0.0f, 0.0f);
    b = bl_vec_set_f(0.0f, 1.0f, 0.0f, 0.0f);
    c = bl_vec_cross3(a, b);

    float dst[] __attribute__((aligned(16))) = { 4.0f, 4.0f, 4.0f, 4.0f };
    bl_vec_store_f4a(dst, c);
    CHECK_CLOSE(0.0f, dst[0], 0.00001f);
    CHECK_CLOSE(0.0f, dst[1], 0.00001f);
    CHECK_CLOSE(1.0f, dst[2], 0.00001f);
  }

  //----------------------------------------------------------------------------
  TEST(vec_dot3) {
    BLVec a, b, c;
    a = bl_vec_set_f(1.0f, 2.0f, 3.0f, 3.0f);
    b = bl_vec_set_f(-4.0f, 4.0f, -6.0f, 4.0f);
    c = bl_vec_dot3(a, b);

    float dst[] __attribute__((aligned(16))) = { 4.0f, 4.0f, 4.0f, 4.0f };
    bl_vec_store_f4a(dst, c);
    CHECK_CLOSE(-14.0f, dst[0], 0.00001f);
  }

  //----------------------------------------------------------------------------
  TEST(vec_dot4) {
    BLVec a, b, c;
    a = bl_vec_set_f(1.0f, 2.0f, 3.0f, 3.0f);
    b = bl_vec_set_f(-4.0f, 4.0f, -6.0f, 4.0f);
    c = bl_vec_dot4(a, b);

    float dst[] __attribute__((aligned(16))) = { 4.0f, 4.0f, 4.0f, 4.0f };
    bl_vec_store_f4a(dst, c);
    CHECK_CLOSE(-2.0f, dst[0], 0.00001f);
  }

  //----------------------------------------------------------------------------
  TEST(vec_length3) {
    BLVec v, l;
    v = bl_vec_set_f(1.0f, 2.0f, 3.0f, 4.0f);
    l = bl_vec_length3(v);

    float dst[] __attribute__((aligned(16))) = { 4.0f, 4.0f, 4.0f, 4.0f };
    bl_vec_store_f4a(dst, l);
    CHECK_CLOSE(sqrtf(14.0f), dst[0], 0.00001f);
  }

  //----------------------------------------------------------------------------
  TEST(vec_length4) {
    BLVec v, l;
    v = bl_vec_set_f(1.0f, 2.0f, 3.0f, 4.0f);
    l = bl_vec_length4(v);

    float dst[] __attribute__((aligned(16))) = { 4.0f, 4.0f, 4.0f, 4.0f };
    bl_vec_store_f4a(dst, l);
    CHECK_CLOSE(sqrtf(30.0f), dst[0], 0.00001f);
  }

  //----------------------------------------------------------------------------
  TEST(vec_length_sq3) {
    BLVec v, l;
    v = bl_vec_set_f(1.0f, 2.0f, 3.0f, 4.0f);
    l = bl_vec_length_sq3(v);

    float dst[] __attribute__((aligned(16))) = { 4.0f, 4.0f, 4.0f, 4.0f };
    bl_vec_store_f4a(dst, l);
    CHECK_CLOSE(14.0f, dst[0], 0.00001f);
  }

  //----------------------------------------------------------------------------
  TEST(vec_length_sq4) {
    BLVec v, l;
    v = bl_vec_set_f(1.0f, 2.0f, 3.0f, 4.0f);
    l = bl_vec_length_sq4(v);

    float dst[] __attribute__((aligned(16))) = { 4.0f, 4.0f, 4.0f, 4.0f };
    bl_vec_store_f4a(dst, l);
    CHECK_CLOSE(30.0f, dst[0], 0.00001f);
  }

  //----------------------------------------------------------------------------
  TEST(vec_lerp) {
    BLVec a, b, c, t;
    a = bl_vec_set_f(1.0f, 1.0f, 1.0f, 1.0f);
    b = bl_vec_set_f(1.0f, 2.0f, -3.0f, 4.0f);
    t = bl_vec_set_f(0.5f, 0.0f, 1.0f, 0.5f);
    c = bl_vec_lerp(a, b, t);

    float dst[] __attribute__((aligned(16))) = { 4.0f, 4.0f, 4.0f, 4.0f };
    bl_vec_store_f4a(dst, c);
    CHECK_CLOSE(1.0f, dst[0], 0.00001f);
    CHECK_CLOSE(1.0f, dst[1], 0.00001f);
    CHECK_CLOSE(-3.0f, dst[2], 0.00001f);
    CHECK_CLOSE(2.5f, dst[3], 0.00001f);
  }

  //----------------------------------------------------------------------------
  TEST(vec_madd) {
    BLVec a, b, c, d;
    a = bl_vec_set_f(1.0f, 2.0f, 3.0f, 4.0f);
    b = bl_vec_set_f(5.0f, 6.0f, 7.0f, 8.0f);
    c = bl_vec_set_f(1.0f, 2.0f, 3.0f, 4.0f);
    d = bl_vec_madd(a, b, c);

    float dst[] __attribute__((aligned(16))) = { 4.0f, 4.0f, 4.0f, 4.0f };
    bl_vec_store_f4a(dst, d);
    CHECK_CLOSE(6.0f, dst[0], 0.00001f);
    CHECK_CLOSE(14.0f, dst[1], 0.00001f);
    CHECK_CLOSE(24.0f, dst[2], 0.00001f);
    CHECK_CLOSE(36.0f, dst[3], 0.00001f);
  }

  //----------------------------------------------------------------------------
  TEST(vec_max) {
    BLVec a, b, c;
    a = bl_vec_set_f(1.0f, 2.0f, 3.0f, 4.0f);
    b = bl_vec_set_f(-1.0f, 0.5f, 7.1f, 2.0f);
    c = bl_vec_max(a, b);

    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
    bl_vec_store_f4a(dst, c);
    CHECK_CLOSE(1.0f, dst[0], 0.00001f);
    CHECK_CLOSE(2.0f, dst[1], 0.00001f);
    CHECK_CLOSE(7.1f, dst[2], 0.00001f);
    CHECK_CLOSE(4.0f, dst[3], 0.00001f);
  }

  //----------------------------------------------------------------------------
  TEST(vec_min) {
    BLVec a, b, c;
    a = bl_vec_set_f(1.0f, 2.0f, 3.0f, 4.0f);
    b = bl_vec_set_f(-1.0f, 0.5f, 7.1f, 2.0f);
    c = bl_vec_min(a, b);

    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
    bl_vec_store_f4a(dst, c);
    CHECK_CLOSE(-1.0f, dst[0], 0.00001f);
    CHECK_CLOSE(0.5f, dst[1], 0.00001f);
    CHECK_CLOSE(3.0f, dst[2], 0.00001f);
    CHECK_CLOSE(2.0f, dst[3], 0.00001f);
  }

  //----------------------------------------------------------------------------
  TEST(vec_mul) {
    BLVec a, b, c;
    a = bl_vec_set_f(1.0f, 2.0f, 3.0f, 4.0f);
    b = bl_vec_set_f(-1.0f, 0.5f, 7.1f, 2.0f);
    c = bl_vec_mul(a, b);

    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
    bl_vec_store_f4a(dst, c);
    CHECK_CLOSE(-1.0f, dst[0], 0.00001f);
    CHECK_CLOSE(1.0f, dst[1], 0.00001f);
    CHECK_CLOSE(21.3f, dst[2], 0.00001f);
    CHECK_CLOSE(8.0f, dst[3], 0.00001f);
  }

  //----------------------------------------------------------------------------
  TEST(vec_neg) {
    BLVec a, b;
    a = bl_vec_set_f(1.0f, -2.0f, 3.0f, 0.0f);
    b = bl_vec_neg(a);

    float dst[] __attribute__((aligned(16))) = { 8.0f, 8.0f, 8.0f, 8.0f };
    bl_vec_store_f4a(dst, b);
    CHECK_CLOSE(-1.0f, dst[0], 0.00001f);
    CHECK_CLOSE(2.0f, dst[1], 0.00001f);
    CHECK_CLOSE(-3.0f, dst[2], 0.00001f);
    CHECK_CLOSE(0.0f, dst[3], 0.00001f);
  }

  //----------------------------------------------------------------------------
  TEST(vec_or) {
    BLVec a, b, c;
    a = bl_vec_set_i(0x12345678, 0x12345678, 0x12345678, 0x12345678);
    b = bl_vec_set_i(0x12000000, 0x00340000, 0x00005600, 0x00000078);
    c = bl_vec_or(a, b);

    uint32_t dst[] __attribute__((aligned(16))) = { 0, 0, 0, 0 };
    bl_vec_store_i4a(dst, c);
    CHECK_EQUAL(0x12345678, dst[0]);
    CHECK_EQUAL(0x12345678, dst[1]);
    CHECK_EQUAL(0x12345678, dst[2]);
    CHECK_EQUAL(0x12345678, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_recip) {
    BLVec a, b;
    a = bl_vec_set_f(1.0f, 0.5f, 0.25f, 8.0f);
    b = bl_vec_recip(a);

    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
    bl_vec_store_f4a(dst, b);
    CHECK_CLOSE(1.0f, dst[0], 0.001f);
    CHECK_CLOSE(2.0f, dst[1], 0.001f);
    CHECK_CLOSE(4.0f, dst[2], 0.001f);
    CHECK_CLOSE(0.125f, dst[3], 0.001f);
  }

  //----------------------------------------------------------------------------
  TEST(vec_rsqrt) {
    BLVec a, b;
    a = bl_vec_set_f(1.0f, 4.0f, 16.0f, 64.0f);
    b = bl_vec_rsqrt(a);

    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
    bl_vec_store_f4a(dst, b);
    CHECK_CLOSE(1.0f, dst[0], 0.001f);
    CHECK_CLOSE(0.5f, dst[1], 0.001f);
    CHECK_CLOSE(0.25f, dst[2], 0.001f);
    CHECK_CLOSE(0.125f, dst[3], 0.001f);
  }

  //----------------------------------------------------------------------------
  TEST(vec_sub) {
    BLVec a, b, c;
    a = bl_vec_set_f(2.0f, 3.0f, -4.0f, 0.0f);
    b = bl_vec_set_f(1.0f, -2.0f, 0.0f, 4.0f);
    c = bl_vec_sub(a, b);

    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
    bl_vec_store_f4a(dst, c);
    CHECK_EQUAL(1.0f, dst[0]);
    CHECK_EQUAL(5.0f, dst[1]);
    CHECK_EQUAL(-4.0f, dst[2]);
    CHECK_EQUAL(-4.0f, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_sqrt) {
    BLVec a, b;
    a = bl_vec_set_f(1.0f, 4.0f, 16.0f, 64.0f);
    b = bl_vec_sqrt(a);

    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
    bl_vec_store_f4a(dst, b);
    CHECK_CLOSE(1.0f, dst[0], 0.002f);
    CHECK_CLOSE(2.0f, dst[1], 0.002f);
    CHECK_CLOSE(4.0f, dst[2], 0.002f);
    CHECK_CLOSE(8.0f, dst[3], 0.002f);
  }

  //----------------------------------------------------------------------------
  TEST(vec_cmp_eq) {
    BLVec a, b, c;
    a = bl_vec_set_f(1.0f, 2.0f, 3.0f, 4.0f);
    b = bl_vec_set_f(2.0f, 2.0f, 0.0f, 4.0f);
    c = bl_vec_cmp_eq(a, b);

    uint32_t dst[] __attribute__((aligned(16))) = { 1, 1, 1, 1 };
    bl_vec_store_i4a(dst, c);
    CHECK_EQUAL(0x00000000, dst[0]);
    CHECK_EQUAL(0xffffffff, dst[1]);
    CHECK_EQUAL(0x00000000, dst[2]);
    CHECK_EQUAL(0xffffffff, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_cmp_ge) {
    BLVec a, b, c;
    a = bl_vec_set_f(1.0f, 2.0f, 3.0f, 4.0f);
    b = bl_vec_set_f(2.0f, 2.0f, 0.0f, 4.0f);
    c = bl_vec_cmp_ge(a, b);

    uint32_t dst[] __attribute__((aligned(16))) = { 1, 1, 1, 1 };
    bl_vec_store_i4a(dst, c);
    CHECK_EQUAL(0x00000000, dst[0]);
    CHECK_EQUAL(0xffffffff, dst[1]);
    CHECK_EQUAL(0xffffffff, dst[2]);
    CHECK_EQUAL(0xffffffff, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_cmp_gt) {
    BLVec a, b, c;
    a = bl_vec_set_f(1.0f, 2.0f, 3.0f, 4.0f);
    b = bl_vec_set_f(2.0f, 2.0f, 0.0f, 4.0f);
    c = bl_vec_cmp_gt(a, b);

    uint32_t dst[] __attribute__((aligned(16))) = { 1, 1, 1, 1 };
    bl_vec_store_i4a(dst, c);
    CHECK_EQUAL(0x00000000, dst[0]);
    CHECK_EQUAL(0x00000000, dst[1]);
    CHECK_EQUAL(0xffffffff, dst[2]);
    CHECK_EQUAL(0x00000000, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_cmp_le) {
    BLVec a, b, c;
    a = bl_vec_set_f(1.0f, 2.0f, 3.0f, 4.0f);
    b = bl_vec_set_f(2.0f, 2.0f, 0.0f, 4.0f);
    c = bl_vec_cmp_le(a, b);

    uint32_t dst[] __attribute__((aligned(16))) = { 1, 1, 1, 1 };
    bl_vec_store_i4a(dst, c);
    CHECK_EQUAL(0xffffffff, dst[0]);
    CHECK_EQUAL(0xffffffff, dst[1]);
    CHECK_EQUAL(0x00000000, dst[2]);
    CHECK_EQUAL(0xffffffff, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_cmp_lt) {
    BLVec a, b, c;
    a = bl_vec_set_f(1.0f, 2.0f, 3.0f, 4.0f);
    b = bl_vec_set_f(2.0f, 2.0f, 0.0f, 4.0f);
    c = bl_vec_cmp_lt(a, b);

    uint32_t dst[] __attribute__((aligned(16))) = { 1, 1, 1, 1 };
    bl_vec_store_i4a(dst, c);
    CHECK_EQUAL(0xffffffff, dst[0]);
    CHECK_EQUAL(0x00000000, dst[1]);
    CHECK_EQUAL(0x00000000, dst[2]);
    CHECK_EQUAL(0x00000000, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_cmp_neq) {
    BLVec a, b, c;
    a = bl_vec_set_f(1.0f, 2.0f, 3.0f, 4.0f);
    b = bl_vec_set_f(2.0f, 2.0f, 0.0f, 4.0f);
    c = bl_vec_cmp_neq(a, b);

    uint32_t dst[] __attribute__((aligned(16))) = { 1, 1, 1, 1 };
    bl_vec_store_i4a(dst, c);
    CHECK_EQUAL(0xffffffff, dst[0]);
    CHECK_EQUAL(0x00000000, dst[1]);
    CHECK_EQUAL(0xffffffff, dst[2]);
    CHECK_EQUAL(0x00000000, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_select) {
    BLVec a, b, c, mask;
    a     = bl_vec_set_f(1.0f, 2.0f, 3.0f, 4.0f);
    b     = bl_vec_set_f(2.0f, 2.0f, 0.0f, 4.0f);
    mask  = bl_vec_cmp_gt(a, b);
    c     = bl_vec_select(a, b, mask);

    float dst[] __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
    bl_vec_store_f4a(dst, c);
    CHECK_EQUAL(1.0f, dst[0]);
    CHECK_EQUAL(2.0f, dst[1]);
    CHECK_EQUAL(0.0f, dst[2]);
    CHECK_EQUAL(4.0f, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_shuffle32) {
    BLVec a, b, c, shuf_dACb;
    a         = bl_vec_set_i(1, 2, 3, 4);
    b         = bl_vec_set_i(5, 6, 7, 8);
    shuf_dACb = BL_VEC_SHUFFLE_MASK_32(d,A,C,b);
    c         = bl_vec_shuffle(a, b, shuf_dACb);
    
    uint32_t dst[] __attribute__((aligned(16))) = { 0, 0, 0, 0 };
    bl_vec_store_i4a(dst, c);
    CHECK_EQUAL(8, dst[0]);
    CHECK_EQUAL(1, dst[1]);
    CHECK_EQUAL(3, dst[2]);
    CHECK_EQUAL(6, dst[3]);
  }

  //----------------------------------------------------------------------------
  TEST(vec_shuffle16) {
    BLVec a, b, c, shuf_hGAd_fCcE;
    a               = bl_vec_set_i16(1, 2, 3, 4, 5, 6, 7, 8);
    b               = bl_vec_set_i16(9, 10, 11, 12, 13, 14, 15, 16);
    shuf_hGAd_fCcE  = BL_VEC_SHUFFLE_MASK_16(h,G,A,d, f,C,c,E);
    c               = bl_vec_shuffle(a, b, shuf_hGAd_fCcE);
    
    uint16_t dst[] __attribute__((aligned(16))) = { 0, 0, 0, 0, 0, 0, 0, 0 };
    bl_vec_store_i16a(dst, c);
    CHECK_EQUAL(16, dst[0]);
    CHECK_EQUAL(7,  dst[1]);
    CHECK_EQUAL(1,  dst[2]);
    CHECK_EQUAL(12, dst[3]);
    CHECK_EQUAL(14, dst[4]);
    CHECK_EQUAL(3,  dst[5]);
    CHECK_EQUAL(11, dst[6]);
    CHECK_EQUAL(5,  dst[7]);
  }
  
  //----------------------------------------------------------------------------
  TEST(vec_shuffle8) {
    BLVec a, b, c, shuf_pOcA_LNop_hGAd_fCcE;
    a                         = bl_vec_set_i8(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    b                         = bl_vec_set_i8(17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32);
    shuf_pOcA_LNop_hGAd_fCcE  = BL_VEC_SHUFFLE_MASK_8(p,O,c,A, L,N,o,p, h,G,A,d, f,C,c,E);
    c                         = bl_vec_shuffle(a, b, shuf_pOcA_LNop_hGAd_fCcE);
    
    uint8_t dst[] __attribute__((aligned(16))) = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    bl_vec_store_i8a(dst, c);
    CHECK_EQUAL(32, dst[0]);
    CHECK_EQUAL(15, dst[1]);
    CHECK_EQUAL(19, dst[2]);
    CHECK_EQUAL(1,  dst[3]);
    CHECK_EQUAL(12, dst[4]);
    CHECK_EQUAL(14, dst[5]);
    CHECK_EQUAL(31, dst[6]);
    CHECK_EQUAL(32, dst[7]);
    CHECK_EQUAL(24, dst[8]);
    CHECK_EQUAL(7,  dst[9]);
    CHECK_EQUAL(1,  dst[10]);
    CHECK_EQUAL(20, dst[11]);
    CHECK_EQUAL(22, dst[12]);
    CHECK_EQUAL(3,  dst[13]);
    CHECK_EQUAL(19, dst[14]);
    CHECK_EQUAL(5,  dst[15]);
  }
}
