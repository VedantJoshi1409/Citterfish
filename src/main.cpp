#include "attacks.h"
#include "board.h"
#include "movegen.h"
#include "zobrist.h"
#include <iostream>

int main() {
  using namespace citterfish;
  zobrist::init_zobrist();
  attacks::init_attacks();
  Board board("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");
  std::cout << board << std::endl;
  MoveList moveList;
  gen_moves(board, moveList);
  std::cout << static_cast<uint16_t>(moveList.count) << " moves generated."
            << std::endl;
  for (uint8_t i = 0; i < moveList.count; ++i) {
    const Move &move = moveList.moves[i];
    std::cout << square_to_string(move.get_from_square()) << " -> "
              << square_to_string(move.get_to_square()) << ": "
              << static_cast<int>(move.get_move_type()) << std::endl;
  }
  StateInfo st;
  board.make_move<BLACK>(Move(e8, g8, CASTLE), &st);
  std::cout << board << std::endl;

  return 0;
}