#include "board.h"
#include "zobrist.h"
#include <iostream>

int main() {
  citterfish::zobrist::initializeZobristKeys();
  citterfish::Board board;
  std::cout << board << std::endl;
  return 0;
}