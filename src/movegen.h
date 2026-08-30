#pragma once

#include "board.h"
#include "types.h"
#include <array>

namespace citterfish {

enum GenType { LEGAL, PSEUDOLEGAL };

struct MoveList {
  std::array<Move, MAX_MOVES> moves;
  uint8_t count;

  void add_move(const Move &move) { moves[count++] = move; }
  void add_promo(const Square from, const Square to) {
    add_move(Move(from, to, QUEEN));
    add_move(Move(from, to, KNIGHT));
    add_move(Move(from, to, ROOK));
    add_move(Move(from, to, BISHOP));
  }
};

void gen_moves(Board &board, MoveList &move_list);
} // namespace citterfish
