// Copyright (c) 2024 The mlkem-native project authors
// SPDX-License-Identifier: Apache-2.0
#include "verify.h"
#include <stddef.h>
#include <stdint.h>

//
// WARNING:
//
// The functions in this compilation unit may be susceptible to
// compiler-induced variable-time code when inlined into their call-sites.
// The purpose of having a separate compilation here is to prevent
// such potentially insecure inlining.
//
// You MUST NOT compile this file using link time optimization.
//

int verify(const uint8_t *a, const uint8_t *b, const size_t len) {
  uint8_t r = 0;
  uint64_t u;

  // Switch to a _signed_ ilen value, so that our loop counter
  // can also be signed, and thus (i - 1) in the loop invariant
  // can yield -1 as required.
  const int ilen = (int)len;

  for (int i = 0; i < ilen; i++)  // clang-format off
    INVARIANT(i >= 0 && i <= ilen)
    INVARIANT((r == 0) == (FORALL(int, k, 0, (i - 1), (a[k] == b[k]))))  // clang-format on
    {
      r |= a[i] ^ b[i];
    }

#ifdef CBMC
#pragma CPROVER check push
#pragma CPROVER check disable "unsigned-overflow"
#endif
  u = (-(uint64_t)r) >> 63;
#ifdef CBMC
#pragma CPROVER check pop
#endif

  return (int)u;
}

void cmov(uint8_t *r, const uint8_t *x, size_t len, uint8_t b) {
  size_t i;

  b = (-b) & 0xFF;
  for (i = 0; i < len; i++)  // clang-format off
    INVARIANT(i <= len)
    // clang-format on
    {
      r[i] ^= b & (r[i] ^ x[i]);
    }
}

/*************************************************
 * Note:
 *
 * Constant-time implementation. Relies on basic
 * properties of bitwise ^ or and &.
 **************************************************/
void cmov_int16(int16_t *r, const int16_t v, const uint16_t b) {
// CBMC issues false alarms here for the implicit conversions between
// uint16_t and int, so disable "conversion-check" here for now.
#pragma CPROVER check push
#pragma CPROVER check disable "conversion"

  // b == 0 => mask = 0x0000
  // b == 1 => mask = 0xFFFF
  const uint16_t mask = -b;

  // mask == 0x0000 => *r == (*r ^ 0x0000) == *r
  // mask == 0xFFFF => *r == (*r ^ (*r ^ v)) == (*r ^ *r) ^ v == 0 ^ v == v
  *r ^= mask & ((*r) ^ v);
#pragma CPROVER check pop
}
