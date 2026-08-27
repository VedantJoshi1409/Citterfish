#pragma once

#include "types.h"
#include "zobrist.h"
#include <array>
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
  bool operator==(const StateInfo&) const = default;
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
  Bitboard get_pieces(PieceType pieceType) const {
    return get_pieces(piece_type_to_color(pieceType), piece_type_to_piece(pieceType));
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
  StateInfo* get_state() const {return st;}
  Piece get_captured() const {return st->captured; }

void checks_pins();
Bitboard attack_mask(Color c);

void make_move(Move move, StateInfo *newSt);
void unmake_move(Move move);

void put_piece(Piece p, Color c, Square s, Bitboard squareBB) {
    pieces[c][p] ^= squareBB;
    occupied[c] ^= squareBB;
    allOccupied ^= squareBB;
    st->zobristHash ^= zobrist::getPieceKey(c, p, s);
    pieceMap[s] = piece_to_piece_type(p, c);
  }
void put_piece(Piece p, Color c, Square s) {
    return put_piece(p, c, s, square_to_bb(s));
  }
void capture_piece(Piece moving, Color c, Piece captured, Square sMoving,
                     Square sCaptured, Bitboard movingBB, Bitboard capturedBB) {
    Color them = ~c;
    pieces[them][captured] ^= capturedBB;
    occupied[them] ^= capturedBB;
    pieces[c][moving] ^= movingBB | capturedBB;
    occupied[c] ^= movingBB | capturedBB;
    allOccupied ^= movingBB;
    st->zobristHash ^= zobrist::getPieceKey(c, moving, sMoving);
    st->zobristHash ^= zobrist::getPieceKey(c, moving, sCaptured);
    st->zobristHash ^= zobrist::getPieceKey(them, captured, sCaptured);
    pieceMap[sMoving] = NO_PIECE_TYPE;
    pieceMap[sCaptured] = piece_to_piece_type(moving, c);
  }
void capture_piece(Piece moving, Color c, Piece captured, Square sMoving, Square sCaptured) {
    capture_piece(moving, c, captured, sMoving, sCaptured, square_to_bb(sMoving), square_to_bb(sCaptured));
  }
void move_piece(Piece p, Color c, Square from, Square to, Bitboard fromBB,
                  Bitboard toBB) {
    put_piece(p, c, to, toBB);
    remove_piece(p, c, from, fromBB);
  }
void move_piece(Piece p, Color c, Square from, Square to) {
    move_piece(p, c, from, to, square_to_bb(from), square_to_bb(to));
  }
void remove_piece(Piece p, Color c, Square s, Bitboard squareBB) {
    pieces[c][p] ^= squareBB;
    occupied[c] ^= squareBB;
    allOccupied ^= squareBB;
    st->zobristHash ^= zobrist::getPieceKey(c, p, s);
    pieceMap[s] = NO_PIECE_TYPE;
  }
void remove_piece(Piece p, Color c, Square s) {
    return remove_piece(p, c, s, square_to_bb(s));
  }

  void refresh_piece_map();
  void refresh_bitboards();
  void refresh_zobrist_hash();
  std::string to_fen() const;

  bool operator==(const Board &b) const {
    bool eq = true;
    for(int i = PAWN; i <= KING; ++i) {
      for (int j = WHITE; j <= BLACK; ++j) {
        eq&=pieces[j][i] == b.pieces[j][i];
      }
    }
    eq&= occupied[WHITE] == b.occupied[WHITE];
    eq&= occupied[BLACK] == b.occupied[BLACK];
    eq&= allOccupied == b.allOccupied;
    eq&= isWhite == b.isWhite;
    eq&= fullmoveClock == b.fullmoveClock;
    for (Square s = a1; s <= h8; ++s) {
      eq&= pieceMap[s] == b.pieceMap[s];
    }
    eq&= st == b.st;


    return eq;
  };
};

std::ostream &operator<<(std::ostream &os, const Board &b);

} // namespace citterfish