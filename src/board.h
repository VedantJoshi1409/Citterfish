#pragma once

#include "types.h"
#include "attacks.h"
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
  uint8_t castlingRights; // First digit is K then Q then k then q
  bool whiteToMove;
  Square enPassantSquare;
  uint16_t halfmoveClock;
  uint16_t fullmoveClock;
  Key zobristHash;
  std::array<PieceType, 64> pieceMap;

  // refreshed
  Bitboard checkers;
  Bitboard pinnedPieces;

public:
  Board(const std::string &fen);
  Board();

  Bitboard getPieces(Color color, Piece piece) const {
    return pieces[color][piece];
  }
  PieceType getPieceFromMap(Square square) const {
    return pieceMap[square];
  }
  bool getWhiteToMove() const { return whiteToMove; }
  uint8_t getCastlingRights() const { return castlingRights; }
  Square getEnPassantSquare() const { return enPassantSquare; }
  uint16_t getHalfmoveClock() const { return halfmoveClock; }
  uint16_t getFullmoveClock() const { return fullmoveClock; }
  Key getZobristHash() const { return zobristHash; }
  Bitboard getOccupied(Color color) const { return occupied[color]; }

  template <Color us> void refreshChecksAndPins() {
    Color them = (us == WHITE) ? BLACK : WHITE;
    Direction D = (us == WHITE) ? SOUTH : NORTH;
    Bitboard occupied = getOccupied(us) | getOccupied(them);
    Bitboard king = getPieces(us, KING);
    Square kingSquare = static_cast<Square>(std::popcount(king));

    // Get all knight checkers
    Bitboard checkers =
        attacks::getKnightAttacks(kingSquare) & getPieces(them, KNIGHT);

    // Get all pawn checkers
    checkers |= attacks::getKingAttacks(kingSquare) & (king << (D + EAST)) &
                (king << (D + WEST)) & getPieces(them, PAWN);

    Bitboard kingRookRay =
        attacks::getSlidingAttacks<ROOK>(kingSquare, occupied);
  }

  void refreshPieceMap(); 
  void refreshBitboards();
  void refreshZobristKey(); 
  std::string toFen() const;
};

std::ostream &operator<<(std::ostream &os, const Board &b);

} // namespace citterfish