#pragma once

#include "types.h"
#include "attacks.h"
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
  Bitboard enPassantSquare;
  uint16_t halfmoveClock;
  uint16_t fullmoveClock;
  Key zobristHash;

  // refreshed
  Bitboard checkers;
  Bitboard pinnedPieces;

public:
  Board(const std::string &fen);
  Board();

  Bitboard getPieces(Color color, Piece piece) const {
    return pieces[color][piece];
  }

  bool getWhiteToMove() const { return whiteToMove; }
  uint8_t getCastlingRights() const { return castlingRights; }
  Bitboard getEnPassantSquare() const { return enPassantSquare; }
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

  std::array<std::array<std::string, 8>, 8> toStringBoard() const {
    std::array<std::array<std::string, 8>, 8> stringBoard;
    for (Color color : {WHITE, BLACK}) {
      for (Piece piece : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
        Bitboard bitboard = pieces[color][piece];
        while (bitboard) {
          int square = std::countr_zero(bitboard);
          int row = square / 8;
          int col = square % 8;
          stringBoard[row][col] = std::string(1, pieceToChar(piece, color));
          bitboard &= bitboard - 1; // Clear lsb
        }
      }
    }
    return stringBoard;
  }

  std::string toFen() const {
    std::string fen;
    auto stringBoard = toStringBoard();
    for (int row = 7; row >= 0; --row) {
      int emptyCount = 0;
      for (int col = 0; col < 8; ++col) {
        const std::string &square = stringBoard[row][col];
        if (square.empty()) {
          ++emptyCount;
        } else {
          if (emptyCount > 0) {
            fen += std::to_string(emptyCount);
            emptyCount = 0;
          }
          fen += square;
        }
      }
      if (emptyCount > 0) {
        fen += std::to_string(emptyCount);
      }
      if (row > 0) {
        fen += '/';
      }
    }
    fen += ' ';
    fen += (whiteToMove ? 'w' : 'b');
    fen += ' ';
    if (castlingRights == 0) {
      fen += '-';
    } else {
      if (castlingRights & WhiteKingSide)
        fen += 'K';
      if (castlingRights & WhiteQueenSide)
        fen += 'Q';
      if (castlingRights & BlackKingSide)
        fen += 'k';
      if (castlingRights & BlackQueenSide)
        fen += 'q';
    }
    fen += ' ';
    fen +=
        squareToString(static_cast<Square>(std::countr_zero(enPassantSquare)));
    fen += ' ';
    fen += std::to_string(halfmoveClock);
    fen += ' ';
    fen += std::to_string(fullmoveClock);
    return fen;
  }
};

std::ostream &operator<<(std::ostream &os, const Board &b);
} // namespace citterfish