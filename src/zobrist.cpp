#include "zobrist.h"
#include <array>

namespace citterfish::zobrist {

bool initializeZobristKeys(uint64_t seed) {
  PRNG rng(seed);
  for (auto &colorArray : detail::pieceKeys) {
    for (auto &pieceArray : colorArray) {
      for (auto &key : pieceArray) {
        key = rng.next();
      }
    }
  }
  for (auto &key : detail::castlingKeys) {
    key = rng.next();
  }
  for (auto &key : detail::enPassantKeys) {
    key = rng.next();
  }
  detail::sideToMoveKey = rng.next();
  return true;
}
} // namespace citterfish::zobrist