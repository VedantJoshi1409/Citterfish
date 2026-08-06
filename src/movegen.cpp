#include "movegen.h"
#include "types.h"
#include "board.h"

namespace citterfish {
    template <Direction D>
    void generatePawnMoves(Bitboard pawns, Bitboard enemyOccupied, Bitboard occupied, MoveList &moveList) {
        Bitboard pawnsBelow7 = pawns & ~(D == NORTH ? RANK_7 : RANK_2);
        Bitboard pawnsOn7 = pawns & (D == NORTH ? RANK_7 : RANK_2);
        Bitboard bb = (pawnsBelow7 << D) & ~occupied; //Generate single pawn push
        while (bb) {
            Square to = static_cast<Square>(std::countr_zero(bb));
            Square from = static_cast<Square>(to - D);
            moveList.addMove(Move(from, to));
            bb &= bb - 1; //clear lsb
        }

        bb = (pawnsBelow7 & (D == NORTH ? RANK_2 : RANK_7)) << (2 * D) & ~occupied; //Generate double pawn push
        while (bb) {
            Square to = static_cast<Square>(std::countr_zero(bb));
            Square from = static_cast<Square>(to - 2 * D);
            moveList.addMove(Move(from, to));
            bb &= bb - 1; //clear lsb
        }

        bb = (pawnsBelow7 & ~FILE_A) << (D + WEST) & enemyOccupied; //Generate captures to the left
        while (bb) {
            Square to = static_cast<Square>(std::countr_zero(bb));
            Square from = static_cast<Square>(to - D - WEST);
            moveList.addMove(Move(from, to));
            bb &= bb - 1; //clear lsb
        }

        bb = (pawnsBelow7 & ~FILE_H) << (D + EAST) & enemyOccupied; //Generate captures to the right
        while (bb) {
            Square to = static_cast<Square>(std::countr_zero(bb));
            Square from = static_cast<Square>(to - D - EAST);
            moveList.addMove(Move(from, to));
            bb &= bb - 1; //clear lsb
        }

        bb = pawnsOn7 << D & ~occupied; //Generate single pawn push for promotion
        while (bb) {
            Square to = static_cast<Square>(std::countr_zero(bb));
            Square from = static_cast<Square>(to - D);
            moveList.addMove(Move(from, to, PROMOTION, QUEEN));
            moveList.addMove(Move(from, to, PROMOTION, KNIGHT));
            moveList.addMove(Move(from, to, PROMOTION, ROOK));
            moveList.addMove(Move(from, to, PROMOTION, BISHOP));
            bb &= bb - 1; //clear lsb
        }

        bb = (pawnsOn7 & ~FILE_A) << (D + WEST) & enemyOccupied; //Generate captures to the left for promotion
        while (bb) {
            Square to = static_cast<Square>(std::countr_zero(bb));
            Square from = static_cast<Square>(to - D - WEST);
            moveList.addMove(Move(from, to, PROMOTION, QUEEN));
            moveList.addMove(Move(from, to, PROMOTION, KNIGHT));
            moveList.addMove(Move(from, to, PROMOTION, ROOK));
            moveList.addMove(Move(from, to, PROMOTION, BISHOP));
            bb &= bb - 1; //clear lsb
        }

        bb = (pawnsOn7 & ~FILE_H) << (D + EAST) & enemyOccupied; //Generate captures to the right for promotion
        while (bb) {
            Square to = static_cast<Square>(std::countr_zero(bb));
            Square from = static_cast<Square>(to - D - EAST);
            moveList.addMove(Move(from, to, PROMOTION, QUEEN));
            moveList.addMove(Move(from, to, PROMOTION, KNIGHT));
            moveList.addMove(Move(from, to, PROMOTION, ROOK));
            moveList.addMove(Move(from, to, PROMOTION, BISHOP));
            bb &= bb - 1; //clear lsb
        }
    }

    void generateMoves(const Board &board, MoveList &moveList) {
        moveList.count = 0;
        generatePawnMoves<NORTH>(board.getPieces(WHITE, PAWN), 0ULL, board.getPieces(WHITE, PAWN), moveList);
    }
}