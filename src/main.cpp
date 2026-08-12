#include "attacks.h"
#include "board.h"
#include "movegen.h"
#include "zobrist.h"
#include "attacks.h"
#include <iostream>

int main() {
  using namespace citterfish;
  zobrist::initializeZobristKeys();
  attacks::initializeAttacks();
  Board board;
  std::cout << board << std::endl;

  // MoveList moveList;
  // generateMoves(board, moveList);
  // std::cout << static_cast<uint16_t>(moveList.count) << " moves generated."
  // << std::endl; for (uint8_t i = 0; i < moveList.count; ++i) {
  //   const Move &move = moveList.moves[i];
  //   std::cout << squareToString(move.getFromSquare()) << " -> "
  //             << squareToString(move.getToSquare()) << std::endl;
  // }

  return 0;
}