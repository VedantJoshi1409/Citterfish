#pragma once

#include "attacks.h"
#include "types.h"
#include "zobrist.h"
#include <array>
#include <bit>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <string>

namespace citterfish {

class Board {

private:
  // updated
  std::array<std::array<Bitboard, 6>, 2> pieces;
  std::array<Bitboard, 2> occupied;
  Bitboard all_occupied;
  uint8_t castling_rights; // First digit is K then Q then k then q
  bool is_white;
  Square en_passant_square;
  uint16_t halfmove_clock;
  uint16_t fullmove_clock;
  Key zobrist_hash;
  std::array<PieceType, 64> piece_map;

  // refreshed
  Bitboard checkers;
  Bitboard pinned_pieces;
  Bitboard pinners;

public:
  Board(const std::string &fen);
  Board();

  template <Piece P> Bitboard get_pieces(Color color) const {
    return pieces[color][P];
  }
  Bitboard get_pieces(Color color, Piece piece) const {
    return pieces[color][piece];
  }
  PieceType get_piece_map(Square square) const { return piece_map[square]; }
  bool get_is_white() const { return is_white; }
  uint8_t get_castling_rights() const { return castling_rights; }
  Square get_en_passant_square() const { return en_passant_square; }
  uint16_t get_halfmove_clock() const { return halfmove_clock; }
  uint16_t get_fullmove_clock() const { return fullmove_clock; }
  Key get_zobrist_hash() const { return zobrist_hash; }
  Bitboard get_occupied(Color color) const { return occupied[color]; }
  Bitboard get_occupied() const { return all_occupied; }
  Bitboard get_checkers() const { return checkers; }
  Bitboard get_pinned() const { return pinned_pieces; }
  Bitboard get_pinners() const { return pinners; }

  template <Color us> void refresh_checks_pins() {
    constexpr Color them = ~us;
    constexpr Direction D = (us == WHITE) ? SOUTH : NORTH;
    Square king_square = static_cast<Square>(std::countr_zero(get_pieces<KING>(us)));
    // Get all knight checkers
    Bitboard cur_checkers =
        attacks::knight_attacks(king_square) & get_pieces<KNIGHT>(them);

    // Get all pawn checkers
    cur_checkers |=
        attacks::pawn_attackers<us>(king_square) & get_pieces<PAWN>(them);

    Bitboard king_ortho = attacks::sliding_attacks<ROOK>(
        king_square,
        get_occupied(them)); // Where the king can be orthogonally attacked from
    Bitboard king_diag = attacks::sliding_attacks<BISHOP>(
        king_square,
        get_occupied(them)); // Where the king can be diagonally attacked from
    Bitboard king_attackers =
        (king_ortho & (get_pieces<ROOK>(them) | get_pieces<QUEEN>(them))) |
        (king_diag & (get_pieces<BISHOP>(them) | get_pieces<QUEEN>(them)));
    Bitboard cur_pinned = 0;
    Bitboard cur_pinners = 0;
    while (king_attackers) {
      Square attacker = static_cast<Square>(std::countr_zero(king_attackers));
      Bitboard blockers =
          attacks::from_to_bb(king_square, attacker) & get_occupied(us);
      if (blockers == 0) {
        cur_checkers |= 1ULL << attacker;
      } else if ((blockers & (blockers - 1)) == 0) {
        cur_pinned |= blockers;
        cur_pinners |= 1ULL << attacker;
      }
      king_attackers &= king_attackers - 1;
    }
    checkers = cur_checkers;
    pinned_pieces = cur_pinned;
    pinners = cur_pinners;
  }

  void refresh_piece_map();
  void refresh_bitboards();
  void refresh_zobrist_hash();
  std::string to_fen() const;
};

std::ostream &operator<<(std::ostream &os, const Board &b);

} // namespace citterfish