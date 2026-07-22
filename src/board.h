#pragma once

#include <cstdint>
#include <array>

using Bitboard = std::uint64_t;

enum Color {
    WHITE,
    BLACK
};

enum Piece{
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

class Board {
    
    std::array<std::array<Bitboard, 6>, 2> pieces;
    std::array<Bitboard, 2> occupied;
    std::array<std::array<bool, 2>, 2> castlingRights;
    bool whiteToMove;
    Bitboard enPassantSquare;
    uint16_t halfmoveClock;
    uint64_t zobristHash;
    

};