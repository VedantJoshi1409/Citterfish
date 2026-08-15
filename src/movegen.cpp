#include "movegen.h"
#include "attacks.h"
#include "board.h"
#include "types.h"

namespace citterfish {
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
    move_list.add_move(Move(from, to, PROMOTION, QUEEN));
    move_list.add_move(Move(from, to, PROMOTION, KNIGHT));
    move_list.add_move(Move(from, to, PROMOTION, ROOK));
    move_list.add_move(Move(from, to, PROMOTION, BISHOP));
  }

  bb = shift<D + WEST>(pawnsOn7 & ~FILE_A) &
       target; // Generate captures to the left for promotion
  while (bb) {
    Square to = pop_least_square(bb);
    Square from = static_cast<Square>(to - D - WEST);
    move_list.add_move(Move(from, to, PROMOTION, QUEEN));
    move_list.add_move(Move(from, to, PROMOTION, KNIGHT));
    move_list.add_move(Move(from, to, PROMOTION, ROOK));
    move_list.add_move(Move(from, to, PROMOTION, BISHOP));
  }

  bb = shift<D + EAST>(pawnsOn7 & ~FILE_H) &
       target; // Generate captures to the right for promotion
  while (bb) {
    Square to = pop_least_square(bb);
    Square from = static_cast<Square>(to - D - EAST);
    move_list.add_move(Move(from, to, PROMOTION, QUEEN));
    move_list.add_move(Move(from, to, PROMOTION, KNIGHT));
    move_list.add_move(Move(from, to, PROMOTION, ROOK));
    move_list.add_move(Move(from, to, PROMOTION, BISHOP));
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
    Piece piece = piece_type_to_piece(b.get_piece_map(pinned_piece));

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
        if ((pinned_bb & (is_white ? RANK_7 : RANK_2)) != 0) { // promoting
          move_list.add_move(Move(pinned_piece, to, PROMOTION, QUEEN));
          move_list.add_move(Move(pinned_piece, to, PROMOTION, KNIGHT));
          move_list.add_move(Move(pinned_piece, to, PROMOTION, ROOK));
          move_list.add_move(Move(pinned_piece, to, PROMOTION, BISHOP));
        } else {
          move_list.add_move(Move(pinned_piece, to));
        }
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
    mask |= attacks::sliding_attacks<BISHOP>(pop_least_square(cur),
                                             b.get_occupied());
  }

  cur = b.get_pieces<ROOK>(C);
  while (cur) {
    mask |=
        attacks::sliding_attacks<ROOK>(pop_least_square(cur), b.get_occupied());
  }

  cur = b.get_pieces<QUEEN>(C);
  while (cur) {
    mask |= attacks::sliding_attacks<QUEEN>(pop_least_square(cur),
                                            b.get_occupied());
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
    gen_pawn_moves<C>(b.get_pieces<PAWN>(C) & nonPinned, b.get_occupied(~C),
                      ~b.get_occupied(), move_list);
    gen_knight_moves(b.get_pieces<KNIGHT>(C) & nonPinned, b.get_occupied(C),
                     move_list);
    gen_slider_moves<ROOK>(b.get_pieces<ROOK>(C) & nonPinned, b.get_occupied(C),
                           b.get_occupied(), move_list);
    gen_slider_moves<BISHOP>(b.get_pieces<BISHOP>(C) & nonPinned,
                             b.get_occupied(C), b.get_occupied(), move_list);
    gen_slider_moves<QUEEN>(b.get_pieces<QUEEN>(C) & nonPinned,
                            b.get_occupied(C), b.get_occupied(), move_list);
    gen_king_moves(b.get_pieces<KING>(C), b.get_occupied(C) | attackedSquares,
                   move_list);

  } else if ((b.get_checkers() & (b.get_checkers() - 1)) == 0) { // one checker
    Bitboard nonPinned = ~b.get_pinned();
    gen_pawn_moves<C>(b.get_pieces<PAWN>(C) & nonPinned, b.get_occupied(~C),
                      ~b.get_occupied(), move_list);
    gen_knight_moves(b.get_pieces<KNIGHT>(C) & nonPinned, b.get_occupied(C),
                     move_list);
    gen_slider_moves<ROOK>(b.get_pieces<ROOK>(C) & nonPinned, b.get_occupied(C),
                           b.get_occupied(), move_list);
    gen_slider_moves<BISHOP>(b.get_pieces<BISHOP>(C) & nonPinned,
                             b.get_occupied(C), b.get_occupied(), move_list);
    gen_slider_moves<QUEEN>(b.get_pieces<QUEEN>(C) & nonPinned,
                            b.get_occupied(C), b.get_occupied(), move_list);
    gen_king_moves(b.get_pieces<KING>(C), b.get_occupied(C) | attackedSquares,
                   move_list);
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