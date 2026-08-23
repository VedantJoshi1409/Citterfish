#include "movegen.h"
#include "attacks.h"
#include "board.h"
#include "types.h"

namespace citterfish {

void gen_promo(Square from, Square to, MoveList &move_list) {
  move_list.add_move(Move(from, to, PROMOTION, QUEEN));
  move_list.add_move(Move(from, to, PROMOTION, KNIGHT));
  move_list.add_move(Move(from, to, PROMOTION, ROOK));
  move_list.add_move(Move(from, to, PROMOTION, BISHOP));
}

template <Color C>
void gen_pawn_moves(Bitboard pawns, Bitboard target, Bitboard empty,
                    MoveList &move_list) {
  constexpr Direction D = pawn_dir<C>();
  constexpr Bitboard promo_rank = D == NORTH ? RANK_7 : RANK_2;

  Bitboard pawnsBelow7 = pawns & ~promo_rank;
  Bitboard pawnsOn7 = pawns & promo_rank;

  Bitboard bb = (shift<D>(pawnsBelow7)) & empty; // Generate single pawn push
  while (bb) {
    Square to = pop_least_square(bb);
    Square from = static_cast<Square>(to - D);
    move_list.add_move(Move(from, to));
  }

  bb = shift<static_cast<Direction>(2 * D)>(pawnsBelow7 &
                                            (D == NORTH ? RANK_2 : RANK_7)) &
       empty; // Generate double pawn push
  while (bb) {
    Square to = pop_least_square(bb);
    Square from = static_cast<Square>(to - 2 * D);
    move_list.add_move(Move(from, to));
  }

  bb = shift<D + WEST>(pawnsBelow7 & ~FILE_A) &
       target; // Generate captures to the left
  while (bb) {
    Square to = pop_least_square(bb);
    Square from = static_cast<Square>(to - D - WEST);
    move_list.add_move(Move(from, to));
  }

  bb = shift<D + EAST>(pawnsBelow7 & ~FILE_H) &
       target; // Generate captures to the right
  while (bb) {
    Square to = pop_least_square(bb);
    Square from = static_cast<Square>(to - D - EAST);
    move_list.add_move(Move(from, to));
  }

  bb = shift<D>(pawnsOn7) & empty; // Generate single pawn push for promotion
  while (bb) {
    Square to = pop_least_square(bb);
    Square from = static_cast<Square>(to - D);
    gen_promo(from, to, move_list);
  }

  bb = shift<D + WEST>(pawnsOn7 & ~FILE_A) &
       target; // Generate captures to the left for promotion
  while (bb) {
    Square to = pop_least_square(bb);
    Square from = static_cast<Square>(to - D - WEST);
    gen_promo(from, to, move_list);
  }

  bb = shift<D + EAST>(pawnsOn7 & ~FILE_H) &
       target; // Generate captures to the right for promotion
  while (bb) {
    Square to = pop_least_square(bb);
    Square from = static_cast<Square>(to - D - EAST);
    gen_promo(from, to, move_list);
  }
}

template <Color C>
void gen_en_passant(Bitboard pawns, Square ep_square, MoveList &move_list) {
  Bitboard attackers = attacks::pawn_attackers<~C>(ep_square) &
                       pawns; // flip color since the enemy getting attacked
  while (attackers) {
    Square from = pop_least_square(attackers);
    move_list.add_move(Move(from, ep_square, ENPASSANT));
  }
}

template <Color C>
void gen_en_passant(Bitboard pawns, Bitboard king, Bitboard orthoAttackers,
                    Bitboard occupied, Square ep_square, MoveList &move_list) {
  constexpr Direction D = pawn_dir<C>();
  Bitboard attackers = attacks::pawn_attackers<~C>(ep_square) &
                       pawns; // flip color since the enemy getting attacked
  if ((attackers & (attackers - 1)) == 0) { // check edge case
    constexpr Bitboard epRank = C == WHITE ? RANK_5 : RANK_4;
    if ((king & epRank) != 0 && (orthoAttackers & epRank) != 0) {
      occupied ^= attackers | (1ULL << (ep_square - D));
      if ((attacks::sliding_attacks<ROOK>(least_square(king), occupied) &
           (orthoAttackers & epRank)) != 0) {
        // king is on ep rank and without the pawns in the way it would be in
        // check
        return;
      }
    } else {
      Square from = least_square(attackers);
      move_list.add_move(Move(from, ep_square, ENPASSANT));
    }
  } else {
    while (attackers) { // no edge case
      Square from = pop_least_square(attackers);
      move_list.add_move(Move(from, ep_square, ENPASSANT));
    }
  }
}

void gen_knight_moves(Bitboard knights, Bitboard blocks, MoveList &move_list) {
  Bitboard bb = knights;
  while (bb) {
    Square from = pop_least_square(bb);
    Bitboard attacks = attacks::knight_attacks(from) & ~blocks;
    while (attacks) {
      Square to = pop_least_square(attacks);
      move_list.add_move(Move(from, to));
    }
  }
}

template <Piece P>
void gen_slider_moves(Bitboard pieces, Bitboard blocks, Bitboard occupied,
                      MoveList &move_list) {
  while (pieces) {
    Square cur = pop_least_square(pieces);
    Bitboard attacks = attacks::sliding_attacks<P>(cur, occupied) & ~blocks;
    while (attacks) {
      Square to = pop_least_square(attacks);
      move_list.add_move(Move(cur, to));
    }
  }
}

void gen_king_moves(Bitboard king, Bitboard blocks, MoveList &move_list) {
  Bitboard bb = king;
  Square from = least_square(bb);
  Bitboard attacks = attacks::king_attacks(from) & ~blocks;
  while (attacks) {
    Square to = pop_least_square(attacks);
    move_list.add_move(Move(from, to));
  }
}

template <Color C>
void gen_castle_moves(Bitboard enemy_attacks, Bitboard occupied,
                      uint8_t castleRights, MoveList &moveList) {
  if constexpr (C == WHITE) {
    if ((castleRights & WHITE_KINGSIDE) != 0 &&
        (occupied & WHITE_KINGSIDE_EMPTY) == 0 &&
        (enemy_attacks & WHITE_KINGSIDE_ATTACKED) == 0) {
      moveList.add_move(Move(e1, g1, CASTLE));
    }
    if ((castleRights & WHITE_QUEENSIDE) != 0 &&
        (occupied & WHITE_QUEENSIDE_EMPTY) == 0 &&
        (enemy_attacks & WHITE_QUEENSIDE_ATTACKED) == 0) {
      moveList.add_move(Move(e1, c1, CASTLE));
    }
  } else {
    if ((castleRights & BLACK_KINGSIDE) != 0 &&
        (occupied & BLACK_KINGSIDE_EMPTY) == 0 &&
        (enemy_attacks & BLACK_KINGSIDE_ATTACKED) == 0) {
      moveList.add_move(Move(e8, g8, CASTLE));
    }
    if ((castleRights & BLACK_QUEENSIDE) != 0 &&
        (occupied & BLACK_QUEENSIDE_EMPTY) == 0 &&
        (enemy_attacks & BLACK_QUEENSIDE_ATTACKED) == 0) {
      moveList.add_move(Move(e8, c8, CASTLE));
    }
  }
}

template <Color C> void gen_pinned_moves(Board &b, MoveList &move_list) {
  constexpr Direction D = (C == WHITE) ? NORTH : SOUTH;
  constexpr bool is_white = C == WHITE;
  Bitboard pinners = b.get_pinners();
  Square king_square = least_square(b.get_pieces<KING>(C));

  while (pinners) { // get all pinned piece moves
    Square attacker = pop_least_square(pinners);
    Bitboard pin_ray =
        attacks::from_to_bb(king_square, attacker) | 1ULL << attacker;
    Bitboard pinned_bb = pin_ray & b.get_pinned();
    Square pinned_piece = least_square(pinned_bb);
    Piece piece = piece_type_to_piece(b.piece_on(pinned_piece));

    if (piece == PAWN) {
      if (((1ULL << b.get_en_passant_square()) & pin_ray) !=
          0) { // enPassantable
        gen_en_passant<C>(pinned_bb, b.get_en_passant_square(), move_list);
      }

      Bitboard pawn_moves = attacks::pawn_attacks<C>(pinned_bb) & attacker;
      pawn_moves |= shift<D>(pinned_bb);
      if ((pinned_bb & (is_white ? RANK_2 : RANK_7)) != 0)
        pawn_moves |= shift<static_cast<Direction>(2 * D)>(pinned_bb);

      pawn_moves &= pin_ray; // only keep moves along pin ray
      while (pawn_moves) {
        Square to = pop_least_square(pawn_moves);
        move_list.add_move(Move(pinned_piece, to));
        // no way to promo a pinned pawn
      }

    } else {
      switch (piece) {
      case BISHOP:
        pin_ray &= attacks::diag_rays(pinned_piece);
        break;
      case ROOK:
        // only can be pinned once rook moves so no castling
        pin_ray &= attacks::ortho_rays(pinned_piece);
        break;
      case QUEEN:
        pin_ray &= (attacks::ortho_rays(pinned_piece) |
                    attacks::diag_rays(pinned_piece));
        break;
      default:
        break;
      }
      while (pin_ray) {
        move_list.add_move(Move(pinned_piece, pop_least_square(pin_ray)));
      }
    }
  }
}

// generates the attack mask for all squares attacked by pieces
template <Color C> Bitboard gen_attack_mask(Board &b) {
  constexpr Direction D = C == WHITE ? NORTH : SOUTH;
  Bitboard mask = 0;
  mask |= attacks::pawn_attacks<C>(b.get_pieces<PAWN>(C));

  Bitboard cur = b.get_pieces<KNIGHT>(C);
  while (cur) {
    mask |= attacks::knight_attacks(pop_least_square(cur));
  }

  cur = b.get_pieces<BISHOP>(C);
  while (cur) {
    mask |=
        attacks::sliding_attacks<BISHOP>(pop_least_square(cur), b.get_pieces());
  }

  cur = b.get_pieces<ROOK>(C);
  while (cur) {
    mask |=
        attacks::sliding_attacks<ROOK>(pop_least_square(cur), b.get_pieces());
  }

  cur = b.get_pieces<QUEEN>(C);
  while (cur) {
    mask |=
        attacks::sliding_attacks<QUEEN>(pop_least_square(cur), b.get_pieces());
  }

  mask |= attacks::king_attacks(least_square(b.get_pieces<KING>(C)));
  return mask;
}

template <Color C> void gen_moves(Board &b, MoveList &move_list) {
  move_list.count = 0;
  Square king_square = least_square(b.get_pieces<KING>(C));
  Bitboard attackedSquares = gen_attack_mask<~C>(b);
  b.refresh_checks_pins<C>();
  if (b.get_checkers() == 0) { // no checkers
    gen_pinned_moves<C>(b, move_list);
    Bitboard nonPinned = ~b.get_pinned();
    gen_pawn_moves<C>(b.get_pieces<PAWN>(C) & nonPinned, b.get_pieces(~C),
                      ~b.get_pieces(), move_list);
    gen_en_passant<C>(b.get_pieces<PAWN>(C) & nonPinned, b.get_pieces<KING>(C),
                      b.get_pieces<ROOK>(~C) | b.get_pieces<QUEEN>(~C),
                      b.get_pieces(), b.get_en_passant_square(), move_list);
    gen_knight_moves(b.get_pieces<KNIGHT>(C) & nonPinned, b.get_pieces(C),
                     move_list);
    gen_slider_moves<ROOK>(b.get_pieces<ROOK>(C) & nonPinned, b.get_pieces(C),
                           b.get_pieces(), move_list);
    gen_slider_moves<BISHOP>(b.get_pieces<BISHOP>(C) & nonPinned,
                             b.get_pieces(C), b.get_pieces(), move_list);
    gen_slider_moves<QUEEN>(b.get_pieces<QUEEN>(C) & nonPinned, b.get_pieces(C),
                            b.get_pieces(), move_list);
    gen_king_moves(b.get_pieces<KING>(C), b.get_pieces(C) | attackedSquares,
                   move_list);
    gen_castle_moves<C>(attackedSquares, b.get_pieces(C),
                        b.get_castling_rights(), move_list);

  } else if ((b.get_checkers() & (b.get_checkers() - 1)) == 0) { // one checker
    Square checker = static_cast<Square>(std::countr_zero(b.get_checkers()));
    Bitboard legal =
        attacks::from_to_bb(checker, static_cast<Square>(std::countr_zero(
                                         b.get_pieces<KING>(C)))) |
        b.get_checkers();

    Bitboard nonPinned = ~b.get_pinned();
    gen_pawn_moves<C>(
        b.get_pieces<PAWN>(C) & nonPinned, legal, legal & ~b.get_checkers(),
        move_list); // Only captures are on legal squares and only empty squares
                    // are the legal ones minus the checker
    constexpr Direction D = C == WHITE ? NORTH : SOUTH;
    if (b.get_en_passant_square() - D ==
        checker) { // can only en passant in check if the checker is the double
                   // pawn push
      gen_en_passant<C>(b.get_pieces<PAWN>(C) & nonPinned,
                        b.get_pieces<KING>(C),
                        b.get_pieces<ROOK>(~C) | b.get_pieces<QUEEN>(~C),
                        b.get_pieces(), b.get_en_passant_square(), move_list);
    }
    gen_knight_moves(b.get_pieces<KNIGHT>(C) & nonPinned, ~legal,
                     move_list); // knight is blocked by any non legal square
    gen_slider_moves<ROOK>(
        b.get_pieces<ROOK>(C) & nonPinned, ~legal, b.get_pieces(),
        move_list); // sliders are blocked by any non legal sqaure
    gen_slider_moves<BISHOP>(b.get_pieces<BISHOP>(C) & nonPinned, ~legal,
                             b.get_pieces(), move_list);
    gen_slider_moves<QUEEN>(b.get_pieces<QUEEN>(C) & nonPinned, ~legal,
                            b.get_pieces(), move_list);
    gen_king_moves(b.get_pieces<KING>(C),
                   b.get_pieces(C) | attackedSquares | ~legal, move_list);
  } else { // two checkers
    gen_king_moves(b.get_pieces<KING>(C), b.get_pieces(C) | attackedSquares,
                   move_list);
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