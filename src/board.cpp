#include "board.h"
#include "types.h"
#include "zobrist.h"
#include <iostream>
#include <string>
#include <string_view>

namespace citterfish {
Board::Board(const std::string &fen) {
  pieceMap.fill(NO_PIECE_TYPE);
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
      pieceMap[index] = piece_to_piece_type(char_to_piece(c), char_to_color(c));
      ++index;
    }
  }
  isWhite = fen.at(i + 1) == 'w';
  i += 3;
  st->castlingRights = 0;
  for (; i < fen.length() && fen.at(i) != ' '; i++) {
    switch (fen.at(i)) {
    case 'K':
      st->castlingRights |= WHITE_KINGSIDE;
      break;
    case 'Q':
      st->castlingRights |= WHITE_QUEENSIDE;
      break;
    case 'k':
      st->castlingRights |= BLACK_KINGSIDE;
      break;
    case 'q':
      st->castlingRights |= BLACK_QUEENSIDE;
      break;
    }
  }
  if (fen.at(i + 1) == '-') {
    st->enPassantSquare = NO_SQUARE;
    i += 3;
  } else {
    st->enPassantSquare = string_to_square(fen.substr(i + 1, 2));
    i += 4;
  }

  if (i >= fen.size()) {
    st->halfmoveClock = 0;
    fullmoveClock = 1;
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
    st->halfmoveClock = half == "" ? 0 : std::stoi(half);
    fullmoveClock = full == "" ? 1 : std::stoi(full);
  }

  refresh_bitboards();
  refresh_zobrist_hash();
}

Board::Board()
    : Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {}

void Board::refresh_piece_map() {
  pieceMap.fill(NO_PIECE_TYPE);
  for (Color color : {WHITE, BLACK}) {
    for (Piece piece : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
      Bitboard bitboard = pieces[color][piece];
      while (bitboard) {
        int square = std::countr_zero(bitboard);
        pieceMap[square] = piece_to_piece_type(piece, color);
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
    if (pieceMap[square] != NO_PIECE_TYPE) {
      Piece piece = piece_type_to_piece(pieceMap[square]);
      Color color = piece_type_to_color(pieceMap[square]);
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

void Board::refresh_zobrist_hash() {
  st->zobristHash = 0;
  for (Square square = a1; square <= h8; ++square) {
    if (pieceMap[square] != NO_PIECE_TYPE) {
      Piece piece = piece_type_to_piece(pieceMap[square]);
      Color color = piece_type_to_color(pieceMap[square]);
      st->zobristHash ^= zobrist::getPieceKey(color, piece, square);
    }
  }
  st->zobristHash ^= zobrist::getSideToMoveKey(isWhite ? WHITE : BLACK);
  st->zobristHash ^= zobrist::getEnPassantKey(st->enPassantSquare);
  st->zobristHash ^= zobrist::getCastlingKey(st->castlingRights);
}

template <Color us> void Board::refresh_checks_pins() {
  constexpr Color them = ~us;
  constexpr Direction D = (us == WHITE) ? SOUTH : NORTH;
  Square kingSquare =
      static_cast<Square>(std::countr_zero(get_pieces<KING>(us)));
  // Get all knight checkers
  Bitboard curCheckers =
      attacks::knight_attacks(kingSquare) & get_pieces<KNIGHT>(them);

  // Get all pawn checkers
  curCheckers |=
      attacks::pawn_attackers<us>(kingSquare) & get_pieces<PAWN>(them);

  Bitboard king_ortho = attacks::sliding_attacks<ROOK>(
      kingSquare,
      get_pieces(them)); // Where the king can be orthogonally attacked from
  Bitboard king_diag = attacks::sliding_attacks<BISHOP>(
      kingSquare,
      get_pieces(them)); // Where the king can be diagonally attacked from
  Bitboard kingAttackers =
      (king_ortho & (get_pieces<ROOK>(them) | get_pieces<QUEEN>(them))) |
      (king_diag & (get_pieces<BISHOP>(them) | get_pieces<QUEEN>(them)));
  Bitboard curPinned = 0;
  Bitboard curPinners = 0;
  while (kingAttackers) {
    Square attacker = static_cast<Square>(std::countr_zero(kingAttackers));
    Bitboard blockers =
        attacks::from_to_bb(kingSquare, attacker) & get_pieces(us);
    if (blockers == 0) {
      curCheckers |= 1ULL << attacker;
    } else if ((blockers & (blockers - 1)) == 0) {
      curPinned |= blockers;
      curPinners |= 1ULL << attacker;
    }
    kingAttackers &= kingAttackers - 1;
  }
  st->checkers = curCheckers;
  st->pinnedPieces = curPinned;
  st->pinners = curPinners;
}
template void Board::refresh_checks_pins<WHITE>();
template void Board::refresh_checks_pins<BLACK>();

template <Color us> void Board::make_move(Move move, StateInfo *newSt) {
  constexpr Color them = ~us;
  Square from = move.get_from_square();
  Square to = move.get_to_square();
  Bitboard fromBB = 1ULL << from;
  Bitboard toBB = 1ULL << to;
  MoveType type = move.get_move_type();
  Piece moving = piece_type_to_piece(piece_on(from));

  // copy the incremental fields of stateInfo
  memcpy(newSt, st, offsetof(StateInfo, checkers));
  newSt->prevSt = st;
  st = newSt;
  ++st->halfmoveClock;
  isWhite = !isWhite;
  ++fullmoveClock;

  if (type == REGULAR) { // if regular just remove captured, and move moving
    PieceType captured = piece_on(to);
    if (captured != NO_PIECE_TYPE) {
      Piece capturedPiece = piece_type_to_piece(captured);
      st->captured = capturedPiece;
      capture_piece<us>(moving, capturedPiece, from, to, fromBB, toBB);
    } else {
      st->captured = NO_PIECE;
      move_piece<us>(moving, from, to, fromBB, toBB);
    }
  }
}
template void Board::make_move<WHITE>(Move move, StateInfo *newSt);
template void Board::make_move<BLACK>(Move move, StateInfo *newSt);

std::string Board::to_fen() const {
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
        fen += piece_type_to_char(pieceMap[square]);
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
  fen += (isWhite ? 'w' : 'b');
  fen += ' ';
  if (st->castlingRights == 0) {
    fen += '-';
  } else {
    if (st->castlingRights & WHITE_KINGSIDE)
      fen += 'K';
    if (st->castlingRights & WHITE_QUEENSIDE)
      fen += 'Q';
    if (st->castlingRights & BLACK_KINGSIDE)
      fen += 'k';
    if (st->castlingRights & BLACK_QUEENSIDE)
      fen += 'q';
  }
  fen += ' ';
  fen += square_to_string(st->enPassantSquare);
  fen += ' ';
  fen += std::to_string(st->halfmoveClock);
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
      output += b.piece_on(square) == NO_PIECE_TYPE
                    ? ' '
                    : piece_type_to_char(b.piece_on(square));
      output += " | ";
    }
    output += "\n" + std::string(separator);
  }
  output += std::string((b.is_white() ? "White" : "Black")) + " to move\n";
  std::string castle{};
  if ((b.castling_rights() & WHITE_KINGSIDE) != 0)
    castle += 'K';
  if ((b.castling_rights() & WHITE_QUEENSIDE) != 0)
    castle += 'Q';
  if ((b.castling_rights() & BLACK_KINGSIDE) != 0)
    castle += 'k';
  if ((b.castling_rights() & BLACK_QUEENSIDE) != 0)
    castle += 'q';
  if (castle.empty())
    castle = '-';
  output += "Castle rights: " + castle + "\n";
  output += "En passant square: " +
            square_to_string(static_cast<Square>(b.en_passant_square())) +
            std::string("\n");
  output += "Halfmove clock: " + std::to_string(b.halfmove_clock()) + "\n";
  output += "Fullmove clock: " + std::to_string(b.fullmove_clock()) + "\n";
  output += "Zobrist hash: " + std::to_string(b.zobrist_hash()) + "\n";
  output += "FEN: " + b.to_fen() + "\n";
  os << output;
  return os;
}
} // namespace citterfish