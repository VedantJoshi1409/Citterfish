#pragma once

#include "magics.h"
#include "types.h"

namespace citterfish::attacks {
void initializeAttacks();
Bitboard getKnightAttacks(Square square);
Bitboard getKingAttacks(Square square);

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