#include "attacks.h"
#include "board.h"
#include "perft.h"
#include "zobrist.h"
#include <iostream>

int main() {
  using namespace citterfish;
  zobrist::init_zobrist();
  attacks::init_attacks();

  Board board;
  std::cout << board << std::endl;
  perft(7, board);

  return 0;
}