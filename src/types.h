#pragma once

#include <cctype>
#include <cstdint>
#include <string>

namespace citterfish {

using Bitboard = std::uint64_t;
using Key = std::uint64_t;


enum Color : uint8_t { WHITE, BLACK, BAD_COLOR };
enum Piece : uint8_t { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, BAD_PIECE };

inline char pieceToChar(Piece piece, Color color) {
  switch (piece) {
  case PAWN:
    return color == WHITE ? 'P' : 'p';
  case KNIGHT:
    return color == WHITE ? 'N' : 'n';
  case BISHOP:
    return color == WHITE ? 'B' : 'b';
  case ROOK:
    return color == WHITE ? 'R' : 'r';
  case QUEEN:
    return color == WHITE ? 'Q' : 'q';
  case KING:
    return color == WHITE ? 'K' : 'k';
  default:
    return '?'; // Should never happen
  }
}

inline Color charToColor(char c) {
  if (std::islower(c))
    return BLACK;
  else if (std::isupper(c))
    return WHITE;
  return BAD_COLOR;
}

inline Piece charToPiece(char c) {
  switch (std::tolower(c)) {
  case 'p':
    return PAWN;
  case 'n':
    return KNIGHT;
  case 'b':
    return BISHOP;
  case 'r':
    return ROOK;
  case 'q':
    return QUEEN;
  case 'k':
    return KING;
  }
  return BAD_PIECE;
}


enum CastlingRight : uint8_t {
  None = 0,
  WhiteKingSide = 1 << 0,
  WhiteQueenSide = 1 << 1,
  BlackKingSide = 1 << 2,
  BlackQueenSide = 1 << 3
};


// clang-format off
enum Square : std::uint8_t {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8,

    none
};
// clang-format on

inline Square squareFromString(const std::string &square) {
  if (square.length() != 2) {
    return none;
  }
  char fileChar = square.at(0);
  char rankChar = square.at(1);
  if (fileChar < 'a' || fileChar > 'h' || rankChar < '1' || rankChar > '8') {
    return none;
  }
  int file = fileChar - 'a';
  int rank = rankChar - '1';
  return static_cast<Square>(rank * 8 + file);
}

inline std::string squareToString(Square square) {
  if (square == 64) {
    return "-";
  }
  int rank = square / 8;
  int file = square % 8;
  char fileChar = 'a' + file;
  char rankChar = '1' + rank;
  return std::string{fileChar, rankChar};
}


enum MoveType : uint8_t {
  QUIET,
  ENPASSANT,
  CASTLE,
  PROMOTION
};

class Move {
  private:
  uint16_t moveData;
    // bits 0-5: from square (6 bits)
    // bits 6-11: to square (6 bits)
    // bits 12-13: promotion piece (2 bits)
    // bits 14-15: flags (2 bits)

  public:
  Move() = default;
  Move(Square from, Square to, MoveType type = QUIET, Piece promotionPiece = KNIGHT) {
    moveData = static_cast<uint16_t>(from) |
               (static_cast<uint16_t>(to) << 6) |
               ((static_cast<uint16_t>(promotionPiece)-1) << 12) |
               (static_cast<uint16_t>(type) << 14);
  }
  constexpr Square getFromSquare() const { return static_cast<Square>(moveData & 0x3F); }
  constexpr Square getToSquare() const { return static_cast<Square>((moveData >> 6) & 0x3F); }
  constexpr MoveType getMoveType() const { return static_cast<MoveType>((moveData >> 14) & 0x03); }
  constexpr Piece getPromotionPiece() const { return static_cast<Piece>(((moveData >> 12) & 0x03) + 1); }

};


}