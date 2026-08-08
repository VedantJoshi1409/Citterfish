#include "magics.h"
#include "types.h"
#include <array>
#include <fstream>
#include <string>

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
void fillMagicSearchTable(Square square, Bitboard mask,
                          std::array<Bitboard, 4096> &occupancyTable,
                          std::array<Bitboard, 4096> &attackTable) {
  int bitcount = std::popcount(mask);
  int tableSize = 1 << bitcount;
  for (int i = 0; i < tableSize; ++i) {
    occupancyTable[i] = PDEP(i, mask); // compute occupancies
    attackTable[i] =
        (P == ROOK) ? slowRookAttack(square, occupancyTable[i])
                    : slowBishopAttack(
                          square, occupancyTable[i]); // find slow attack mask
                                                      // for each occupancy
  }
}

} // namespace

template <Piece P> void genMagicsArray() {
  std::ifstream in((P == ROOK) ? "data/rook_magics.txt"
                               : "data/bishop_magics.txt");
  std::string tok;
  uint32_t tableIdx = 0;

  for (Square square = a1; square <= h8; ++square) {
    in >> tok;
    uint64_t magic = std::stoull(tok, nullptr, 0);
    Bitboard mask = (P == ROOK) ? getRookMask(square) : getBishopMask(square);
    uint32_t curIdx = tableIdx;
    uint32_t shift = 0;
    int bitcount = std::popcount(mask);
    int tableSize = 1 << bitcount;

    std::array<Bitboard, 4096> map{
        0}; // map between the hashed idx and the attack masks
    std::array<Bitboard, 4096>
        occupancyTable; // contains <tableSize> occupancy masks
    std::array<Bitboard, 4096>
        attackTable; // contains the attackMask for each occupancy
    fillMagicSearchTable<P>(square, mask, occupancyTable, attackTable);
    bool failed = false;
    for (int shiftOffset = 0; shiftOffset < bitcount && !failed;
         ++shiftOffset) { // trying to find best magics
      int constructiveCollisions = 0;
      uint64_t maxIdx = 0;
      for (int occIdx = 0; occIdx < tableSize;
           ++occIdx) { // loop through each possible occupancy mask
        uint64_t idx =
            (occupancyTable[occIdx] * magic) >> (64 - bitcount + shiftOffset);
        maxIdx = std::max(maxIdx, idx);
        if (map[idx] == 0) { // fresh hash
          map[idx] = attackTable[occIdx];
        } else if (map[idx] != attackTable[occIdx]) { // destructive collision
          failed = true;
          break;
        } else {
          ++constructiveCollisions;
        }
      }
      if (!failed) {
        shift = shiftOffset;
        tableIdx = curIdx + maxIdx;
      } else if (shiftOffset == 0) {
        std::cout << "FAIL" << std::endl;
      }
      map.fill(0); // if checking for best magic, reset
    }
    std::cout << "{" << mask << "ULL, " << magic << "ULL, " << curIdx << ", "
              << 64 - bitcount + shift << "}," << std::endl;
    ++tableIdx;
  }
  --tableIdx;
  std::cout << "Size: " << tableIdx << std::endl;
}
template void genMagicsArray<ROOK>();
template void genMagicsArray<BISHOP>();

template <Piece P> void fillAttackMap() {
  for (Square square = a1; square <= h8; ++square) {
    Magic m = (P == ROOK) ? RookMagics[square] : BishopMagics[square];
    int bitCount = std::popcount(m.mask);
    for (int i = 0; i < (1ULL << bitCount); i++) {
      Bitboard occupancy = PDEP(i, m.mask);
      uint64_t hashIdx = ((occupancy * m.magic) >> m.shift) + m.idx;
      uint64_t attacks = (P == ROOK) ? slowRookAttack(square, occupancy)
                                     : slowBishopAttack(square, occupancy);
      ((P == ROOK) ? RookAttackTable[hashIdx] : BishopAttackTable[hashIdx]) =
          attacks;
    }
  }
}
template void fillAttackMap<ROOK>();
template void fillAttackMap<BISHOP>();

template <Piece P> bool verifyAttackMap() {
  for (Square square = a1; square <= h8; ++square) {
    Magic m = (P == ROOK) ? RookMagics[square] : BishopMagics[square];
    int bitCount = std::popcount(m.mask);
    for (int i = 0; i < (1ULL << bitCount); i++) {
      Bitboard occupancy = PDEP(i, m.mask);

      uint64_t hashIdx = ((occupancy * m.magic) >> m.shift) + m.idx;
      uint64_t attacks = (P == ROOK) ? slowRookAttack(square, occupancy)
                                     : slowBishopAttack(square, occupancy);
      if (((P == ROOK) ? RookAttackTable[hashIdx]
                       : BishopAttackTable[hashIdx]) != attacks) {
        return false;
      }
    }
  }
  return true;
}
template bool verifyAttackMap<ROOK>();
template bool verifyAttackMap<BISHOP>();

} // namespace citterfish