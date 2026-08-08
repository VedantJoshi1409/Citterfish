#include "attacks.h"
#include "types.h"
#include <array>

namespace citterfish::attacks {
namespace {
void initializeKnightAttacks() {
  for (int square = 0; square < 64; ++square) {
    Bitboard attacks = 0;
    int rank = square / 8;
    int file = square % 8;

    if (rank < 7) {   // Can do up left left or up right right
      if (file > 1) { // up left left
        attacks |= 1ULL << (square + NORTH + WEST + WEST);
      }
      if (file < 6) { // up right right
        attacks |= 1ULL << (square + NORTH + EAST + EAST);
      }
      if (rank < 6) {   // Can do up up left or up up right
        if (file > 0) { // up up left
          attacks |= 1ULL << (square + NORTH + NORTH + WEST);
        }
        if (file < 7) { // up up right
          attacks |= 1ULL << (square + NORTH + NORTH + EAST);
        }
      }
    }
    if (rank > 0) {   // Can do down left left or down right right
      if (file > 1) { // down left left
        attacks |= 1ULL << (square + SOUTH + WEST + WEST);
      }
      if (file < 6) { // down right right
        attacks |= 1ULL << (square + SOUTH + EAST + EAST);
      }
      if (rank > 1) {   // Can do down down left or down down right
        if (file > 0) { // down down left
          attacks |= 1ULL << (square + SOUTH + SOUTH + WEST);
        }
        if (file < 7) { // down down right
          attacks |= 1ULL << (square + SOUTH + SOUTH + EAST);
        }
      }
    }
    detail::knightAttacks[static_cast<int>(square)] = attacks;
  }
}

void initializeKingAttacks() {
  for (int square = 0; square < 64; ++square) {
    Bitboard attacks = 0;
    int rank = square / 8;
    int file = square % 8;

    if (rank < 7) {   // Can do up left, up, up right
      if (file > 0) { // up left
        attacks |= 1ULL << (square + NORTH + WEST);
      }
      attacks |= 1ULL << (square + NORTH); // up
      if (file < 7) {                      // up right
        attacks |= 1ULL << (square + NORTH + EAST);
      }
    }
    if (file > 0) { // Can do left
      attacks |= 1ULL << (square + WEST);
    }
    if (file < 7) { // Can do right
      attacks |= 1ULL << (square + EAST);
    }
    if (rank > 0) {   // Can do down left, down, down right
      if (file > 0) { // down left
        attacks |= 1ULL << (square + SOUTH + WEST);
      }
      attacks |= 1ULL << (square + SOUTH); // down
      if (file < 7) {                      // down right
        attacks |= 1ULL << (square + SOUTH + EAST);
      }
    }
    detail::kingAttacks[static_cast<int>(square)] = attacks;
  }
}
} // namespace

void initializeAttacks() {
  initializeKnightAttacks();
  initializeKingAttacks();
  fillAttackMap<ROOK>();
  fillAttackMap<BISHOP>();
}

} // namespace citterfish::attacks
