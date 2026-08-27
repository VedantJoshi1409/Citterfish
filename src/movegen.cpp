#include "movegen.h"
#include "attacks.h"
#include "board.h"
#include "types.h"
#include <cassert>

namespace citterfish {

template <Color C>
void gen_pawn_moves(Bitboard pawns, Bitboard target, Bitboard empty,
                    MoveList &moveList) {
  constexpr Direction D = pawn_dir<C>();
  constexpr Bitboard promoRank = D == NORTH ? RANK_7 : RANK_2;

  Bitboard pawnsBelow7 = pawns & ~promoRank;
  Bitboard pawnsOn7 = pawns & promoRank;

  Bitboard bb = (shift<D>(pawnsBelow7)) & empty; // Generate single pawn push
  Bitboard doublePushBB = shift<D>(bb & (D == NORTH ? RANK_3 : RANK_6)) & empty; //only pawns that can move up one can move up double
  while (bb) {
    Square to = pop_least_square(bb);
    Square from = static_cast<Square>(to - D);
    moveList.add_move(Move(from, to));
  }

  while (doublePushBB) {
    Square to = pop_least_square(doublePushBB);
    Square from = static_cast<Square>(to - 2 * D);
    moveList.add_move(Move(from, to));
  }

  bb = shift<D + WEST>(pawnsBelow7 & ~FILE_A) &
       target; // Generate captures to the left
  while (bb) {
    Square to = pop_least_square(bb);
    Square from = static_cast<Square>(to - D - WEST);
    moveList.add_move(Move(from, to));
  }

  bb = shift<D + EAST>(pawnsBelow7 & ~FILE_H) &
       target; // Generate captures to the right
  while (bb) {
    Square to = pop_least_square(bb);
    Square from = static_cast<Square>(to - D - EAST);
    moveList.add_move(Move(from, to));
  }

  bb = shift<D>(pawnsOn7) & empty; // Generate single pawn push for promotion
  while (bb) {
    Square to = pop_least_square(bb);
    Square from = static_cast<Square>(to - D);
    moveList.add_promo(from, to);
  }

  bb = shift<D + WEST>(pawnsOn7 & ~FILE_A) &
       target; // Generate captures to the left for promotion
  while (bb) {
    Square to = pop_least_square(bb);
    Square from = static_cast<Square>(to - D - WEST);
    moveList.add_promo(from, to);

  }

  bb = shift<D + EAST>(pawnsOn7 & ~FILE_H) &
       target; // Generate captures to the right for promotion
  while (bb) {
    Square to = pop_least_square(bb);
    Square from = static_cast<Square>(to - D - EAST);
    moveList.add_promo(from, to);
  }
}

template <Color C>
void gen_en_passant(Bitboard pawns, Square ep_square, MoveList &moveList) {
  Bitboard attackers = attacks::pawn_attacks<~C>(ep_square) &
                       pawns; // flip color since the enemy getting attacked
  while (attackers) {
    Square from = pop_least_square(attackers);
    moveList.add_move(Move(from, ep_square, ENPASSANT));
  }
}

template <Color C>
void gen_en_passant(Bitboard pawns, Bitboard king, Bitboard orthoAttackers,
                    Bitboard occupied, Square epSquare, MoveList &moveList) {
  assert(epSquare != NO_SQUARE);
  constexpr Direction D = pawn_dir<C>();
  Bitboard attackers = attacks::pawn_attacks<~C>(epSquare) &
                       pawns; // flip color since the enemy getting attacked
  if (attackers == 0) {
    return;
  } else if ((attackers & (attackers - 1)) == 0) { // check edge case
    constexpr Bitboard epRank = C == WHITE ? RANK_5 : RANK_4;
    if ((king & epRank) != 0 && (orthoAttackers & epRank) != 0) {
      occupied ^= attackers | (1ULL << (epSquare - D));
      if ((attacks::sliding_attacks<ROOK>(least_square(king), occupied) &
           (orthoAttackers & epRank)) != 0) {
        // king is on ep rank and without the pawns in the way it would be in
        // check
        return;
      }
    } else {
      Square from = least_square(attackers);
      moveList.add_move(Move(from, epSquare, ENPASSANT));
    }
  } else {
    while (attackers) { // no edge case
      Square from = pop_least_square(attackers);
      moveList.add_move(Move(from, epSquare, ENPASSANT));
    }
  }
}

void gen_knight_moves(Bitboard knights, Bitboard blocks, MoveList &moveList) {
  Bitboard bb = knights;
  while (bb) {
    Square from = pop_least_square(bb);
    Bitboard attacks = attacks::knight_attacks(from) & ~blocks;
    while (attacks) {
      Square to = pop_least_square(attacks);
      moveList.add_move(Move(from, to));
    }
  }
}

template <Piece P>
void gen_slider_moves(Bitboard pieces, Bitboard blocks, Bitboard occupied,
                      MoveList &moveList) {
  while (pieces) {
    Square cur = pop_least_square(pieces);
    Bitboard attacks = attacks::sliding_attacks<P>(cur, occupied) & ~blocks;
    while (attacks) {
      Square to = pop_least_square(attacks);
      moveList.add_move(Move(cur, to));
    }
  }
}

void gen_king_moves(Bitboard king, Bitboard blocks, MoveList &moveList) {
  Bitboard bb = king;
  Square from = least_square(bb);
  Bitboard attacks = attacks::king_attacks(from) & ~blocks;
  while (attacks) {
    Square to = pop_least_square(attacks);
    moveList.add_move(Move(from, to));
  }
}

template <Color C>
void gen_castle_moves(Bitboard enemyAttacks, Bitboard occupied,
                      uint8_t castleRights, MoveList &moveList) {
  if constexpr (C == WHITE) {
    if ((castleRights & WHITE_KINGSIDE) != 0 &&
        (occupied & WHITE_KINGSIDE_EMPTY) == 0 &&
        (enemyAttacks & WHITE_KINGSIDE_ATTACKED) == 0) {
      moveList.add_move(Move(e1, g1, CASTLE));
    }
    if ((castleRights & WHITE_QUEENSIDE) != 0 &&
        (occupied & WHITE_QUEENSIDE_EMPTY) == 0 &&
        (enemyAttacks & WHITE_QUEENSIDE_ATTACKED) == 0) {
      moveList.add_move(Move(e1, c1, CASTLE));
    }
  } else {
    if ((castleRights & BLACK_KINGSIDE) != 0 &&
        (occupied & BLACK_KINGSIDE_EMPTY) == 0 &&
        (enemyAttacks & BLACK_KINGSIDE_ATTACKED) == 0) {
      moveList.add_move(Move(e8, g8, CASTLE));
    }
    if ((castleRights & BLACK_QUEENSIDE) != 0 &&
        (occupied & BLACK_QUEENSIDE_EMPTY) == 0 &&
        (enemyAttacks & BLACK_QUEENSIDE_ATTACKED) == 0) {
      moveList.add_move(Move(e8, c8, CASTLE));
    }
  }
}

template <Color C> void gen_pinned_moves(Board &b, MoveList &moveList) {
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
      if (((1ULL << b.en_passant_square()) & pin_ray) != 0) { // enPassantable
        gen_en_passant<C>(pinned_bb, b.en_passant_square(), moveList);
      }

      Bitboard pawn_moves = attacks::pawn_attacks<C>(pinned_bb) & attacker;
      pawn_moves |= shift<D>(pinned_bb);
      if ((pinned_bb & (is_white ? RANK_2 : RANK_7)) != 0)
        pawn_moves |= shift<static_cast<Direction>(2 * D)>(pinned_bb);

      pawn_moves &= pin_ray; // only keep moves along pin ray
      while (pawn_moves) {
        Square to = pop_least_square(pawn_moves);
        moveList.add_move(Move(pinned_piece, to));
        // no way to promo a pinned pawn
      }

    } else {
      switch (piece) {
      case KNIGHT:
        pin_ray = 0;
        break;
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
        moveList.add_move(Move(pinned_piece, pop_least_square(pin_ray)));
      }
    }
  }
}

template <Color C> void gen_moves(Board &b, MoveList &moveList) {
  moveList.count = 0;
  Square kingSquare = least_square(b.get_pieces<KING>(C));
  Bitboard attackedSquares = b.attack_mask(~C);
  b.checks_pins();
  if (b.get_checkers() == 0) { // no checkers
    gen_pinned_moves<C>(b, moveList);
    Bitboard nonPinned = ~b.get_pinned();
    gen_pawn_moves<C>(b.get_pieces<PAWN>(C) & nonPinned, b.get_pieces(~C),
                      ~b.get_pieces(), moveList);
    if (b.en_passant_square() != NO_SQUARE) {
      gen_en_passant<C>(b.get_pieces<PAWN>(C) & nonPinned, b.get_pieces<KING>(C),
                      b.get_pieces<ROOK>(~C) | b.get_pieces<QUEEN>(~C),
                      b.get_pieces(), b.en_passant_square(), moveList);
    }
    gen_knight_moves(b.get_pieces<KNIGHT>(C) & nonPinned, b.get_pieces(C),
                     moveList);
    gen_slider_moves<ROOK>(b.get_pieces<ROOK>(C) & nonPinned, b.get_pieces(C),
                           b.get_pieces(), moveList);
    gen_slider_moves<BISHOP>(b.get_pieces<BISHOP>(C) & nonPinned,
                             b.get_pieces(C), b.get_pieces(), moveList);
    gen_slider_moves<QUEEN>(b.get_pieces<QUEEN>(C) & nonPinned, b.get_pieces(C),
                            b.get_pieces(), moveList);
    gen_king_moves(b.get_pieces<KING>(C), b.get_pieces(C) | attackedSquares,
                   moveList);
    gen_castle_moves<C>(attackedSquares, b.get_pieces(C), b.castling_rights(),
                        moveList);

  } else if ((b.get_checkers() & (b.get_checkers() - 1)) == 0) { // one checker
    Square checker = static_cast<Square>(std::countr_zero(b.get_checkers()));
    Bitboard legal =
        attacks::from_to_bb(checker, static_cast<Square>(std::countr_zero(
                                         b.get_pieces<KING>(C)))) |
        b.get_checkers();

    Bitboard nonPinned = ~b.get_pinned();
    gen_pawn_moves<C>(
        b.get_pieces<PAWN>(C) & nonPinned, b.get_checkers(), legal & ~b.get_checkers(),
        moveList); // Only captures are on legal squares and only empty squares
                   // are the legal ones minus the checker
    constexpr Direction D = C == WHITE ? NORTH : SOUTH;
    if (b.en_passant_square() - D ==
        checker) { // can only en passant in check if the checker is the double
                   // pawn push
      gen_en_passant<C>(b.get_pieces<PAWN>(C) & nonPinned,
                        b.get_pieces<KING>(C),
                        b.get_pieces<ROOK>(~C) | b.get_pieces<QUEEN>(~C),
                        b.get_pieces(), b.en_passant_square(), moveList);
    }
    gen_knight_moves(b.get_pieces<KNIGHT>(C) & nonPinned, ~legal,
                     moveList); // knight is blocked by any non legal square
    gen_slider_moves<ROOK>(
        b.get_pieces<ROOK>(C) & nonPinned, ~legal, b.get_pieces(),
        moveList); // sliders are blocked by any non legal sqaure
    gen_slider_moves<BISHOP>(b.get_pieces<BISHOP>(C) & nonPinned, ~legal,
                             b.get_pieces(), moveList);
    gen_slider_moves<QUEEN>(b.get_pieces<QUEEN>(C) & nonPinned, ~legal,
                            b.get_pieces(), moveList);
    gen_king_moves(b.get_pieces<KING>(C),
                   b.get_pieces(C) | attackedSquares, moveList);
  } else { // two checkers
    gen_king_moves(b.get_pieces<KING>(C), b.get_pieces(C) | attackedSquares,
                   moveList);
  }
}

void gen_moves(Board &b, MoveList &move_list) {
  if (b.is_white()) {
    return gen_moves<WHITE>(b, move_list);
  } else {
    return gen_moves<BLACK>(b, move_list);
  }
}
} // namespace citterfish