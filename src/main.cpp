#include "board.h"
#include "zobrist.h"
#include "types.h"
#include <iostream>


int main() {
  using namespace citterfish;
  zobrist::initializeZobristKeys();
  Board board;
  std::cout << board << std::endl;
  return 0;
}