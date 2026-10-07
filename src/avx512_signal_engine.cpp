#include "photon/avx512_signal_engine.hpp"
#include <algorithm>
#include <cmath>
#include <immintrin.h>

namespace photon::signal {

#if defined(__x86_64__) || defined(_M_X64)
__attribute__((target("avx512f")))
static float imbalance_avx512(const Book8& b) noexcept {
  constexpr __mmask16 M = 0x00ff;
  const __m512 bid = _mm512_maskz_loadu_ps(M, b.bid_qty.data());
  const __m512 ask = _mm512_maskz_loadu_ps(M, b.ask_qty.data());
  const __m512 den = _mm512_add_ps(bid, ask);
  const __m512 num = _mm512_sub_ps(bid, ask);
  const __m512 safe = _mm512_max_ps(den, _mm512_set1_ps(1.0e-12f));
  const __m512 x = _mm512_div_ps(num, safe);
  return _mm512_reduce_add_ps(x) * 0.125f;
}

__attribute__((target("avx512f")))
static float max_imbalance_avx512(const Book8& b) noexcept {
  constexpr __mmask16 M = 0x00ff;
  const __m512 bid = _mm512_maskz_loadu_ps(M, b.bid_qty.data());
  const __m512 ask = _mm512_maskz_loadu_ps(M, b.ask_qty.data());
  const __m512 den = _mm512_add_ps(bid, ask);
  const __m512 num = _mm512_sub_ps(bid, ask);
  const __m512 safe = _mm512_max_ps(den, _mm512_set1_ps(1.0e-12f));
  const __m512 x = _mm512_div_ps(num, safe);
  return _mm512_reduce_max_ps(x);
}

__attribute__((target("avx512f")))
static float weighted_imbalance_avx512(const Book8& b) noexcept {
  constexpr __mmask16 M = 0x00ff;
  const __m512 bid = _mm512_maskz_loadu_ps(M, b.bid_qty.data());
  const __m512 ask = _mm512_maskz_loadu_ps(M, b.ask_qty.data());
  const __m512 levels = _mm512_set_ps(0,0,0,0,0,0,0,0,8,7,6,5,4,3,2,1);
  const __m512 num = _mm512_sub_ps(bid, ask);
  const __m512 den = _mm512_max_ps(_mm512_add_ps(bid, ask), _mm512_set1_ps(1e-12f));
  const __m512 w = _mm512_div_ps(levels, _mm512_set1_ps(36.0f));
  return _mm512_reduce_add_ps(_mm512_mul_ps(_mm512_div_ps(num, den), w));
}
#endif

static float scalar_imbalance(const Book8& b) noexcept {
  float sum = 0.0f;
  for (std::size_t i=0; i<8; ++i) {
    const float den = b.bid_qty[i] + b.ask_qty[i];
    sum += (b.bid_qty[i] - b.ask_qty[i]) / std::max(den, 1.0e-12f);
  }
  return sum * 0.125f;
}

static float scalar_max(const Book8& b) noexcept {
  float m = -1.0f;
  for (std::size_t i=0; i<8; ++i) {
    const float den = b.bid_qty[i] + b.ask_qty[i];
    m = std::max(m, (b.bid_qty[i] - b.ask_qty[i]) / std::max(den, 1.0e-12f));
  }
  return m;
}

float imbalance(const Book8& b) noexcept {
#if defined(__x86_64__) || defined(_M_X64)
  return __builtin_cpu_supports("avx512f") ? imbalance_avx512(b) : scalar_imbalance(b);
#else
  return scalar_imbalance(b);
#endif
}

float max_imbalance(const Book8& b) noexcept {
#if defined(__x86_64__) || defined(_M_X64)
  return __builtin_cpu_supports("avx512f") ? max_imbalance_avx512(b) : scalar_max(b);
#else
  return scalar_max(b);
#endif
}

float weighted_imbalance(const Book8& b) noexcept {
#if defined(__x86_64__) || defined(_M_X64)
  if (__builtin_cpu_supports("avx512f")) return weighted_imbalance_avx512(b);
#endif
  float r = 0.0f;
  for (std::size_t i=0; i<8; ++i) {
    const float den = std::max(b.bid_qty[i] + b.ask_qty[i], 1e-12f);
    r += ((b.bid_qty[i] - b.ask_qty[i]) / den) * (float(i + 1) / 36.0f);
  }
  return r;
}
} // namespace photon::signal
