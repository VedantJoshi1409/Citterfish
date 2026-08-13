#pragma once

#include "magics.h"
#include "types.h"

namespace citterfish::attacks {
namespace detail {
inline std::array<Bitboard, 64> knightAttacks;
inline std::array<Bitboard, 64> kingAttacks;
inline std::array<std::array<Bitboard, 64>, 2> pawnAttackers;
inline std::array<std::array<Bitboard, 64>, 64> fromToBitboards{};
} // namespace detail

void initializeAttacks();
Bitboard getKnightAttacks(Square square);
Bitboard getKingAttacks(Square square);

inline Bitboard getKnightAttacks(Square square) {
  return detail::knightAttacks[static_cast<int>(square)];
}

inline Bitboard getKingAttacks(Square square) {
  return detail::kingAttacks[static_cast<int>(square)];
}

template <Color C> inline Bitboard getPawnAttackers(Square square) {
  return detail::pawnAttackers[C][square];
}

inline Bitboard getFromToBitboard(Square from, Square to) {
  return detail::fromToBitboards[static_cast<int>(from)][static_cast<int>(to)];
}

template <Piece P>
inline Bitboard getSlidingAttacks(Square square, Bitboard occupied) {
  if constexpr (P == QUEEN) {
    return getSlidingAttacks<ROOK>(square, occupied) |
           getSlidingAttacks<BISHOP>(square, occupied);
  }
  const Magic &m = (P == ROOK) ? RookMagics[square] : BishopMagics[square];
  uint64_t idx = (((m.mask & occupied) * m.magic) >> m.shift) + m.idx;
  return (P == ROOK) ? RookAttackTable[idx] : BishopAttackTable[idx];
}

inline Bitboard getSlidingAttacks(Piece p, Square s, Bitboard occ) {
  switch (p) {
  case ROOK:
    return getSlidingAttacks<ROOK>(s, occ);
  case BISHOP:
    return getSlidingAttacks<BISHOP>(s, occ);
  case QUEEN:
    return getSlidingAttacks<QUEEN>(s, occ);
  default:
    return 0;
  }
}
} // namespace citterfish::attacks