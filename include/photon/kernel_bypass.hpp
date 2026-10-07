#pragma once
#include <cstddef>
#include <cstdint>
#if defined(PHOTON_ENABLE_EF_VI)
#include <etherfabric/ef_vi.h>
#endif
#if defined(PHOTON_ENABLE_OPENONLOAD)
#include <onload/extensions.h>
#include <sys/uio.h>
#endif
namespace photon::net {
#if defined(PHOTON_ENABLE_EF_VI)
class EfViTx {
  ef_vi* vi_{};
public:
  explicit EfViTx(ef_vi* vi) noexcept : vi_(vi) {}
  bool submit(ef_addr dma_address, int length, ef_request_id id) noexcept {
    return vi_ != nullptr && ef_vi_transmit(vi_, dma_address, length, id) == 0;
  }
  void push() noexcept { if (vi_ != nullptr) ef_vi_transmit_push(vi_); }
  int available() const noexcept { return vi_ == nullptr ? 0 : ef_vi_transmit_space(vi_); }
};
#endif
#if defined(PHOTON_ENABLE_OPENONLOAD)
class OnloadDelegatedTcp {
public:
  static int prepare(int fd, int size, unsigned flags, onload_delegated_send& state) noexcept {
    return static_cast<int>(onload_delegated_send_prepare(fd, size, flags, &state));
  }
  static void update(onload_delegated_send& state, int bytes, bool push) noexcept {
    onload_delegated_send_tcp_update(&state, bytes, push ? 1 : 0);
  }
  static void advance(onload_delegated_send& state, int bytes) noexcept {
    onload_delegated_send_tcp_advance(&state, bytes);
  }
  static int complete(int fd, const iovec* iov, int iovlen, int flags = 0) noexcept {
    return onload_delegated_send_complete(fd, iov, iovlen, flags);
  }
  static int cancel(int fd) noexcept { return onload_delegated_send_cancel(fd); }
};
#endif
} // namespace photon::net
