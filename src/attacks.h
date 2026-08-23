#pragma once

#include "magics.h"
#include "types.h"

namespace citterfish::attacks {
namespace detail {
inline std::array<Bitboard, 64> knightAttacks;
inline std::array<Bitboard, 64> kingAttacks;
inline std::array<std::array<Bitboard, 64>, 2> pawnAttackers;
inline std::array<Bitboard, 64> orthoRays;
inline std::array<Bitboard, 64> diagRays;
inline std::array<std::array<Bitboard, 64>, 64> fromToBB{};
} // namespace detail

void init_attacks();

template <Color C> inline Bitboard pawn_attacks(Bitboard pawns) {
  if constexpr (C == WHITE) {
    return ((pawns & ~FILE_A) << (NORTH + WEST)) |
           ((pawns & ~FILE_H) << (NORTH + WEST));
  } else {
    return ((pawns & ~FILE_A) >> -(SOUTH + WEST)) |
           ((pawns & ~FILE_H) >> -(SOUTH + WEST));
  }
}

inline Bitboard knight_attacks(Square square) {
  return detail::knightAttacks[static_cast<int>(square)];
}

inline Bitboard king_attacks(Square square) {
  return detail::kingAttacks[static_cast<int>(square)];
}

template <Color C> inline Bitboard pawn_attackers(Square square) {
  return detail::pawnAttackers[C][square];
}

inline Bitboard ortho_rays(Square s) { return detail::orthoRays[s]; }

inline Bitboard diag_rays(Square s) { return detail::diagRays[s]; }

inline Bitboard from_to_bb(Square from, Square to) {
  return detail::fromToBB[static_cast<int>(from)][static_cast<int>(to)];
}

template <Piece P>
inline Bitboard sliding_attacks(Square square, Bitboard occupied) {
  if constexpr (P == QUEEN) {
    return sliding_attacks<ROOK>(square, occupied) |
           sliding_attacks<BISHOP>(square, occupied);
  }
  const Magic &m = (P == ROOK) ? ROOK_MAGICS[square] : BISHOP_MAGICS[square];
  uint64_t idx = (((m.mask & occupied) * m.magic) >> m.shift) + m.idx;
  return (P == ROOK) ? ROOK_ATTACK_TABLE[idx] : BISHOP_ATTACK_TABLE[idx];
}

inline Bitboard sliding_attacks(Piece p, Square s, Bitboard occ) {
  switch (p) {
  case ROOK:
    return sliding_attacks<ROOK>(s, occ);
  case BISHOP:
    return sliding_attacks<BISHOP>(s, occ);
  case QUEEN:
    return sliding_attacks<QUEEN>(s, occ);
  default:
    return 0;
  }
}
} // namespace citterfish::attacks