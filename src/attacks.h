#pragma once

#include "types.h"

namespace citterfish::attacks {
    void initializeKnightAttacks();
    Bitboard getKnightAttacks(Square square);
}