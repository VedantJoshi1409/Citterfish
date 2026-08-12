#pragma once

#include "types.h"
#include <array>

namespace citterfish::zobrist {

namespace detail {
inline std::array<std::array<std::array<Key, 64>, 6>, 2> pieceKeys;
inline std::array<Key, 16> castlingKeys;
inline std::array<Key, 65> enPassantKeys; // file only — see below
inline std::array<Key, 2> sideToMoveKey;
} // namespace detail

inline Key getPieceKey(Color c, Piece p, Square s) {
  return detail::pieceKeys[c][p][s];
}
inline Key getCastlingKey(uint8_t r) { return detail::castlingKeys[r]; }
inline Key getEnPassantKey(Square square) {
  return detail::enPassantKeys[square];
}
inline Key getSideToMoveKey(Color c) { return detail::sideToMoveKey[c]; }

bool initializeZobristKeys(uint64_t seed = 0xDEADBEEFCAFEBABEULL);

} // namespace citterfish::zobrist