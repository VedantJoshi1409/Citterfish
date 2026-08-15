#include "attacks.h"
#include "board.h"
#include "movegen.h"
#include "zobrist.h"
#include <iostream>

int main() {
  using namespace citterfish;
  zobrist::init_zobrist();
  attacks::init_attacks();
  Board board("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 1");
  std::cout << board << std::endl;
  MoveList move_list;
  gen_moves(board, move_list);
  std::cout << static_cast<uint16_t>(move_list.count) << " moves generated."
            << std::endl;
  for (uint8_t i = 0; i < move_list.count; ++i) {
    const Move &move = move_list.moves[i];
    std::cout << square_to_string(move.get_from_square()) << " -> "
              << square_to_string(move.get_to_square()) << ": "
              << static_cast<int>(move.get_move_type()) << std::endl;
  }

  return 0;
}