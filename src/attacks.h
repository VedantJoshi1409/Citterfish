#pragma once

#include "types.h"

namespace citterfish::attacks {
    void initializeKnightAttacks();
    void initializeKingAttacks();
    void initializeBishopAttacks();
    void initializeRookAttacks();
    Bitboard getKnightAttacks(Square square);
    Bitboard getKingAttacks(Square square);
    Bitboard getBishopAttacks(Square square);
    Bitboard getRookAttacks(Square square);
}