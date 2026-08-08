#include "magics.h"
#include "types.h"
#include <array>

namespace citterfish {
namespace {

Bitboard getBishopMask(Square square) {
  Bitboard mask = 0;
  int rank = square / 8;
  int file = square % 8;

  for (int r = rank + 1, f = file + 1; r < 7 && f < 7; ++r, ++f) { // Up-Right
    mask |= 1ULL << (r * 8 + f);
  }
  for (int r = rank + 1, f = file - 1; r < 7 && f > 0; ++r, --f) { // Up-Left
    mask |= 1ULL << (r * 8 + f);
  }
  for (int r = rank - 1, f = file + 1; r > 0 && f < 7; --r, ++f) { // Down-Right
    mask |= 1ULL << (r * 8 + f);
  }
  for (int r = rank - 1, f = file - 1; r > 0 && f > 0; --r, --f) { // Down-Left
    mask |= 1ULL << (r * 8 + f);
  }

  return mask;
}

Bitboard getRookMask(Square square) {
  Bitboard mask = 0;
  int rank = square / 8;
  int file = square % 8;

  for (int r = rank + 1; r < 7; ++r) { // Up
    mask |= 1ULL << (r * 8 + file);
  }
  for (int r = rank - 1; r > 0; --r) { // Down
    mask |= 1ULL << (r * 8 + file);
  }
  for (int f = file + 1; f < 7; ++f) { // Right
    mask |= 1ULL << (rank * 8 + f);
  }
  for (int f = file - 1; f > 0; --f) { // Left
    mask |= 1ULL << (rank * 8 + f);
  }

  return mask;
}

Bitboard slowRookAttack(Square square, Bitboard occupancy) {
  Bitboard attacks = 0;
  int rank = square / 8;
  int file = square % 8;

  // Up
  for (int r = rank + 1; r < 8; ++r) {
    attacks |= 1ULL << (r * 8 + file);
    if (occupancy & (1ULL << (r * 8 + file)))
      break;
  }
  // Down
  for (int r = rank - 1; r >= 0; --r) {
    attacks |= 1ULL << (r * 8 + file);
    if (occupancy & (1ULL << (r * 8 + file)))
      break;
  }
  // Right
  for (int f = file + 1; f < 8; ++f) {
    attacks |= 1ULL << (rank * 8 + f);
    if (occupancy & (1ULL << (rank * 8 + f)))
      break;
  }
  // Left
  for (int f = file - 1; f >= 0; --f) {
    attacks |= 1ULL << (rank * 8 + f);
    if (occupancy & (1ULL << (rank * 8 + f)))
      break;
  }

  return attacks;
}

Bitboard slowBishopAttack(Square square, Bitboard occupancy) {
  Bitboard attacks = 0;
  int rank = square / 8;
  int file = square % 8;

  // Up-Right
  for (int r = rank + 1, f = file + 1; r < 8 && f < 8; ++r, ++f) {
    attacks |= 1ULL << (r * 8 + f);
    if (occupancy & (1ULL << (r * 8 + f)))
      break;
  }
  // Up-Left
  for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; ++r, --f) {
    attacks |= 1ULL << (r * 8 + f);
    if (occupancy & (1ULL << (r * 8 + f)))
      break;
  }
  // Down-Right
  for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; --r, ++f) {
    attacks |= 1ULL << (r * 8 + f);
    if (occupancy & (1ULL << (r * 8 + f)))
      break;
  }
  // Down-Left
  for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; --r, --f) {
    attacks |= 1ULL << (r * 8 + f);
    if (occupancy & (1ULL << (r * 8 + f)))
      break;
  }

  return attacks;
}

Bitboard PDEP(Bitboard source, Bitboard mask) {
  Bitboard result{0};
  while (mask) {
    Bitboard lsb = mask & -mask; // Get least significant bit of mask
    if (source & 1) {
      result |= lsb; // Set the corresponding bit in result
    }
    source >>= 1;     // Shift source to process the next bit
    mask &= mask - 1; // Clear the least significant bit of mask
  }
  return result;
}

template <Piece P>
void fillMagicSearchTable(Square square, Bitboard attackMask,
                          std::array<Bitboard, 4096> &occupancyTable,
                          std::array<Bitboard, 4096> &attackTable) {
  int bitcount = std::popcount(attackMask);
  int tableSize = 1 << bitcount;
  for (int i = 0; i < tableSize; ++i) {
    occupancyTable[i] = PDEP(i, attackMask); // compute occupancies
    attackTable[i] =
        (P == ROOK) ? slowRookAttack(square, occupancyTable[i])
                    : slowBishopAttack(
                          square, occupancyTable[i]); // find slow attack mask
                                                      // for each occupancy
  }
}

template <Piece P> void genMagicsArrays() {

  for (Square square = a1; square <= h8; ++square) {
    Bitboard attackMask =
        (P == ROOK) ? getRookMask(square) : getBishopMask(square);
  }
}

} // namespace
} // namespace citterfish