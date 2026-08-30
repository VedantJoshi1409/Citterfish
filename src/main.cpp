#include "attacks.h"
#include "board.h"
#include "perft.h"
#include "uci.h"
#include "zobrist.h"
#include <iostream>

int main() {
  using namespace citterfish;
  zobrist::init_zobrist();
  attacks::init_attacks();
  temp_perft();

  return 0;
}