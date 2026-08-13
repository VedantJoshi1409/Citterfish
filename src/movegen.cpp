#include "movegen.h"
#include "attacks.h"
#include "board.h"
#include "types.h"

namespace citterfish {
template <Direction D>
void generatePawnMoves(Bitboard pawns, Bitboard enemyOccupied,
                       Bitboard occupied, MoveList &moveList) {
  Bitboard pawnsBelow7 = pawns & ~(D == NORTH ? RANK_7 : RANK_2);
  Bitboard pawnsOn7 = pawns & (D == NORTH ? RANK_7 : RANK_2);
  Bitboard bb = (pawnsBelow7 << D) & ~occupied; // Generate single pawn push
  while (bb) {
    Square to = static_cast<Square>(std::countr_zero(bb));
    Square from = static_cast<Square>(to - D);
    moveList.addMove(Move(from, to));
    bb &= bb - 1; // clear lsb
  }

  bb = (pawnsBelow7 & (D == NORTH ? RANK_2 : RANK_7)) << (2 * D) &
       ~occupied; // Generate double pawn push
  while (bb) {
    Square to = static_cast<Square>(std::countr_zero(bb));
    Square from = static_cast<Square>(to - 2 * D);
    moveList.addMove(Move(from, to));
    bb &= bb - 1; // clear lsb
  }

  bb = (pawnsBelow7 & ~FILE_A) << (D + WEST) &
       enemyOccupied; // Generate captures to the left
  while (bb) {
    Square to = static_cast<Square>(std::countr_zero(bb));
    Square from = static_cast<Square>(to - D - WEST);
    moveList.addMove(Move(from, to));
    bb &= bb - 1; // clear lsb
  }

  bb = (pawnsBelow7 & ~FILE_H) << (D + EAST) &
       enemyOccupied; // Generate captures to the right
  while (bb) {
    Square to = static_cast<Square>(std::countr_zero(bb));
    Square from = static_cast<Square>(to - D - EAST);
    moveList.addMove(Move(from, to));
    bb &= bb - 1; // clear lsb
  }

  bb = pawnsOn7 << D & ~occupied; // Generate single pawn push for promotion
  while (bb) {
    Square to = static_cast<Square>(std::countr_zero(bb));
    Square from = static_cast<Square>(to - D);
    moveList.addMove(Move(from, to, PROMOTION, QUEEN));
    moveList.addMove(Move(from, to, PROMOTION, KNIGHT));
    moveList.addMove(Move(from, to, PROMOTION, ROOK));
    moveList.addMove(Move(from, to, PROMOTION, BISHOP));
    bb &= bb - 1; // clear lsb
  }

  bb = (pawnsOn7 & ~FILE_A) << (D + WEST) &
       enemyOccupied; // Generate captures to the left for promotion
  while (bb) {
    Square to = static_cast<Square>(std::countr_zero(bb));
    Square from = static_cast<Square>(to - D - WEST);
    moveList.addMove(Move(from, to, PROMOTION, QUEEN));
    moveList.addMove(Move(from, to, PROMOTION, KNIGHT));
    moveList.addMove(Move(from, to, PROMOTION, ROOK));
    moveList.addMove(Move(from, to, PROMOTION, BISHOP));
    bb &= bb - 1; // clear lsb
  }

  bb = (pawnsOn7 & ~FILE_H) << (D + EAST) &
       enemyOccupied; // Generate captures to the right for promotion
  while (bb) {
    Square to = static_cast<Square>(std::countr_zero(bb));
    Square from = static_cast<Square>(to - D - EAST);
    moveList.addMove(Move(from, to, PROMOTION, QUEEN));
    moveList.addMove(Move(from, to, PROMOTION, KNIGHT));
    moveList.addMove(Move(from, to, PROMOTION, ROOK));
    moveList.addMove(Move(from, to, PROMOTION, BISHOP));
    bb &= bb - 1; // clear lsb
  }
}

void generateKnightMoves(Bitboard knights, Bitboard friendlyOccupied,
                         MoveList &moveList) {
  Bitboard bb = knights;
  while (bb) {
    Square from = static_cast<Square>(std::countr_zero(bb));
    Bitboard attacks = attacks::getKnightAttacks(from) & ~friendlyOccupied;
    while (attacks) {
      Square to = static_cast<Square>(std::countr_zero(attacks));
      moveList.addMove(Move(from, to));
      attacks &= attacks - 1; // clear lsb
    }
    bb &= bb - 1; // clear lsb
  }
}

void generateKingMoves(Bitboard king, Bitboard friendlyOccupied,
                       MoveList &moveList) {
  Bitboard bb = king;
  Square from = static_cast<Square>(std::countr_zero(bb));
  Bitboard attacks = attacks::getKingAttacks(from) & ~friendlyOccupied;
  while (attacks) {
    Square to = static_cast<Square>(std::countr_zero(attacks));
    moveList.addMove(Move(from, to));
    attacks &= attacks - 1; // clear lsb
  }
}

template <Color C>
void generatePinnedMoves(Board &b, MoveList &moveList) {
  Direction D = (C == WHITE) ? NORTH : SOUTH;
  Bitboard pinners = b.getPinners();
  Square kingSquare = static_cast<Square>(std::countr_zero(b.getPieces<KING>(C)));
    while (pinners) { //get all pinned piece moves
      Square attacker = static_cast<Square>(std::countr_zero(pinners));
      Bitboard pinRay = attacks::getFromToBitboard(kingSquare, attacker) | 1ULL << attacker;
      Square pinnedPiece = static_cast<Square>(std::countr_zero(pinRay & b.getPinned()));

      Piece piece = pieceTypeToPiece(b.getPieceFromMap(pinnedPiece));
      if (piece == PAWN) {
        //Cant enpassant if pinned
        Bitboard pawnBB = 1ULL << pinnedPiece;
        Bitboard pawnMoves = ((pawnBB << (D + EAST)) | (pawnBB << (D + WEST))) & attacker; //attacking squares
        pawnMoves |= pawnBB << D;
        if ((pawnBB & ((C == WHITE) ? RANK_2 : RANK_7)) != 0) pawnMoves |= pawnBB << 2*D;
        pawnMoves &= pinRay; //only keep moves along pin ray
        while (pawnMoves) {
          Square toSquare = static_cast<Square>(std::countr_zero(pawnMoves));
          if ((pawnBB & ((C == WHITE) ? RANK_7 : RANK_2)) != 0) { //promoting
            moveList.addMove(Move(pinnedPiece, toSquare, PROMOTION, QUEEN));
            moveList.addMove(Move(pinnedPiece, toSquare, PROMOTION, KNIGHT));
            moveList.addMove(Move(pinnedPiece, toSquare, PROMOTION, ROOK));
            moveList.addMove(Move(pinnedPiece, toSquare, PROMOTION, BISHOP));
          } else {
            moveList.addMove(Move(pinnedPiece, toSquare));
          }
          pawnMoves &= pawnMoves-1;
        }


      } else {
        switch (piece) {
        case BISHOP:
        pinRay &= attacks::getDiagRays(pinnedPiece);
        break;
        case ROOK: 
        //only can be pinned once rook moves so no castling
        pinRay &= attacks::getOrthoRays(pinnedPiece);
        break;
        case QUEEN:
        pinRay &= (attacks::getOrthoRays(pinnedPiece) | attacks::getDiagRays(pinnedPiece));
        break;
        default:
        break;
      }
      while (pinRay) {
        moveList.addMove(Move(pinnedPiece, static_cast<Square>(std::countr_zero(pinRay))));
        pinRay &= pinRay -1;
      }
      }
    }
}

template <Color C>
void generateMoves(Board &b, MoveList &moveList) {
  moveList.count = 0;
  Square kingSquare = static_cast<Square>(std::countr_zero(b.getPieces<KING>(C)));
  b.refreshChecksAndPins<C>();
  if (b.getCheckers() == 0) { //no checkers
    
    

  } else if ((b.getCheckers() & (b.getCheckers()-1)) == 0) { //one checker

  } else { //two checkers
    
  }
}
} // namespace citterfish