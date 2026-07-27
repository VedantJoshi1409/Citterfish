#pragma once

#include "types.h"
#include <array>
#include <bit>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <string>

class Board {

private:
  std::array<std::array<Bitboard, 6>, 2> pieces;
  std::array<Bitboard, 2> occupied;
  uint8_t castlingRights; // First digit is K then Q then k then q
  bool whiteToMove;
  Bitboard enPassantSquare;
  uint16_t halfmoveClock;
  uint16_t fullmoveClock;
  uint64_t zobristHash;

public:
  Board(const std::string &fen);
  Board();

  bool getWhiteToMove() const { return whiteToMove; }
  void setWhiteToMove(const bool toMove) { whiteToMove = toMove; }
  uint8_t getCastlingRights() const { return castlingRights; }
  void setCastlingRights(const uint8_t rights) { castlingRights = rights; }
  Bitboard getEnPassantSquare() const { return enPassantSquare; }
  void setEnPassantSquare(const Bitboard square) { enPassantSquare = square; }
  uint16_t getHalfmoveClock() const { return halfmoveClock; }
  void setHalfmoveClock(const uint16_t clock) { halfmoveClock = clock; }
  uint16_t getFullmoveClock() const { return fullmoveClock; }
  void setFullmoveClock(const uint16_t clock) { fullmoveClock = clock; }

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