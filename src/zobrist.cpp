#include "zobrist.h"
#include <array>
#include <random>

namespace citterfish::zobrist {
namespace {
std::array<std::array<std::array<Key, 64>, 6>, 2> pieceKeys;
std::array<Key, 16> castlingKeys;
std::array<Key, 64> enPassantKeys;
Key sideToMoveKey;
} // namespace

bool initializeZobristKeys(uint64_t seed) {
  std::mt19937_64 rng(seed);
  for (auto &colorArray : pieceKeys) {
    for (auto &pieceArray : colorArray) {
      for (auto &key : pieceArray) {
        key = rng();
      }
    }
  }
  for (auto &key : castlingKeys) {
    key = rng();
  }
  for (auto &key : enPassantKeys) {
    key = rng();
  }
  sideToMoveKey = rng();
  return true;
}

Key getPieceKey(Color color, Piece piece, Square square) {
  return pieceKeys[color][piece][square];
}
Key getCastlingKey(uint8_t rights) { return castlingKeys[rights]; }
Key getEnPassantKey(Square square) { return enPassantKeys[square]; }
Key getSideToMoveKey() { return sideToMoveKey; }
} // namespace citterfish::zobrist