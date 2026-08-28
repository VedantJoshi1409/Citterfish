#include "attacks.h"
#include "board.h"
#include "magics.h"
#include "types.h"
#include "zobrist.h"
#include "perft.h"
#include <iostream>

int main() {
  using namespace citterfish;
  zobrist::init_zobrist();
  attacks::init_attacks();
  
  print_bb(9330740550166839185ULL);
  print_bb(ROOK_ATTACK_TABLE[88024]);
  print_bb(attacks::sliding_attacks<ROOK>(h8, 9330740550166839185ULL));
  // Board board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");//this is kiwipete
  // StateInfo st;
  // board.make_move(Move(f3,h3), &st);
  // board.make_move(Move(b4,b3), &st);
  // board.make_move(Move(h3,h8), &st);
  // board.make_move(Move(e8,d8), &st);
  // board.make_move(Move(h8,d8), &st);

  // // board.unmake_move(Move(e8,g8, CASTLE));

  
  // // board.unmake_move(move);
  // // board.refresh_piece_map();
  // std::cout << board << std::endl;  
  // perft(1,board);

  return 0;
}