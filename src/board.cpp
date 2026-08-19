#include "board.h"
#include "types.h"
#include "zobrist.h"
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>

namespace citterfish {
Board::Board(const std::string &fen) {
  piece_map.fill(NO_PIECE_TYPE);
  st = new StateInfo();
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
  st->castling_rights = 0;
  for (; i < fen.length() && fen.at(i) != ' '; i++) {
    switch (fen.at(i)) {
    case 'K':
      st->castling_rights |= WHITE_KINGSIDE;
      break;
    case 'Q':
      st->castling_rights |= WHITE_QUEENSIDE;
      break;
    case 'k':
      st->castling_rights |= BLACK_KINGSIDE;
      break;
    case 'q':
      st->castling_rights |= BLACK_QUEENSIDE;
      break;
    }
  }
  if (fen.at(i + 1) == '-') {
    st->en_passant_square = no_square;
    i += 3;
  } else {
    st->en_passant_square = string_to_square(fen.substr(i + 1, 2));
    i+=4;
  }
  
  if (i >= fen.size()) {
    st->halfmove_clock = 0;
    fullmove_clock = 1;
  } else {
    std::string s = fen.substr(i);
    std::string half = "";
    std::string full = "";
    bool doneHalf = false;
    for (char c : s) {
      if (c == ' ') {
        doneHalf = true;
      } else if (!doneHalf) {
        half += c;
      } else {
        full += c;
      }
    }
    st->halfmove_clock = half == "" ? 0 : std::stoi(half);
    fullmove_clock = full == "" ? 1 : std::stoi(full);
  }

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
  st->zobrist_hash = 0;
  for (Square square = a1; square <= h8; ++square) {
    if (piece_map[square] != NO_PIECE_TYPE) {
      Piece piece = piece_type_to_piece(piece_map[square]);
      Color color = piece_type_to_color(piece_map[square]);
      st->zobrist_hash ^= zobrist::getPieceKey(color, piece, square);
    }
  }
  st->zobrist_hash ^= zobrist::getSideToMoveKey(is_white ? WHITE : BLACK);
  st->zobrist_hash ^= zobrist::getEnPassantKey(st->en_passant_square);
  st->zobrist_hash ^= zobrist::getCastlingKey(st->castling_rights);
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
  if (st->castling_rights == 0) {
    fen += '-';
  } else {
    if (st->castling_rights & WHITE_KINGSIDE)
      fen += 'K';
    if (st->castling_rights & WHITE_QUEENSIDE)
      fen += 'Q';
    if (st->castling_rights & BLACK_KINGSIDE)
      fen += 'k';
    if (st->castling_rights & BLACK_QUEENSIDE)
      fen += 'q';
  }
  fen += ' ';
  fen += square_to_string(st->en_passant_square);
  fen += ' ';
  fen += std::to_string(st->halfmove_clock);
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