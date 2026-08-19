#include "board.h"
#include "types.h"
#include "zobrist.h"
#include <iostream>
#include <ranges>
#include <string>

namespace citterfish {
Board::Board(const std::string &fen) {
  piece_map.fill(NO_PIECE_TYPE);
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
      piece_map[index] =
          piece_to_piece_type(char_to_piece(c), char_to_color(c));
      ++index;
    }
  }
  is_white = fen.at(i + 1) == 'w';
  i += 3;
  castling_rights = 0;
  for (; i < fen.length() && fen.at(i) != ' '; i++) {
    switch (fen.at(i)) {
    case 'K':
      castling_rights |= WHITE_KINGSIDE;
      break;
    case 'Q':
      castling_rights |= WHITE_QUEENSIDE;
      break;
    case 'k':
      castling_rights |= BLACK_KINGSIDE;
      break;
    case 'q':
      castling_rights |= BLACK_QUEENSIDE;
      break;
    }
  }
  en_passant_square = (fen.at(i + 1) == '-')
                          ? no_square
                          : string_to_square(fen.substr(i + 1, 2));
  halfmove_clock = (i + 3) < fen.size() ? fen.at(i + 3) - '0' : 0;
  fullmove_clock = (i + 5) < fen.size() ? fen.at(i + 5) - '0' : 0;
  refresh_bitboards();
  refresh_zobrist_hash();
}

Board::Board()
    : Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {}

void Board::refresh_piece_map() {
  piece_map.fill(NO_PIECE_TYPE);
  for (Color color : {WHITE, BLACK}) {
    for (Piece piece : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
      Bitboard bitboard = pieces[color][piece];
      while (bitboard) {
        int square = std::countr_zero(bitboard);
        piece_map[square] = piece_to_piece_type(piece, color);
        bitboard &= bitboard - 1; // Clear lsb
      }
    }
  }
}

void Board::refresh_bitboards() {
  for (auto &row : pieces) {
    for (auto &x : row) {
      x = 0;
    }
  }
  for (Square square = a1; square <= h8; ++square) {
    if (piece_map[square] != NO_PIECE_TYPE) {
      Piece piece = piece_type_to_piece(piece_map[square]);
      Color color = piece_type_to_color(piece_map[square]);
      pieces[color][piece] |= 1ULL << square;
    }
  }
  occupied[WHITE] = pieces[WHITE][PAWN] | pieces[WHITE][KNIGHT] |
                    pieces[WHITE][BISHOP] | pieces[WHITE][ROOK] |
                    pieces[WHITE][QUEEN] | pieces[WHITE][KING];
  occupied[BLACK] = pieces[BLACK][PAWN] | pieces[BLACK][KNIGHT] |
                    pieces[BLACK][BISHOP] | pieces[BLACK][ROOK] |
                    pieces[BLACK][QUEEN] | pieces[BLACK][KING];
  all_occupied = occupied[WHITE] | occupied[BLACK];
}

void Board::refresh_zobrist_hash() {
  zobrist_hash = 0;
  for (Square square = a1; square <= h8; ++square) {
    if (piece_map[square] != NO_PIECE_TYPE) {
      Piece piece = piece_type_to_piece(piece_map[square]);
      Color color = piece_type_to_color(piece_map[square]);
      zobrist_hash ^= zobrist::getPieceKey(color, piece, square);
    }
  }
  zobrist_hash ^= zobrist::getSideToMoveKey(is_white ? WHITE : BLACK);
  zobrist_hash ^= zobrist::getEnPassantKey(en_passant_square);
  zobrist_hash ^= zobrist::getCastlingKey(castling_rights);
}

std::string Board::to_fen() const {
  std::string fen;
  for (int row = 7; row >= 0; --row) {
    int emptyCount = 0;
    for (int col = 0; col < 8; ++col) {
      int square = row * 8 + col;
      if (piece_map[square] == NO_PIECE_TYPE) {
        ++emptyCount;
      } else {
        if (emptyCount > 0) {
          fen += std::to_string(emptyCount);
          emptyCount = 0;
        }
        fen += piece_type_to_char(piece_map[square]);
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
  fen += (is_white ? 'w' : 'b');
  fen += ' ';
  if (castling_rights == 0) {
    fen += '-';
  } else {
    if (castling_rights & WHITE_KINGSIDE)
      fen += 'K';
    if (castling_rights & WHITE_QUEENSIDE)
      fen += 'Q';
    if (castling_rights & BLACK_KINGSIDE)
      fen += 'k';
    if (castling_rights & BLACK_QUEENSIDE)
      fen += 'q';
  }
  fen += ' ';
  fen += square_to_string(en_passant_square);
  fen += ' ';
  fen += std::to_string(halfmove_clock);
  fen += ' ';
  fen += std::to_string(fullmove_clock);
  return fen;
}

std::ostream &operator<<(std::ostream &os, const Board &b) {
  constexpr std::string_view separator = "+---+---+---+---+---+---+---+---+\n";
  std::string output{separator};
  for (int r = 7; r >= 0; r--) {
    output += "| ";
    for (int c = 0; c < 8; c++) {
      Square square = static_cast<Square>(r * 8 + c);
      output += b.get_piece_map(square) == NO_PIECE_TYPE
                    ? ' '
                    : piece_type_to_char(b.get_piece_map(square));
      output += " | ";
    }
    output += "\n" + std::string(separator);
  }
  output += std::string((b.get_is_white() ? "White" : "Black")) + " to move\n";
  std::string castle{};
  if ((b.get_castling_rights() & WHITE_KINGSIDE) != 0)
    castle += 'K';
  if ((b.get_castling_rights() & WHITE_QUEENSIDE) != 0)
    castle += 'Q';
  if ((b.get_castling_rights() & BLACK_KINGSIDE) != 0)
    castle += 'k';
  if ((b.get_castling_rights() & BLACK_QUEENSIDE) != 0)
    castle += 'q';
  if (castle.empty())
    castle = '-';
  output += "Castle rights: " + castle + "\n";
  output += "En passant square: " +
            square_to_string(static_cast<Square>(b.get_en_passant_square())) +
            std::string("\n");
  output += "Halfmove clock: " + std::to_string(b.get_halfmove_clock()) + "\n";
  output += "Fullmove clock: " + std::to_string(b.get_fullmove_clock()) + "\n";
  output += "Zobrist hash: " + std::to_string(b.get_zobrist_hash()) + "\n";
  output += "FEN: " + b.to_fen() + "\n";
  os << output;
  return os;
}
} // namespace citterfish