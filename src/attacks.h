#pragma once

#include "magics.h"
#include "types.h"

namespace citterfish::attacks {
namespace detail {
inline std::array<Bitboard, 64> knight_attacks;
inline std::array<Bitboard, 64> king_attacks;
inline std::array<std::array<Bitboard, 64>, 2> pawn_attackers;
inline std::array<Bitboard, 64> ortho_rays;
inline std::array<Bitboard, 64> diag_rays;
inline std::array<std::array<Bitboard, 64>, 64> from_to_bb{};
} // namespace detail

void initialize_attacks();

inline Bitboard knight_attacks(Square square) {
  return detail::knight_attacks[static_cast<int>(square)];
}

inline Bitboard king_attacks(Square square) {
  return detail::king_attacks[static_cast<int>(square)];
}

template <Color C> inline Bitboard pawn_attackers(Square square) {
  return detail::pawn_attackers[C][square];
}

inline Bitboard ortho_rays(Square s) { return detail::ortho_rays[s]; }

inline Bitboard diag_rays(Square s) { return detail::diag_rays[s]; }

inline Bitboard from_to_bb(Square from, Square to) {
  return detail::from_to_bb[static_cast<int>(from)][static_cast<int>(to)];
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