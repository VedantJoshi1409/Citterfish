#pragma once

#include "types.h"
#include <array>

namespace citterfish {
struct Magic {
  Bitboard attackMask;
  uint64_t magic;
  uint32_t idx;
  uint8_t shift;
};

inline constexpr std::array<Magic, 64> RookMagics = {{}};
inline constexpr std::array<Magic, 64> BishopMagics = {{}};
} // namespace citterfish