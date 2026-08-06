#pragma once

#include "types.h"
#include <array>
#include "board.h"

namespace citterfish {
    
struct MoveList {
    std::array<Move, MAX_MOVES> moves;
    uint8_t count;
    
    void addMove(const Move& move) {
        moves[count++] = move;
    }
};

void generateMoves(const Board &board, MoveList &moveList);

}

