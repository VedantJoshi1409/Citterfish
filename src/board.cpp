#include "board.h"
#include "types.h"
#include "zobrist.h"
#include <iostream>
#include <ranges>
#include <string>

namespace citterfish {
Board::Board(const std::string &fen) {
  pieceMap.fill(NO_PIECE_TYPE);
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
      pieceMap[index] = pieceToPieceType(charToPiece(c), charToColor(c));
      ++index;
    }
  }
  whiteToMove = fen.at(i + 1) == 'w';
  i += 3;
  castlingRights = 0;
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
  enPassantSquare = (fen.at(i + 1) == '-')
                        ? no_square
                        : squareFromString(fen.substr(i + 1, 2));
  halfmoveClock = (i + 3) < fen.size() ? fen.at(i + 3) - '0' : 0;
  fullmoveClock = (i + 5) < fen.size() ? fen.at(i + 5) - '0' : 0;
  refreshBitboards();
  refreshZobristKey();
}

Board::Board()
    : Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -") {}

void Board::refreshPieceMap() {
  pieceMap.fill(NO_PIECE_TYPE);
  for (Color color : {WHITE, BLACK}) {
    for (Piece piece : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
      Bitboard bitboard = pieces[color][piece];
      while (bitboard) {
        int square = std::countr_zero(bitboard);
        pieceMap[square] = pieceToPieceType(piece, color);
        bitboard &= bitboard - 1; // Clear lsb
      }
    }
  }
}

void Board::refreshBitboards() {
  for (auto &row : pieces) {
    for (auto &x : row) {
      x = 0;
    }
  }
  for (Square square = a1; square <= h8; ++square) {
    if (pieceMap[square] != NO_PIECE_TYPE) {
      Piece piece = pieceTypeToPiece(pieceMap[square]);
      Color color = pieceTypeToColor(pieceMap[square]);
      pieces[color][piece] |= 1ULL << square;
    }
  }
  occupied[WHITE] = pieces[WHITE][PAWN] | pieces[WHITE][KNIGHT] |
                    pieces[WHITE][BISHOP] | pieces[WHITE][ROOK] |
                    pieces[WHITE][QUEEN] | pieces[WHITE][KING];
  occupied[BLACK] = pieces[BLACK][PAWN] | pieces[BLACK][KNIGHT] |
                    pieces[BLACK][BISHOP] | pieces[BLACK][ROOK] |
                    pieces[BLACK][QUEEN] | pieces[BLACK][KING];
  allOccupied = occupied[WHITE] | occupied[BLACK];
}

void Board::refreshZobristKey() {
  zobristHash = 0;
  for (Square square = a1; square <= h8; ++square) {
    if (pieceMap[square] != NO_PIECE_TYPE) {
      Piece piece = pieceTypeToPiece(pieceMap[square]);
      Color color = pieceTypeToColor(pieceMap[square]);
      zobristHash ^= zobrist::getPieceKey(color, piece, square);
    }
  }
  zobristHash ^= zobrist::getSideToMoveKey(whiteToMove ? WHITE : BLACK);
  zobristHash ^= zobrist::getEnPassantKey(enPassantSquare);
  zobristHash ^= zobrist::getCastlingKey(castlingRights);
}

std::string Board::toFen() const {
  std::string fen;
  for (int row = 7; row >= 0; --row) {
    int emptyCount = 0;
    for (int col = 0; col < 8; ++col) {
      int square = row * 8 + col;
      if (pieceMap[square] == NO_PIECE_TYPE) {
        ++emptyCount;
      } else {
        if (emptyCount > 0) {
          fen += std::to_string(emptyCount);
          emptyCount = 0;
        }
        fen += pieceTypeToChar(pieceMap[square]);
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
  fen += squareToString(enPassantSquare);
  fen += ' ';
  fen += std::to_string(halfmoveClock);
  fen += ' ';
  fen += std::to_string(fullmoveClock);
  return fen;
}

std::ostream &operator<<(std::ostream &os, const Board &b) {
  constexpr std::string_view separator = "+---+---+---+---+---+---+---+---+\n";
  std::string output{separator};
  for (int r = 7; r >= 0; r--) {
    output += "| ";
    for (int c = 0; c < 8; c++) {
      Square square = static_cast<Square>(r * 8 + c);
      output += b.getPieceFromMap(square) == NO_PIECE_TYPE
                    ? ' '
                    : pieceTypeToChar(b.getPieceFromMap(square));
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
            squareToString(static_cast<Square>(b.getEnPassantSquare())) +
            std::string("\n");
  output += "Halfmove clock: " + std::to_string(b.getHalfmoveClock()) + "\n";
  output += "Fullmove clock: " + std::to_string(b.getFullmoveClock()) + "\n";
  output += "Zobrist hash: " + std::to_string(b.getZobristHash()) + "\n";
  output += "FEN: " + b.toFen() + "\n";
  os << output;
  return os;
}
} // namespace citterfish