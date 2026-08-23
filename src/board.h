#pragma once

#include "attacks.h"
#include "types.h"
#include "zobrist.h"
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <ostream>
#include <string>

namespace citterfish {
struct StateInfo {
  // copied
  Key zobrist_hash;
  uint16_t halfmove_clock;
  uint8_t castling_rights; // First digit is K then Q then k then q

  // refreshed
  Bitboard checkers;
  Bitboard pinned_pieces;
  Bitboard pinners;
  Piece captured;
  Square en_passant_square;
  uint16_t repeatAmount;

  StateInfo *prevSt;

  StateInfo() = default;
};

class Board {

private:
  std::array<std::array<Bitboard, 6>, 2> pieces;
  std::array<Bitboard, 2> occupied;
  Bitboard all_occupied;
  bool is_white;
  uint16_t fullmove_clock;
  std::array<PieceType, 64> piece_map;
  StateInfo *st;

public:
  Board(const std::string &fen);
  Board();

  template <Piece P> Bitboard get_pieces(Color color) const {
    return pieces[color][P];
  }
  Bitboard get_pieces(Color color, Piece piece) const {
    return pieces[color][piece];
  }
  PieceType piece_on(Square square) const { return piece_map[square]; }
  bool get_is_white() const { return is_white; }
  uint8_t get_castling_rights() const { return st->castling_rights; }
  Square get_en_passant_square() const { return st->en_passant_square; }
  uint16_t get_halfmove_clock() const { return st->halfmove_clock; }
  uint16_t get_fullmove_clock() const { return fullmove_clock; }
  Key get_zobrist_hash() const { return st->zobrist_hash; }
  Bitboard get_pieces(Color color) const { return occupied[color]; }
  Bitboard get_pieces() const { return all_occupied; }
  Bitboard get_checkers() const { return st->checkers; }
  Bitboard get_pinned() const { return st->pinned_pieces; }
  Bitboard get_pinners() const { return st->pinners; }

  template <Color us> void refresh_checks_pins() {
    constexpr Color them = ~us;
    constexpr Direction D = (us == WHITE) ? SOUTH : NORTH;
    Square king_square =
        static_cast<Square>(std::countr_zero(get_pieces<KING>(us)));
    // Get all knight checkers
    Bitboard cur_checkers =
        attacks::knight_attacks(king_square) & get_pieces<KNIGHT>(them);

    // Get all pawn checkers
    cur_checkers |=
        attacks::pawn_attackers<us>(king_square) & get_pieces<PAWN>(them);

    Bitboard king_ortho = attacks::sliding_attacks<ROOK>(
        king_square,
        get_pieces(them)); // Where the king can be orthogonally attacked from
    Bitboard king_diag = attacks::sliding_attacks<BISHOP>(
        king_square,
        get_pieces(them)); // Where the king can be diagonally attacked from
    Bitboard king_attackers =
        (king_ortho & (get_pieces<ROOK>(them) | get_pieces<QUEEN>(them))) |
        (king_diag & (get_pieces<BISHOP>(them) | get_pieces<QUEEN>(them)));
    Bitboard cur_pinned = 0;
    Bitboard cur_pinners = 0;
    while (king_attackers) {
      Square attacker = static_cast<Square>(std::countr_zero(king_attackers));
      Bitboard blockers =
          attacks::from_to_bb(king_square, attacker) & get_pieces(us);
      if (blockers == 0) {
        cur_checkers |= 1ULL << attacker;
      } else if ((blockers & (blockers - 1)) == 0) {
        cur_pinned |= blockers;
        cur_pinners |= 1ULL << attacker;
      }
      king_attackers &= king_attackers - 1;
    }
    st->checkers = cur_checkers;
    st->pinned_pieces = cur_pinned;
    st->pinners = cur_pinners;
  }

  template <Color us> void make_move(Move move, StateInfo *newSt) {
    constexpr Color them = ~us;
    Square from = move.get_from_square();
    Square to = move.get_to_square();
    Bitboard fromBB = 1ULL << from;
    Bitboard toBB = 1ULL << to;
    MoveType type = move.get_move_type();
    Piece moving = piece_type_to_piece(piece_on(to));

    // copy the incremental fields of stateInfo
    memcpy(newSt, st, offsetof(StateInfo, checkers));
    newSt->prevSt = st;
    st = newSt;
    ++st->halfmove_clock;
    is_white = !is_white;
    ++fullmove_clock;

    if (type == REGULAR) { // if regular just remove captured, and move moving
      Piece captured = piece_type_to_piece(piece_on(to));
      st->captured = piece_type_to_piece(piece_on(to));
      if (captured != NO_PIECE) {
        capture_piece<us>(moving, captured, from, to, fromBB, toBB);
      } else {
        move_piece<us>(moving, from, to, fromBB, toBB);
      }
    }
  }

  template <Color us> void put_piece(Piece p, Square s, Bitboard squareBB) {
    pieces[us][p] ^= squareBB;
    occupied[us] ^= squareBB;
    all_occupied ^= squareBB;
    st->zobrist_hash ^= zobrist::getPieceKey(us, p, s);
    piece_map[s] = piece_to_piece_type(p, us);
  }

  template <Color us>
  void capture_piece(Piece moving, Piece captured, Square sMoving,
                     Square sCaptured, Bitboard movingBB, Bitboard capturedBB) {
    constexpr Color them = ~us;
    pieces[them][captured] ^= capturedBB;
    occupied[them] ^= capturedBB;
    pieces[us][moving] ^= movingBB | capturedBB;
    occupied[us] ^= movingBB | capturedBB;
    all_occupied ^= movingBB;
    st->zobrist_hash ^= zobrist::getPieceKey(us, moving, sMoving);
    st->zobrist_hash ^= zobrist::getPieceKey(us, moving, sCaptured);
    st->zobrist_hash ^= zobrist::getPieceKey(them, captured, sCaptured);
    piece_map[sMoving] = NO_PIECE_TYPE;
    piece_map[sCaptured] = piece_to_piece_type(moving, us);
  }

  template <Color us>
  void move_piece(Piece p, Square from, Square to, Bitboard fromBB,
                  Bitboard toBB) {
    put_piece<us>(p, to, toBB);
    remove_piece<us>(p, from, fromBB);
  }

  template <Color us> void remove_piece(Piece p, Square s, Bitboard squareBB) {
    pieces[us][p] ^= squareBB;
    occupied[us] ^= squareBB;
    all_occupied ^= squareBB;
    st->zobrist_hash ^= zobrist::getPieceKey(us, p, s);
    piece_map[s] = NO_PIECE_TYPE;
  }

  void refresh_piece_map();
  void refresh_bitboards();
  void refresh_zobrist_hash();
  std::string to_fen() const;
};

std::ostream &operator<<(std::ostream &os, const Board &b);

} // namespace citterfish