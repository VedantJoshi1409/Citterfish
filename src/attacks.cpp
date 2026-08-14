#include "attacks.h"
#include "types.h"
#include <array>

namespace citterfish::attacks {
namespace {
void initialize_knight_attacks() {
  for (int square = 0; square < 64; ++square) {
    Bitboard attacks = 0;
    int rank = square / 8;
    int file = square % 8;

    if (rank < 7) {   // Can do up left left or up right right
      if (file > 1) { // up left left
        attacks |= 1ULL << (square + NORTH + WEST + WEST);
      }
      if (file < 6) { // up right right
        attacks |= 1ULL << (square + NORTH + EAST + EAST);
      }
      if (rank < 6) {   // Can do up up left or up up right
        if (file > 0) { // up up left
          attacks |= 1ULL << (square + NORTH + NORTH + WEST);
        }
        if (file < 7) { // up up right
          attacks |= 1ULL << (square + NORTH + NORTH + EAST);
        }
      }
    }
    if (rank > 0) {   // Can do down left left or down right right
      if (file > 1) { // down left left
        attacks |= 1ULL << (square + SOUTH + WEST + WEST);
      }
      if (file < 6) { // down right right
        attacks |= 1ULL << (square + SOUTH + EAST + EAST);
      }
      if (rank > 1) {   // Can do down down left or down down right
        if (file > 0) { // down down left
          attacks |= 1ULL << (square + SOUTH + SOUTH + WEST);
        }
        if (file < 7) { // down down right
          attacks |= 1ULL << (square + SOUTH + SOUTH + EAST);
        }
      }
    }
    detail::knight_attacks[static_cast<int>(square)] = attacks;
  }
}

void initialize_king_attacks() {
  for (int square = 0; square < 64; ++square) {
    Bitboard attacks = 0;
    int rank = square / 8;
    int file = square % 8;

    if (rank < 7) {   // Can do up left, up, up right
      if (file > 0) { // up left
        attacks |= 1ULL << (square + NORTH + WEST);
      }
      attacks |= 1ULL << (square + NORTH); // up
      if (file < 7) {                      // up right
        attacks |= 1ULL << (square + NORTH + EAST);
      }
    }
    if (file > 0) { // Can do left
      attacks |= 1ULL << (square + WEST);
    }
    if (file < 7) { // Can do right
      attacks |= 1ULL << (square + EAST);
    }
    if (rank > 0) {   // Can do down left, down, down right
      if (file > 0) { // down left
        attacks |= 1ULL << (square + SOUTH + WEST);
      }
      attacks |= 1ULL << (square + SOUTH); // down
      if (file < 7) {                      // down right
        attacks |= 1ULL << (square + SOUTH + EAST);
      }
    }
    detail::king_attacks[static_cast<int>(square)] = attacks;
  }
}

void initialize_pawn_attackers() {
  for (Square s = a1; s <= h8; ++s) {
    Bitboard w_pawns = 0;
    Bitboard b_pawns = 0;
    Bitboard piece = 1ULL << s;
    if ((piece & FILE_A) == 0) {
      w_pawns |= 1ULL << (s + (SOUTH + WEST)); // wpawns attack from below
      b_pawns |= 1ULL << (s + (NORTH + WEST)); // bpawns attack from above
    }
    if ((piece & FILE_H) == 0) {
      w_pawns |= 1ULL << (s + (SOUTH + EAST)); // wpawns attack from below
      b_pawns |= 1ULL << (s + (NORTH + EAST)); // bpawns attack from above
    }
    detail::pawn_attackers[WHITE][s] =
        b_pawns; // b_pawns attack you if you are white
    detail::pawn_attackers[BLACK][s] =
        w_pawns; // w_pawns attack you if you are black
  }
}

void initialize_from_to_bb() {
  constexpr int dirs[8][2] = {{1, 0}, {-1, 0},  {0, 1},  {0, -1},
                              {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};
  for (Square from = a1; from <= h8; ++from) {
    const int r = from / 8;
    const int f = from % 8;
    for (const auto &[dr, df] : dirs) {
      Bitboard bb = 0;
      for (int i = r + dr, j = f + df; 0 <= i && i < 8 && 0 <= j && j < 8;
           i += dr, j += df) {
        int toSquare = i * 8 + j;
        detail::from_to_bb[from][toSquare] = bb;
        bb |= 1ULL << toSquare;
      }
    }
  }
}

void initialize_rays() {
  for (Square s = a1; s <= h8; ++s) {
    detail::ortho_rays[s] = sliding_attacks<ROOK>(s, 0);
    detail::diag_rays[s] = sliding_attacks<BISHOP>(s, 0);
  }
}
} // namespace

void initialize_attacks() {
  initialize_knight_attacks();
  initialize_king_attacks();
  initialize_pawn_attackers();
  initialize_from_to_bb();
  fill_attack_map<ROOK>();
  fill_attack_map<BISHOP>();
  initialize_rays();
}

} // namespace citterfish::attacks
