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

  // accessors
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

  template <Color us> void refresh_checks_pins();

  template <Color us> void make_move(Move move, StateInfo *newSt);

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