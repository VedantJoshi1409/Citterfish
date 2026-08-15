#pragma once

#include "board.h"
#include "types.h"
#include <array>

namespace citterfish {

struct MoveList {
  std::array<Move, MAX_MOVES> moves;
  uint8_t count;

  void add_move(const Move &move) { moves[count++] = move; }
};

void gen_moves(Board &board, MoveList &move_list);
} // namespace citterfish
