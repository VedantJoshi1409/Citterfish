#include "zobrist.h"
#include <array>

namespace citterfish::zobrist {
namespace {
std::array<std::array<std::array<Key, 64>, 6>, 2> pieceKeys;
std::array<Key, 16> castlingKeys;
std::array<Key, 64> enPassantKeys;
Key sideToMoveKey;
} // namespace

bool initializeZobristKeys(uint64_t seed) {
  PRNG rng(seed);
  for (auto &colorArray : pieceKeys) {
    for (auto &pieceArray : colorArray) {
      for (auto &key : pieceArray) {
        key = rng.next();
      }
    }
  }
  for (auto &key : castlingKeys) {
    key = rng.next();
  }
  for (auto &key : enPassantKeys) {
    key = rng.next();
  }
  sideToMoveKey = rng.next();
  return true;
}

Key getPieceKey(Color color, Piece piece, Square square) {
  return pieceKeys[color][piece][square];
}
Key getCastlingKey(uint8_t rights) { return castlingKeys[rights]; }
Key getEnPassantKey(Square square) { return enPassantKeys[square]; }
Key getSideToMoveKey() { return sideToMoveKey; }
} // namespace citterfish::zobrist