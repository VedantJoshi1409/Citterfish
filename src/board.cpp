#include "board.h"
#include "types.h"
#include "zobrist.h"
#include <iostream>
#include <ranges>
#include <string>

namespace citterfish {
Board::Board(const std::string &fen)
    : pieces{}, castlingRights(0), zobristHash(0), halfmoveClock(0),
      fullmoveClock(0) {
  uint8_t index = 56;
  int i = 0;
  for (; i < fen.length() && fen.at(i) != ' '; i++) {
    char c = fen.at(i);
    if (c == '/') { // new row
      index -= 16;
      continue;
    }
    if ('0' <= c && c <= '9') { // skip squares
      index += (c - '0');
    } else {
      pieces[charToColor(c)][charToPiece(c)] |= 1ULL << index;
      zobristHash ^= zobrist::getPieceKey(charToColor(c), charToPiece(c),
                                          static_cast<Square>(index));
      ++index;
    }
  }
  whiteToMove = fen.at(i + 1) == 'w';
  if (whiteToMove) {
    zobristHash ^= zobrist::getSideToMoveKey();
  }
  i += 3;
  for (; i < fen.length() && fen.at(i) != ' '; i++) {
    switch (fen.at(i)) {
    case 'K':
      castlingRights |= WhiteKingSide;
      break;
    case 'Q':
      castlingRights |= WhiteQueenSide;
      break;
    case 'k':
      castlingRights |= BlackKingSide;
      break;
    case 'q':
      castlingRights |= BlackQueenSide;
      break;
    }
  }
  zobristHash ^= zobrist::getCastlingKey(castlingRights);
  if (fen.at(i + 1) == '-') {
    enPassantSquare = 0;
  } else {
    enPassantSquare = 1ULL << squareFromString(fen.substr(i + 1, 2));
    zobristHash ^= zobrist::getEnPassantKey(
        static_cast<Square>(std::countr_zero(enPassantSquare)));
  }
  halfmoveClock = fen.at(i + 3) - '0';
  fullmoveClock = fen.at(i + 5) - '0';
}

Board::Board()
    : Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {}

std::ostream &operator<<(std::ostream &os, const Board &b) {
  auto stringBoard = b.toStringBoard();
  constexpr std::string_view separator = "+---+---+---+---+---+---+---+---+\n";
  std::string output{separator};
  for (const auto &row : std::views::reverse(stringBoard)) {
    output += "| ";
    for (const auto &square : row) {
      output += square == "" ? " " : square;
      output += " | ";
    }
    output += "\n" + std::string(separator);
  }
  output +=
      std::string((b.getWhiteToMove() ? "White" : "Black")) + " to move\n";
  std::string castle{};
  if ((b.getCastlingRights() & WhiteKingSide) != 0)
    castle += 'K';
  if ((b.getCastlingRights() & WhiteQueenSide) != 0)
    castle += 'Q';
  if ((b.getCastlingRights() & BlackKingSide) != 0)
    castle += 'k';
  if ((b.getCastlingRights() & BlackQueenSide) != 0)
    castle += 'q';
  if (castle.empty())
    castle = '-';
  output += "Castle rights: " + castle + "\n";
  output += "En passant square: " +
            squareToString(
                static_cast<Square>(std::countr_zero(b.getEnPassantSquare()))) +
            std::string("\n");
  output += "Halfmove clock: " + std::to_string(b.getHalfmoveClock()) + "\n";
  output += "Fullmove clock: " + std::to_string(b.getFullmoveClock()) + "\n";
  output += "Zobrist hash: " + std::to_string(b.getZobristHash()) + "\n";
  output += "FEN: " + b.toFen() + "\n";
  os << output;
  return os;
}
} // namespace citterfish