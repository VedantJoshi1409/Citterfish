#include "movegen.h"
#include "attacks.h"
#include "board.h"
#include "types.h"

namespace citterfish {
template <Color C>
void generatePawnMoves(Bitboard pawns, Bitboard enemyOccupied,
                       Bitboard occupied, MoveList &moveList) {
  constexpr Direction D = C == WHITE ? NORTH : SOUTH;
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

template <Color C>
void generateEnPassant(Bitboard pawns, Square enPassantSquare, MoveList &moveList) {
  Bitboard attackers = attacks::pawn_attackers<~C>(enPassantSquare) & pawns; //flip color since the enemy getting attacked
  while (attackers) {
    Square from = getLeastSquare(attackers);
    moveList.addMove(Move(from, enPassantSquare, ENPASSANT));
    attackers &= attackers-1;
  }
}

void generateKnightMoves(Bitboard knights, Bitboard friendlyOccupied,
                         MoveList &moveList) {
  Bitboard bb = knights;
  while (bb) {
    Square from = getLeastSquare(bb);
    Bitboard attacks = attacks::knight_attacks(from) & ~friendlyOccupied;
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
  Bitboard attacks = attacks::king_attacks(from) & ~friendlyOccupied;
  while (attacks) {
    Square to = getLeastSquare(attacks);
    moveList.addMove(Move(from, to));
    attacks &= attacks - 1; // clear lsb
  }
}

template <Color C> void generatePinnedMoves(Board &b, MoveList &moveList) {
  constexpr Direction D = (C == WHITE) ? NORTH : SOUTH;
  constexpr bool isWhite = C == WHITE;
  Bitboard pinners = b.get_pinners();
  Square kingSquare = getLeastSquare(b.get_pieces<KING>(C));
  while (pinners) { // get all pinned piece moves
    Square attacker = getLeastSquare(pinners);
    Bitboard pinRay =
        attacks::from_to_bb(kingSquare, attacker) | 1ULL << attacker;
    Square pinnedPiece = getLeastSquare(pinRay & b.get_pinned());

    Piece piece = pieceTypeToPiece(b.get_piece_map(pinnedPiece));
    if (piece == PAWN) {
      
      Bitboard pawnBB = 1ULL << pinnedPiece;
      if (((1ULL << b.get_en_passant_square()) & pinRay) != 0) { //enPassantable
        generateEnPassant<C>(pawnBB, b.get_en_passant_square(), moveList);
      }
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
        pinRay &= attacks::diag_rays(pinnedPiece);
        break;
      case ROOK:
        // only can be pinned once rook moves so no castling
        pinRay &= attacks::ortho_rays(pinnedPiece);
        break;
      case QUEEN:
        pinRay &= (attacks::ortho_rays(pinnedPiece) |
                   attacks::diag_rays(pinnedPiece));
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
  Bitboard cur = b.get_pieces<PAWN>(C);
  mask |= (cur & ~FILE_A) << (D + WEST);
  mask |= (cur & ~FILE_H) << (D + EAST);

  cur = b.get_pieces<KNIGHT>(C);
  while (cur) {
    mask |= attacks::knight_attacks(getLeastSquare(cur));
    cur &= cur - 1;
  }

  cur = b.get_pieces<BISHOP>(C);
  while (cur) {
    mask |= attacks::sliding_attacks<BISHOP>(getLeastSquare(cur),
                                               b.get_occupied());
    cur &= cur - 1;
  }

  cur = b.get_pieces<ROOK>(C);
  while (cur) {
    mask |=
        attacks::sliding_attacks<ROOK>(getLeastSquare(cur), b.get_occupied());
    cur &= cur - 1;
  }

  cur = b.get_pieces<QUEEN>(C);
  while (cur) {
    mask |=
        attacks::sliding_attacks<QUEEN>(getLeastSquare(cur), b.get_occupied());
    cur &= cur - 1;
  }

  mask |= attacks::king_attacks(getLeastSquare(b.get_pieces<KING>(C)));
  return mask;
}

template <Color C> void generateMoves(Board &b, MoveList &moveList) {
  moveList.count = 0;
  Square kingSquare = getLeastSquare(b.get_pieces<KING>(C));
  Bitboard attackedSquares = generateAttackMask<~C>(b);
  b.refresh_checks_pins<C>();
  if (b.get_checkers() == 0) { // no checkers
    generatePinnedMoves<C>(b, moveList);
    Bitboard nonPinned = ~b.get_pinned();
    generatePawnMoves<C>(b.get_pieces<PAWN>(C) & nonPinned, b.get_occupied(~C),
                         b.get_occupied(), moveList);

  } else if ((b.get_checkers() & (b.get_checkers() - 1)) == 0) { // one checker

  } else { // two checkers
  }
}

void generateMoves(Board &b, MoveList &moveList) {
  if (b.get_is_white()) {
  return generateMoves<WHITE>(b, moveList);
  } else {
  return generateMoves<BLACK>(b, moveList);
  }
}
} // namespace citterfish