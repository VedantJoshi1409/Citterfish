#include "board.h"
#include "zobrist.h"
#include "movegen.h"
#include <iostream>


int main() {
  using namespace citterfish;
  zobrist::initializeZobristKeys();
  Board board("8/P7/1P6/1P6/8/8/8/8 w KQkq - 0 1");
  std::cout << board << std::endl;

  MoveList moveList;
  generateMoves(board, moveList);
  std::cout << static_cast<uint16_t>(moveList.count) << " moves generated." << std::endl;
  for (uint8_t i = 0; i < moveList.count; ++i) {
    const Move &move = moveList.moves[i];
    std::cout << squareToString(move.getFromSquare()) << " -> "
              << squareToString(move.getToSquare()) << std::endl;
    if (move.getMoveType() == PROMOTION) {
      std::cout << "Promotion to "
                << pieceToChar(move.getPromotionPiece(), WHITE) << std::endl;
    }
  }

  return 0;
}