#pragma once

#include "types.h"
#include <array>

namespace citterfish::zobrist {
bool initializeZobristKeys(uint64_t seed = 0xDEADBEEFCAFEBABEULL);
Key getPieceKey(Color color, Piece piece, Square square);
Key getCastlingKey(uint8_t rights);
Key getEnPassantKey(Square square);
Key getSideToMoveKey();
} // namespace citterfish::zobrist