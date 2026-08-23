#include "magics.h"
#include "types.h"
#include <array>
#include <fstream>
#include <string>

namespace citterfish {
namespace {

Bitboard get_bishop_mask(Square square) {
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

Bitboard get_rook_mask(Square square) {
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

Bitboard slow_rook_attack(Square square, Bitboard occupancy) {
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

Bitboard slow_bishop_attack(Square square, Bitboard occupancy) {
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
void fill_magic_search_table(Square square, Bitboard mask,
                             std::array<Bitboard, 4096> &occTable,
                             std::array<Bitboard, 4096> &attackTable) {
  int bitcount = std::popcount(mask);
  int table_size = 1 << bitcount;
  for (int i = 0; i < table_size; ++i) {
    occTable[i] = PDEP(i, mask); // compute occupancies
    attackTable[i] =
        (P == ROOK)
            ? slow_rook_attack(square, occTable[i])
            : slow_bishop_attack(square, occTable[i]); // find slow attack mask
                                                       // for each occupancy
  }
}

} // namespace

template <Piece P> void gen_magics_array() {
  std::ifstream in((P == ROOK) ? "data/rook_magics.txt"
                               : "data/bishop_magics.txt");
  std::string tok;
  uint32_t tableIdx = 0;

  for (Square square = a1; square <= h8; ++square) {
    in >> tok;
    uint64_t magic = std::stoull(tok, nullptr, 0);
    Bitboard mask =
        (P == ROOK) ? get_rook_mask(square) : get_bishop_mask(square);
    uint32_t cur_idx = tableIdx;
    uint32_t shift = 0;
    int bitcount = std::popcount(mask);
    int table_size = 1 << bitcount;

    std::array<Bitboard, 4096> map{
        0}; // map between the hashed idx and the attack masks
    std::array<Bitboard, 4096>
        occTable; // contains <table_size> occupancy masks
    std::array<Bitboard, 4096>
        attackTable; // contains the attackMask for each occupancy
    fill_magic_search_table<P>(square, mask, occTable, attackTable);
    bool failed = false;
    for (int shiftOffset = 0; shiftOffset < bitcount && !failed;
         ++shiftOffset) { // trying to find best magics
      int constructiveCollisions = 0;
      uint64_t maxIdx = 0;
      for (int occIdx = 0; occIdx < table_size;
           ++occIdx) { // loop through each possible occupancy mask
        uint64_t idx =
            (occTable[occIdx] * magic) >> (64 - bitcount + shiftOffset);
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
        tableIdx = cur_idx + maxIdx;
      } else if (shiftOffset == 0) {
        std::cout << "FAIL" << std::endl;
      }
      map.fill(0); // if checking for best magic, reset
    }
    std::cout << "{" << mask << "ULL, " << magic << "ULL, " << cur_idx << ", "
              << 64 - bitcount + shift << "}," << std::endl;
    ++tableIdx;
  }
  --tableIdx;
  std::cout << "Size: " << tableIdx << std::endl;
}
template void gen_magics_array<ROOK>();
template void gen_magics_array<BISHOP>();

template <Piece P> void fill_attack_map() {
  for (Square square = a1; square <= h8; ++square) {
    Magic m = (P == ROOK) ? ROOK_MAGICS[square] : BISHOP_MAGICS[square];
    int bitCount = std::popcount(m.mask);
    for (int i = 0; i < (1ULL << bitCount); i++) {
      Bitboard occupancy = PDEP(i, m.mask);
      uint64_t hashIdx = ((occupancy * m.magic) >> m.shift) + m.idx;
      uint64_t attacks = (P == ROOK) ? slow_rook_attack(square, occupancy)
                                     : slow_bishop_attack(square, occupancy);
      ((P == ROOK) ? ROOK_ATTACK_TABLE[hashIdx]
                   : BISHOP_ATTACK_TABLE[hashIdx]) = attacks;
    }
  }
}
template void fill_attack_map<ROOK>();
template void fill_attack_map<BISHOP>();

template <Piece P> bool verify_attack_map() {
  for (Square square = a1; square <= h8; ++square) {
    Magic m = (P == ROOK) ? ROOK_MAGICS[square] : BISHOP_MAGICS[square];
    int bit_count = std::popcount(m.mask);
    for (int i = 0; i < (1ULL << bit_count); i++) {
      Bitboard occupancy = PDEP(i, m.mask);

      uint64_t hashIdx = ((occupancy * m.magic) >> m.shift) + m.idx;
      uint64_t attacks = (P == ROOK) ? slow_rook_attack(square, occupancy)
                                     : slow_bishop_attack(square, occupancy);
      if (((P == ROOK) ? ROOK_ATTACK_TABLE[hashIdx]
                       : BISHOP_ATTACK_TABLE[hashIdx]) != attacks) {
        return false;
      }
    }
  }
  return true;
}
template bool verify_attack_map<ROOK>();
template bool verify_attack_map<BISHOP>();

} // namespace citterfish