#include "photon/avx512_signal_engine.hpp"
#include <immintrin.h>

namespace photon::signal {
float imbalance(const Book8& b) noexcept {
  const __m256 bid = _mm256_load_ps(b.bid_qty.data());
  const __m256 ask = _mm256_load_ps(b.ask_qty.data());
  const __m256 den = _mm256_add_ps(bid, ask);
  const __m256 num = _mm256_sub_ps(bid, ask);
  const __m256 eps = _mm256_set1_ps(1.0e-12f);
  const __m256 safe = _mm256_max_ps(den, eps);
  const __m256 x = _mm256_div_ps(num, safe);
  alignas(32) float v[8];
  _mm256_store_ps(v, x);
  float sum = 0.0f;
  for (float q : v) sum += q;
  return sum / 8.0f;
}
float max_imbalance(const Book8& b) noexcept {
  const __m256 bid = _mm256_load_ps(b.bid_qty.data());
  const __m256 ask = _mm256_load_ps(b.ask_qty.data());
  const __m256 den = _mm256_add_ps(bid, ask);
  const __m256 num = _mm256_sub_ps(bid, ask);
  const __m256 safe = _mm256_max_ps(den, _mm256_set1_ps(1.0e-12f));
  const __m256 x = _mm256_div_ps(num, safe);
  alignas(32) float v[8];
  _mm256_store_ps(v, x);
  float m = -1.0f;
  for (float q : v) if (q > m) m = q;
  return m;
}
} // namespace photon::signal
