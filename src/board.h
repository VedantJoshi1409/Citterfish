#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <ostream>
#include <sstream>
#include <bit>
#include "types.h"

class Board {
    
private:
    std::array<std::array<Bitboard, 6>, 2> pieces;
    std::array<Bitboard, 2> occupied;
    uint8_t castlingRights; // First digit is wShort castle, Second is wLong, and so on
    bool whiteToMove;
    Bitboard enPassantSquare;
    uint16_t halfmoveClock;
    uint64_t zobristHash;
    

public:
    Board(const std::string& fen);
    Board();

    std::array<std::array<std::string, 8>, 8> toStringBoard() const {
        std::array<std::array<std::string, 8>, 8> stringBoard;
        for (Color color : {WHITE, BLACK}) {
            for (Piece piece : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
                Bitboard bitboard = pieces[color][piece];
                while (bitboard) {
                    int square = std::countr_zero(bitboard);
                    int row = square / 8;
                    int col = square % 8;
                    stringBoard[row][col] = std::string(1, pieceToChar(piece, color));
                    bitboard &= bitboard - 1; // Clear lsb
                }
            }
        }
        return stringBoard;
    }
};

std::ostream& operator<<(std::ostream& os, const Board& b);