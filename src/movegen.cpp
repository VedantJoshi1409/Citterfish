#include "movegen.h"
#include "attacks.h"
#include "board.h"
#include "types.h"

namespace citterfish {
template <Direction D>
void generatePawnMoves(Bitboard pawns, Bitboard enemyOccupied,
                       Bitboard occupied, MoveList &moveList) {
  constexpr Bitboard filesOn7 = D == NORTH ? RANK_7 : RANK_2;
  Bitboard pawnsBelow7 = pawns & ~filesOn7;
  Bitboard pawnsOn7 = pawns & filesOn7;
  Bitboard bb = (pawnsBelow7 << D) & ~occupied; // Generate single pawn push
  while (bb) {
    Square to = getLeastSquare(bb);
    Square from = static_cast<Square>(to - D);
    moveList.addMove(Move(from, to));
    bb &= bb - 1; // clear lsb
  }

  bb = (pawnsBelow7 & (D == NORTH ? RANK_2 : RANK_7)) << (2 * D) &
       ~occupied; // Generate double pawn push
  while (bb) {
    Square to = getLeastSquare(bb);
    Square from = static_cast<Square>(to - 2 * D);
    moveList.addMove(Move(from, to));
    bb &= bb - 1; // clear lsb
  }

  bb = (pawnsBelow7 & ~FILE_A) << (D + WEST) &
       enemyOccupied; // Generate captures to the left
  while (bb) {
    Square to = getLeastSquare(bb);
    Square from = static_cast<Square>(to - D - WEST);
    moveList.addMove(Move(from, to));
    bb &= bb - 1; // clear lsb
  }

  bb = (pawnsBelow7 & ~FILE_H) << (D + EAST) &
       enemyOccupied; // Generate captures to the right
  while (bb) {
    Square to = getLeastSquare(bb);
    Square from = static_cast<Square>(to - D - EAST);
    moveList.addMove(Move(from, to));
    bb &= bb - 1; // clear lsb
  }

  bb = pawnsOn7 << D & ~occupied; // Generate single pawn push for promotion
  while (bb) {
    Square to = getLeastSquare(bb);
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
    Square to = getLeastSquare(bb);
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
    Square to = getLeastSquare(bb);
    Square from = static_cast<Square>(to - D - EAST);
    moveList.addMove(Move(from, to, PROMOTION, QUEEN));
    moveList.addMove(Move(from, to, PROMOTION, KNIGHT));
    moveList.addMove(Move(from, to, PROMOTION, ROOK));
    moveList.addMove(Move(from, to, PROMOTION, BISHOP));
    bb &= bb - 1; // clear lsb
  }
}

void generateEnPassant(Bitboard pawns);

void generateKnightMoves(Bitboard knights, Bitboard friendlyOccupied,
                         MoveList &moveList) {
  Bitboard bb = knights;
  while (bb) {
    Square from = getLeastSquare(bb);
    Bitboard attacks = attacks::getKnightAttacks(from) & ~friendlyOccupied;
    while (attacks) {
      Square to = getLeastSquare(attacks);
      moveList.addMove(Move(from, to));
      attacks &= attacks - 1; // clear lsb
    }
    bb &= bb - 1; // clear lsb
  }
}

template <Piece P>
void generateSliderMoves(Bitboard pieces, Bitboard friendlyOccupied,
                         Bitboard occupied, MoveList &moveList) {
  while (pieces) {
    Square cur = getLeastSquare(pieces);
  }
}

void generateKingMoves(Bitboard king, Bitboard friendlyOccupied,
                       MoveList &moveList) {
  Bitboard bb = king;
  Square from = getLeastSquare(bb);
  Bitboard attacks = attacks::getKingAttacks(from) & ~friendlyOccupied;
  while (attacks) {
    Square to = getLeastSquare(attacks);
    moveList.addMove(Move(from, to));
    attacks &= attacks - 1; // clear lsb
  }
}

template <Color C> void generatePinnedMoves(Board &b, MoveList &moveList) {
  constexpr Direction D = (C == WHITE) ? NORTH : SOUTH;
  constexpr bool isWhite = C == WHITE;
  Bitboard pinners = b.getPinners();
  Square kingSquare = getLeastSquare(b.getPieces<KING>(C));
  while (pinners) { // get all pinned piece moves
    Square attacker = getLeastSquare(pinners);
    Bitboard pinRay =
        attacks::getFromToBitboard(kingSquare, attacker) | 1ULL << attacker;
    Square pinnedPiece = getLeastSquare(pinRay & b.getPinned());

    Piece piece = pieceTypeToPiece(b.getPieceFromMap(pinnedPiece));
    if (piece == PAWN) {
      if (((1ULL << b.getEnPassantSquare()) & pinRay) != 0) {
      }
      Bitboard pawnBB = 1ULL << pinnedPiece;
      Bitboard pawnMoves = ((pawnBB << (D + EAST)) | (pawnBB << (D + WEST))) &
                           attacker; // attacking squares
      pawnMoves |= pawnBB << D;
      if ((pawnBB & (isWhite ? RANK_2 : RANK_7)) != 0)
        pawnMoves |= pawnBB << 2 * D;
      pawnMoves &= pinRay; // only keep moves along pin ray
      while (pawnMoves) {
        Square toSquare = getLeastSquare(pawnMoves);
        if ((pawnBB & (isWhite ? RANK_7 : RANK_2)) != 0) { // promoting
          moveList.addMove(Move(pinnedPiece, toSquare, PROMOTION, QUEEN));
          moveList.addMove(Move(pinnedPiece, toSquare, PROMOTION, KNIGHT));
          moveList.addMove(Move(pinnedPiece, toSquare, PROMOTION, ROOK));
          moveList.addMove(Move(pinnedPiece, toSquare, PROMOTION, BISHOP));
        } else {
          moveList.addMove(Move(pinnedPiece, toSquare));
        }
        pawnMoves &= pawnMoves - 1;
      }

    } else {
      switch (piece) {
      case BISHOP:
        pinRay &= attacks::getDiagRays(pinnedPiece);
        break;
      case ROOK:
        // only can be pinned once rook moves so no castling
        pinRay &= attacks::getOrthoRays(pinnedPiece);
        break;
      case QUEEN:
        pinRay &= (attacks::getOrthoRays(pinnedPiece) |
                   attacks::getDiagRays(pinnedPiece));
        break;
      default:
        break;
      }
      while (pinRay) {
        moveList.addMove(Move(pinnedPiece, getLeastSquare(pinRay)));
        pinRay &= pinRay - 1;
      }
    }
  }
}

// generates the attack mask for all squares attacked by pieces
template <Color C> Bitboard generateAttackMask(Board &b) {
  constexpr Direction D = C == WHITE ? NORTH : SOUTH;
  Bitboard mask = 0;
  Bitboard cur = b.getPieces<PAWN>(C);
  mask |= (cur & ~FILE_A) << (D + WEST);
  mask |= (cur & ~FILE_H) << (D + EAST);

  cur = b.getPieces<KNIGHT>(C);
  while (cur) {
    mask |= attacks::getKnightAttacks(getLeastSquare(cur));
    cur &= cur - 1;
  }

  cur = b.getPieces<BISHOP>(C);
  while (cur) {
    mask |= attacks::getSlidingAttacks<BISHOP>(getLeastSquare(cur),
                                               b.getOccupied());
    cur &= cur - 1;
  }

  cur = b.getPieces<ROOK>(C);
  while (cur) {
    mask |=
        attacks::getSlidingAttacks<ROOK>(getLeastSquare(cur), b.getOccupied());
    cur &= cur - 1;
  }

  cur = b.getPieces<QUEEN>(C);
  while (cur) {
    mask |=
        attacks::getSlidingAttacks<QUEEN>(getLeastSquare(cur), b.getOccupied());
    cur &= cur - 1;
  }

  mask |= attacks::getKingAttacks(getLeastSquare(b.getPieces<KING>(C)));
  return mask;
}

template <Color C> void generateMoves(Board &b, MoveList &moveList) {
  constexpr Direction D = C == WHITE ? NORTH : SOUTH;
  moveList.count = 0;
  Square kingSquare = getLeastSquare(b.getPieces<KING>(C));
  Bitboard attackedSquares = generateAttackMask<~C>();
  b.refreshChecksAndPins<C>();
  if (b.getCheckers() == 0) { // no checkers
    generatePinnedMoves<C>(b, moveList);
    Bitboard nonPinned = ~b.getPinned();
    generatePawnMoves<D>(b.getPieces<PAWN>(C) & nonPinned, b.getOccupied(~C),
                         b.getOccupied(), moveList);

  } else if ((b.getCheckers() & (b.getCheckers() - 1)) == 0) { // one checker

  } else { // two checkers
  }
}
} // namespace citterfish