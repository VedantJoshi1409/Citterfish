#pragma once

#include <cstdint>

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

inline char pieceToChar(Piece piece, Color color) {
    switch (piece) {
        case PAWN:   return color == WHITE ? 'P' : 'p';
        case KNIGHT: return color == WHITE ? 'N' : 'n';
        case BISHOP: return color == WHITE ? 'B' : 'b';
        case ROOK:   return color == WHITE ? 'R' : 'r';
        case QUEEN:  return color == WHITE ? 'Q' : 'q';
        case KING:   return color == WHITE ? 'K' : 'k';
        default:     return '?'; // Should never happen
    }
}