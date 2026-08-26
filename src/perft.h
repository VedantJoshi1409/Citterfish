#pragma once

#include "board.h"
#include "movegen.h"
#include "types.h"

namespace citterfish {
    inline uint64_t perft(int depth, Board &b) {
        if (depth == 0) {
            return 1;
        }
        MoveList moveList;
        gen_moves(b, moveList);
        StateInfo st;
        for (int i = 0; i < moveList.count; i++) {
            b.make_move(moveList.moves[i], &st);
        }
        return 0;
    }
}
