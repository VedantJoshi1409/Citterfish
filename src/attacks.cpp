#include "attacks.h"
#include "types.h"
#include <array>

namespace citterfish::attacks {
    namespace {
        std::array<Bitboard, 64> knightAttacks;
        std::array<Bitboard, 64> kingAttacks;
        std::array<Bitboard, 64> bishopMasks;
        std::array<Bitboard, 64> rookMasks;
        
        void initializeKnightAttacks() {
        for (int square = 0; square < 64; ++square) {
            Bitboard attacks = 0;
            int rank = square / 8;
            int file = square % 8;

            if (rank < 7) { //Can do up left left or up right right
                if (file > 1) { //up left left
                    attacks |= 1ULL << (square + NORTH + WEST + WEST);
                }
                if (file < 6) { //up right right
                    attacks |= 1ULL << (square + NORTH + EAST + EAST);
                }
                if (rank < 6) { //Can do up up left or up up right
                    if (file > 0) { //up up left
                        attacks |= 1ULL << (square + NORTH + NORTH + WEST);
                    }
                    if (file < 7) { //up up right
                        attacks |= 1ULL << (square + NORTH + NORTH + EAST);
                    }
                }
            }
            if (rank > 0) { //Can do down left left or down right right
                if (file > 1) { //down left left
                    attacks |= 1ULL << (square + SOUTH + WEST + WEST);
                }
                if (file < 6) { //down right right
                    attacks |= 1ULL << (square + SOUTH + EAST + EAST);
                }
                if (rank > 1) { //Can do down down left or down down right
                    if (file > 0) { //down down left
                        attacks |= 1ULL << (square + SOUTH + SOUTH + WEST);
                    }
                    if (file < 7) { //down down right
                        attacks |= 1ULL << (square + SOUTH + SOUTH + EAST);
                    }
                }
            }
            knightAttacks[static_cast<int>(square)] = attacks;
        }
    }

        void initializeKingAttacks() {
            for (int square = 0; square < 64; ++square) {
                Bitboard attacks = 0;
                int rank = square / 8;
                int file = square % 8;

                if (rank < 7) { //Can do up left, up, up right
                    if (file > 0) { //up left
                        attacks |= 1ULL << (square + NORTH + WEST);
                    }
                    attacks |= 1ULL << (square + NORTH); //up
                    if (file < 7) { //up right
                        attacks |= 1ULL << (square + NORTH + EAST);
                    }
                }
                if (file > 0) { //Can do left
                    attacks |= 1ULL << (square + WEST);
                }
                if (file < 7) { //Can do right
                    attacks |= 1ULL << (square + EAST);
                }
                if (rank > 0) { //Can do down left, down, down right
                    if (file > 0) { //down left
                        attacks |= 1ULL << (square + SOUTH + WEST);
                    }
                    attacks |= 1ULL << (square + SOUTH); //down
                    if (file < 7) { //down right
                        attacks |= 1ULL << (square + SOUTH + EAST);
                    }
                }
                kingAttacks[static_cast<int>(square)] = attacks;
            }
        }

        void initializeBishopMasks() {
            for (int square = 0; square < 64; ++square) {
                Bitboard mask = 0;
                int rank = square / 8;
                int file = square % 8;

                for (int r = rank + 1, f = file + 1; r < 7 && f < 7; ++r, ++f) { // Up-Right
                    mask |= 1ULL << (r * 8 + f);
                }
                for (int r = rank + 1, f = file - 1; r < 7 && f > 0; ++r, --f) { // Up-Left
                    mask |= 1ULL << (r * 8 + f);
                }
                for (int r = rank - 1, f = file + 1; r > 0 && f < 7; --r, ++f) { // Down-Right
                    mask |= 1ULL << (r * 8 + f);
                }
                for (int r = rank - 1, f = file - 1; r > 0 && f > 0; --r, --f) { // Down-Left
                    mask |= 1ULL << (r * 8 + f);
                }

                bishopMasks[static_cast<int>(square)] = mask;
            }
        }

        void initializeRookMasks() {
            for (int square = 0; square < 64; ++square) {
                Bitboard mask = 0;
                int rank = square / 8;
                int file = square % 8;

                for (int r = rank + 1; r < 7; ++r) { // Up
                    mask |= 1ULL << (r * 8 + file);
                }
                for (int r = rank - 1; r > 0; --r) { // Down
                    mask |= 1ULL << (r * 8 + file);
                }
                for (int f = file + 1; f < 7; ++f) { // Right
                    mask |= 1ULL << (rank * 8 + f);
                }
                for (int f = file - 1; f > 0; --f) { // Left
                    mask |= 1ULL << (rank * 8 + f);
                }

                rookMasks[static_cast<int>(square)] = mask;
            }
        }

        Bitboard slowRookAttack(Square square, Bitboard occupancy) {
            Bitboard attacks = 0;
            int rank = square / 8;
            int file = square % 8;

            // Up
            for (int r = rank + 1; r < 8; ++r) {
                attacks |= 1ULL << (r * 8 + file);
                if (occupancy & (1ULL << (r * 8 + file))) break;
            }
            // Down
            for (int r = rank - 1; r >= 0; --r) {
                attacks |= 1ULL << (r * 8 + file);
                if (occupancy & (1ULL << (r * 8 + file))) break;
            }
            // Right
            for (int f = file + 1; f < 8; ++f) {
                attacks |= 1ULL << (rank * 8 + f);
                if (occupancy & (1ULL << (rank * 8 + f))) break;
            }
            // Left
            for (int f = file - 1; f >= 0; --f) {
                attacks |= 1ULL << (rank * 8 + f);
                if (occupancy & (1ULL << (rank * 8 + f))) break;
            }

            return attacks;
        }

        Bitboard slowBishopAttack(Square square, Bitboard occupancy) {
            Bitboard attacks = 0;
            int rank = square / 8;
            int file = square % 8;

            // Up-Right
            for (int r = rank + 1, f = file + 1; r < 8 && f < 8; ++r, ++f) {
                attacks |= 1ULL << (r * 8 + f);
                if (occupancy & (1ULL << (r * 8 + f))) break;
            }
            // Up-Left
            for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; ++r, --f) {
                attacks |= 1ULL << (r * 8 + f);
                if (occupancy & (1ULL << (r * 8 + f))) break;
            }
            // Down-Right
            for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; --r, ++f) {
                attacks |= 1ULL << (r * 8 + f);
                if (occupancy & (1ULL << (r * 8 + f))) break;
            }
            // Down-Left
            for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; --r, --f) {
                attacks |= 1ULL << (r * 8 + f);
                if (occupancy & (1ULL << (r * 8 + f))) break;
            }

            return attacks;
        }
        
        Bitboard PDEP(Bitboard source, Bitboard mask) {
            Bitboard result;
            while(mask) {
                Bitboard lsb = mask & -mask; // Get least significant bit of mask
                if (source & 1) {
                    result |= lsb; // Set the corresponding bit in result
                }
                source >>= 1; // Shift source to process the next bit
                mask &= mask - 1; // Clear the least significant bit of mask
            }
            return result;
        }

        void fillOccupancyTable(Bitboard attackMask, std::array<Bitboard, 4096>& occupancyTable) {
            int bitcount = std::popcount(attackMask);
            int tableSize = 1 << bitcount;
            for (int i = 0; i < tableSize; ++i) {
                occupancyTable[i] = PDEP(i, attackMask);
            }
        }

        void findRookMagics(Square square, int epochs, uint64_t seed=0xDEADBEEF) {
            PRNG rng(seed);
            Bitboard mask = rookMasks[static_cast<int>(square)];
            int bitcount = std::popcount(mask);
            int tableSize = 1 << bitcount;
            std::array<Bitboard, 4096> occupancyTable;
            fillOccupancyTable(mask, occupancyTable);
            for (int epoch = 0; epoch < epochs; ++epoch) {
                
            }
        }
    }

    void initializeAttacks() {
        initializeKnightAttacks();
        initializeKingAttacks();
        initializeBishopMasks();
        initializeRookMasks();
    }

    Bitboard getKnightAttacks(Square square) {
        return knightAttacks[static_cast<int>(square)];
    }

    Bitboard getKingAttacks(Square square) {
        return kingAttacks[static_cast<int>(square)];
    }
}
