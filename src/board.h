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
  Key zobristHash;
  uint16_t halfmoveClock;
  uint8_t castlingRights; // First digit is K then Q then k then q

  // refreshed
  Bitboard checkers;
  Bitboard pinnedPieces;
  Bitboard pinners;
  Piece captured;
  Square enPassantSquare;
  uint16_t repeatAmount;

  StateInfo *prevSt;

  StateInfo() = default;
};

class Board {

private:
  std::array<std::array<Bitboard, 6>, 2> pieces;
  std::array<Bitboard, 2> occupied;
  Bitboard allOccupied;
  bool isWhite;
  uint16_t fullmoveClock;
  std::array<PieceType, 64> pieceMap;
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
  PieceType piece_on(Square square) const { return pieceMap[square]; }
  bool is_white() const { return isWhite; }
  uint8_t castling_rights() const { return st->castlingRights; }
  Square en_passant_square() const { return st->enPassantSquare; }
  uint16_t halfmove_clock() const { return st->halfmoveClock; }
  uint16_t fullmove_clock() const { return fullmoveClock; }
  Key zobrist_hash() const { return st->zobristHash; }
  Bitboard get_pieces(Color color) const { return occupied[color]; }
  Bitboard get_pieces() const { return allOccupied; }
  Bitboard get_checkers() const { return st->checkers; }
  Bitboard get_pinned() const { return st->pinnedPieces; }
  Bitboard get_pinners() const { return st->pinners; }

  template <Color us> void refresh_checks_pins() {
    constexpr Color them = ~us;
    constexpr Direction D = (us == WHITE) ? SOUTH : NORTH;
    Square kingSquare =
        static_cast<Square>(std::countr_zero(get_pieces<KING>(us)));
    // Get all knight checkers
    Bitboard curCheckers =
        attacks::knight_attacks(kingSquare) & get_pieces<KNIGHT>(them);

    // Get all pawn checkers
    curCheckers |=
        attacks::pawn_attackers<us>(kingSquare) & get_pieces<PAWN>(them);

    Bitboard king_ortho = attacks::sliding_attacks<ROOK>(
        kingSquare,
        get_pieces(them)); // Where the king can be orthogonally attacked from
    Bitboard king_diag = attacks::sliding_attacks<BISHOP>(
        kingSquare,
        get_pieces(them)); // Where the king can be diagonally attacked from
    Bitboard kingAttackers =
        (king_ortho & (get_pieces<ROOK>(them) | get_pieces<QUEEN>(them))) |
        (king_diag & (get_pieces<BISHOP>(them) | get_pieces<QUEEN>(them)));
    Bitboard curPinned = 0;
    Bitboard curPinners = 0;
    while (kingAttackers) {
      Square attacker = static_cast<Square>(std::countr_zero(kingAttackers));
      Bitboard blockers =
          attacks::from_to_bb(kingSquare, attacker) & get_pieces(us);
      if (blockers == 0) {
        curCheckers |= 1ULL << attacker;
      } else if ((blockers & (blockers - 1)) == 0) {
        curPinned |= blockers;
        curPinners |= 1ULL << attacker;
      }
      kingAttackers &= kingAttackers - 1;
    }
    st->checkers = curCheckers;
    st->pinnedPieces = curPinned;
    st->pinners = curPinners;
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
    ++st->halfmoveClock;
    isWhite = !isWhite;
    ++fullmoveClock;

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
    allOccupied ^= squareBB;
    st->zobristHash ^= zobrist::getPieceKey(us, p, s);
    pieceMap[s] = piece_to_piece_type(p, us);
  }

  template <Color us>
  void capture_piece(Piece moving, Piece captured, Square sMoving,
                     Square sCaptured, Bitboard movingBB, Bitboard capturedBB) {
    constexpr Color them = ~us;
    pieces[them][captured] ^= capturedBB;
    occupied[them] ^= capturedBB;
    pieces[us][moving] ^= movingBB | capturedBB;
    occupied[us] ^= movingBB | capturedBB;
    allOccupied ^= movingBB;
    st->zobristHash ^= zobrist::getPieceKey(us, moving, sMoving);
    st->zobristHash ^= zobrist::getPieceKey(us, moving, sCaptured);
    st->zobristHash ^= zobrist::getPieceKey(them, captured, sCaptured);
    pieceMap[sMoving] = NO_PIECE_TYPE;
    pieceMap[sCaptured] = piece_to_piece_type(moving, us);
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
    allOccupied ^= squareBB;
    st->zobristHash ^= zobrist::getPieceKey(us, p, s);
    pieceMap[s] = NO_PIECE_TYPE;
  }

  void refresh_piece_map();
  void refresh_bitboards();
  void refresh_zobrist_hash();
  std::string to_fen() const;
};

std::ostream &operator<<(std::ostream &os, const Board &b);

} // namespace citterfish