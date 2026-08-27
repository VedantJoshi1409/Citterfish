#include "attacks.h"
#include "board.h"
#include "zobrist.h"
#include "perft.h"
#include <iostream>

int main() {
  using namespace citterfish;
  zobrist::init_zobrist();
  attacks::init_attacks();
  Board board;
  StateInfo st;
 board.make_move(Move(c2,c4), &st);
  board.make_move(Move(d7,d5), &st);
  board.make_move(Move(d1,a4), &st);
  
  // board.unmake_move(move);
  // board.refresh_piece_map();
  std::cout << board << std::endl;  
  perft(7,board);

  return 0;
}