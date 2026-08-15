#include "movegen.h"
#include "attacks.h"
#include "board.h"
#include "types.h"

namespace citterfish {
template <Color C>
void gen_pawn_moves(Bitboard pawns, Bitboard target, Bitboard occupied,
                    MoveList &move_list) {
  constexpr Direction D = C == WHITE ? NORTH : SOUTH;
  constexpr Bitboard filesOn7 = D == NORTH ? RANK_7 : RANK_2;
  Bitboard pawnsBelow7 = pawns & ~filesOn7;
  Bitboard pawnsOn7 = pawns & filesOn7;
  Bitboard bb = (pawnsBelow7 << D) & ~occupied; // Generate single pawn push
  while (bb) {
    Square to = get_least_square(bb);
    Square from = static_cast<Square>(to - D);
    move_list.add_move(Move(from, to));
    bb &= bb - 1; // clear lsb
  }

  bb = (pawnsBelow7 & (D == NORTH ? RANK_2 : RANK_7)) << (2 * D) &
       ~occupied; // Generate double pawn push
  while (bb) {
    Square to = get_least_square(bb);
    Square from = static_cast<Square>(to - 2 * D);
    move_list.add_move(Move(from, to));
    bb &= bb - 1; // clear lsb
  }

  bb = (pawnsBelow7 & ~FILE_A) << (D + WEST) &
       target; // Generate captures to the left
  while (bb) {
    Square to = get_least_square(bb);
    Square from = static_cast<Square>(to - D - WEST);
    move_list.add_move(Move(from, to));
    bb &= bb - 1; // clear lsb
  }

  bb = (pawnsBelow7 & ~FILE_H) << (D + EAST) &
       target; // Generate captures to the right
  while (bb) {
    Square to = get_least_square(bb);
    Square from = static_cast<Square>(to - D - EAST);
    move_list.add_move(Move(from, to));
    bb &= bb - 1; // clear lsb
  }

  bb = pawnsOn7 << D & ~occupied; // Generate single pawn push for promotion
  while (bb) {
    Square to = get_least_square(bb);
    Square from = static_cast<Square>(to - D);
    move_list.add_move(Move(from, to, PROMOTION, QUEEN));
    move_list.add_move(Move(from, to, PROMOTION, KNIGHT));
    move_list.add_move(Move(from, to, PROMOTION, ROOK));
    move_list.add_move(Move(from, to, PROMOTION, BISHOP));
    bb &= bb - 1; // clear lsb
  }

  bb = (pawnsOn7 & ~FILE_A) << (D + WEST) &
       target; // Generate captures to the left for promotion
  while (bb) {
    Square to = get_least_square(bb);
    Square from = static_cast<Square>(to - D - WEST);
    move_list.add_move(Move(from, to, PROMOTION, QUEEN));
    move_list.add_move(Move(from, to, PROMOTION, KNIGHT));
    move_list.add_move(Move(from, to, PROMOTION, ROOK));
    move_list.add_move(Move(from, to, PROMOTION, BISHOP));
    bb &= bb - 1; // clear lsb
  }

  bb = (pawnsOn7 & ~FILE_H) << (D + EAST) &
       target; // Generate captures to the right for promotion
  while (bb) {
    Square to = get_least_square(bb);
    Square from = static_cast<Square>(to - D - EAST);
    move_list.add_move(Move(from, to, PROMOTION, QUEEN));
    move_list.add_move(Move(from, to, PROMOTION, KNIGHT));
    move_list.add_move(Move(from, to, PROMOTION, ROOK));
    move_list.add_move(Move(from, to, PROMOTION, BISHOP));
    bb &= bb - 1; // clear lsb
  }
}

template <Color C>
void gen_en_passant(Bitboard pawns, Square ep_square, MoveList &move_list) {
  Bitboard attackers = attacks::pawn_attackers<~C>(ep_square) &
                       pawns; // flip color since the enemy getting attacked
  while (attackers) {
    Square from = get_least_square(attackers);
    move_list.add_move(Move(from, ep_square, ENPASSANT));
    attackers &= attackers - 1;
  }
}

void gen_knight_moves(Bitboard knights, Bitboard blocks, MoveList &move_list) {
  Bitboard bb = knights;
  while (bb) {
    Square from = get_least_square(bb);
    Bitboard attacks = attacks::knight_attacks(from) & ~blocks;
    while (attacks) {
      Square to = get_least_square(attacks);
      move_list.add_move(Move(from, to));
      attacks &= attacks - 1; // clear lsb
    }
    bb &= bb - 1; // clear lsb
  }
}

template <Piece P>
void gen_slider_moves(Bitboard pieces, Bitboard blocks, Bitboard occupied,
                      MoveList &move_list) {
  while (pieces) {
    Square cur = get_least_square(pieces);
  }
}

void gen_king_moves(Bitboard king, Bitboard blocks, MoveList &move_list) {
  Bitboard bb = king;
  Square from = get_least_square(bb);
  Bitboard attacks = attacks::king_attacks(from) & ~blocks;
  while (attacks) {
    Square to = get_least_square(attacks);
    move_list.add_move(Move(from, to));
    attacks &= attacks - 1; // clear lsb
  }
}

template <Color C> void gen_pinned_moves(Board &b, MoveList &move_list) {
  constexpr Direction D = (C == WHITE) ? NORTH : SOUTH;
  constexpr bool isWhite = C == WHITE;
  Bitboard pinners = b.get_pinners();
  Square kingSquare = get_least_square(b.get_pieces<KING>(C));
  while (pinners) { // get all pinned piece moves
    Square attacker = get_least_square(pinners);
    Bitboard pinRay =
        attacks::from_to_bb(kingSquare, attacker) | 1ULL << attacker;
    Square pinnedPiece = get_least_square(pinRay & b.get_pinned());

    Piece piece = piece_type_to_piece(b.get_piece_map(pinnedPiece));
    if (piece == PAWN) {

      Bitboard pawnBB = 1ULL << pinnedPiece;
      if (((1ULL << b.get_en_passant_square()) & pinRay) !=
          0) { // enPassantable
        gen_en_passant<C>(pawnBB, b.get_en_passant_square(), move_list);
      }
      Bitboard pawnMoves = ((pawnBB << (D + EAST)) | (pawnBB << (D + WEST))) &
                           attacker; // attacking squares
      pawnMoves |= pawnBB << D;
      if ((pawnBB & (isWhite ? RANK_2 : RANK_7)) != 0)
        pawnMoves |= pawnBB << 2 * D;
      pawnMoves &= pinRay; // only keep moves along pin ray
      while (pawnMoves) {
        Square toSquare = get_least_square(pawnMoves);
        if ((pawnBB & (isWhite ? RANK_7 : RANK_2)) != 0) { // promoting
          move_list.add_move(Move(pinnedPiece, toSquare, PROMOTION, QUEEN));
          move_list.add_move(Move(pinnedPiece, toSquare, PROMOTION, KNIGHT));
          move_list.add_move(Move(pinnedPiece, toSquare, PROMOTION, ROOK));
          move_list.add_move(Move(pinnedPiece, toSquare, PROMOTION, BISHOP));
        } else {
          move_list.add_move(Move(pinnedPiece, toSquare));
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
        move_list.add_move(Move(pinnedPiece, get_least_square(pinRay)));
        pinRay &= pinRay - 1;
      }
    }
  }
}

// generates the attack mask for all squares attacked by pieces
template <Color C> Bitboard gen_attack_mask(Board &b) {
  constexpr Direction D = C == WHITE ? NORTH : SOUTH;
  Bitboard mask = 0;
  Bitboard cur = b.get_pieces<PAWN>(C);
  mask |= (cur & ~FILE_A) << (D + WEST);
  mask |= (cur & ~FILE_H) << (D + EAST);

  cur = b.get_pieces<KNIGHT>(C);
  while (cur) {
    mask |= attacks::knight_attacks(get_least_square(cur));
    cur &= cur - 1;
  }

  cur = b.get_pieces<BISHOP>(C);
  while (cur) {
    mask |=
        attacks::sliding_attacks<BISHOP>(get_least_square(cur), b.get_occupied());
    cur &= cur - 1;
  }

  cur = b.get_pieces<ROOK>(C);
  while (cur) {
    mask |=
        attacks::sliding_attacks<ROOK>(get_least_square(cur), b.get_occupied());
    cur &= cur - 1;
  }

  cur = b.get_pieces<QUEEN>(C);
  while (cur) {
    mask |=
        attacks::sliding_attacks<QUEEN>(get_least_square(cur), b.get_occupied());
    cur &= cur - 1;
  }

  mask |= attacks::king_attacks(get_least_square(b.get_pieces<KING>(C)));
  return mask;
}

template <Color C> void gen_moves(Board &b, MoveList &move_list) {
  move_list.count = 0;
  Square kingSquare = get_least_square(b.get_pieces<KING>(C));
  Bitboard attackedSquares = gen_attack_mask<~C>(b);
  b.refresh_checks_pins<C>();
  if (b.get_checkers() == 0) { // no checkers
    gen_pinned_moves<C>(b, move_list);
    Bitboard nonPinned = ~b.get_pinned();
    gen_pawn_moves<C>(b.get_pieces<PAWN>(C) & nonPinned, b.get_occupied(~C),
                      b.get_occupied(), move_list);

  } else if ((b.get_checkers() & (b.get_checkers() - 1)) == 0) { // one checker

  } else { // two checkers
  }
}

void gen_moves(Board &b, MoveList &move_list) {
  if (b.get_is_white()) {
    return gen_moves<WHITE>(b, move_list);
  } else {
    return gen_moves<BLACK>(b, move_list);
  }
}
} // namespace citterfish