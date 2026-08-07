#pragma once

#include "types.h"

namespace citterfish::attacks {
    void initializeAttacks();
    Bitboard getKnightAttacks(Square square);
    Bitboard getKingAttacks(Square square);
    Bitboard getBishopAttacks(Square square);
    Bitboard getRookAttacks(Square square);
}