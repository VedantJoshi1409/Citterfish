#include "zobrist.h"
#include <array>

namespace citterfish::zobrist {

bool init_zobrist(uint64_t seed) {
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
  for (Square square = a1; square <= h8; ++square) {
    detail::enPassantKeys[square] = rng.next();
  }
  detail::enPassantKeys[NO_SQUARE] = 0;
  detail::sideToMoveKey[WHITE] = rng.next();
  detail::sideToMoveKey[BLACK] = 0;
  return true;
}
} // namespace citterfish::zobrist