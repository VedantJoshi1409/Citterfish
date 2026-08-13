#pragma once

#include <cctype>
#include <cstdint>
#include <iostream>
#include <string>

namespace citterfish {

using Key = std::uint64_t;
using Bitboard = std::uint64_t;
inline void printBB(Bitboard bb) {
  std::cout << "\nBitboard: " << bb << std::endl;
  for (int rank = 7; rank >= 0; --rank) {
    for (int file = 0; file < 8; ++file) {
      int square = rank * 8 + file;
      std::cout << ((bb & (1ULL << square)) ? '1' : '0') << " ";
    }
    std::cout << std::endl;
  }
}
constexpr Bitboard ALL_SQUARES = 0xFFFFFFFFFFFFFFFFULL;

enum Color : uint8_t { WHITE, BLACK, BAD_COLOR };
inline Color operator~(Color c) {
  return static_cast<Color>(c^1);
}
enum Piece : uint8_t { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, BAD_PIECE };
enum PieceType : uint8_t { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING, B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING, NO_PIECE_TYPE};
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
inline Color pieceTypeToColor(PieceType pt) {
  return static_cast<Color>(pt/6);
}
inline Piece pieceTypeToPiece(PieceType pt) {
  return static_cast<Piece>(pt%6);
}
inline char pieceTypeToChar(PieceType pieceType) {
  return pieceToChar(pieceTypeToPiece(pieceType), pieceTypeToColor(pieceType));
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
inline PieceType pieceToPieceType(Piece piece, Color color) {
  return static_cast<PieceType>(color*6+piece);
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

    no_square
};
// clang-format on
inline Square squareFromString(const std::string &square) {
  if (square.length() != 2) {
    return no_square;
  }
  char fileChar = square.at(0);
  char rankChar = square.at(1);
  if (fileChar < 'a' || fileChar > 'h' || rankChar < '1' || rankChar > '8') {
    return no_square;
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
inline Square &operator++(Square &square) {
  square = static_cast<Square>(static_cast<int>(square) + 1);
  return square;
}
enum Direction : int8_t { NORTH = 8, SOUTH = -8, EAST = 1, WEST = -1 };
constexpr Square operator+(Square square, Direction direction) {
  return static_cast<Square>(static_cast<int>(square) +
                             static_cast<int>(direction));
}
constexpr Square operator-(Square square, Direction direction) {
  return static_cast<Square>(static_cast<int>(square) -
                             static_cast<int>(direction));
}

enum Files : Bitboard {
  FILE_A = 0x0101010101010101ULL,
  FILE_B = 0x0202020202020202ULL,
  FILE_C = 0x0404040404040404ULL,
  FILE_D = 0x0808080808080808ULL,
  FILE_E = 0x1010101010101010ULL,
  FILE_F = 0x2020202020202020ULL,
  FILE_G = 0x4040404040404040ULL,
  FILE_H = 0x8080808080808080ULL
};
enum Ranks : Bitboard {
  RANK_1 = 0x00000000000000FFULL,
  RANK_2 = 0x000000000000FF00ULL,
  RANK_3 = 0x0000000000FF0000ULL,
  RANK_4 = 0x00000000FF000000ULL,
  RANK_5 = 0x000000FF00000000ULL,
  RANK_6 = 0x0000FF0000000000ULL,
  RANK_7 = 0x00FF000000000000ULL,
  RANK_8 = 0xFF00000000000000ULL
};

enum MoveType : uint8_t { REGULAR, ENPASSANT, CASTLE, PROMOTION };
class Move {
private:
  uint16_t moveData;
  // bits 0-5: from square (6 bits)
  // bits 6-11: to square (6 bits)
  // bits 12-13: promotion piece (2 bits)
  // bits 14-15: flags (2 bits)

public:
  Move() = default;
  Move(Square from, Square to, MoveType type = REGULAR,
       Piece promotionPiece = KNIGHT) {
    moveData = static_cast<uint16_t>(from) | (static_cast<uint16_t>(to) << 6) |
               ((static_cast<uint16_t>(promotionPiece) - 1) << 12) |
               (static_cast<uint16_t>(type) << 14);
  }
  constexpr Square getFromSquare() const {
    return static_cast<Square>(moveData & 0x3F);
  }
  constexpr Square getToSquare() const {
    return static_cast<Square>((moveData >> 6) & 0x3F);
  }
  constexpr MoveType getMoveType() const {
    return static_cast<MoveType>((moveData >> 14) & 0x03);
  }
  constexpr Piece getPromotionPiece() const {
    return static_cast<Piece>(((moveData >> 12) & 0x03) + 1);
  }
};
constexpr uint8_t MAX_MOVES = 224;

constexpr uint32_t ROOK_TABLE_SIZE = 88024;
constexpr uint32_t BISHOP_TABLE_SIZE = 4782;

class PRNG {
  uint64_t state;

public:
  constexpr explicit PRNG(uint64_t seed) : state(seed) {}
  constexpr uint64_t getState() { return state; };
  constexpr uint64_t next() {
    state ^= (state >> 12);
    state ^= (state << 25);
    state ^= (state >> 27);
    return state * 2685821657736338717ULL;
  };

  constexpr uint64_t sparse() { return next() & next() & next(); }
};
} // namespace citterfish