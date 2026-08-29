#include "movegen.h"
#include "attacks.h"
#include "board.h"
#include "types.h"
#include <cassert>

namespace citterfish {

template <Direction D> void splat_pawn_moves(Bitboard bb, MoveList &moveList) {
  while (bb) {
    Square to = pop_least_square(bb);
    moveList.add_move(Move(to - D, to));
  }
}

template <Direction D> void splat_promo_moves(Bitboard bb, MoveList &moveList) {
  while (bb) {
    Square to = pop_least_square(bb);
    moveList.add_promo(to - D, to);
  }
}

void splat_moves(Square from, Bitboard bb, MoveList &moveList) {
  while (bb) {
    Square to = pop_least_square(bb);
    moveList.add_move(Move(from, to));
  }
}

template <Direction D>
void gen_pawn_push(Bitboard pawns, Bitboard empty, Bitboard legal,
                   MoveList &moveList) {
  constexpr Bitboard promoRank = D == NORTH ? RANK_8 : RANK_1;
  Bitboard bb = (shift<D>(pawns)) & empty; // Generate single pawn push
  Bitboard doublePushBB =
      shift<D>(bb & (D == NORTH ? RANK_3 : RANK_6)) & empty &
      legal; // only pawns that can move up one can move up double
  bb &= legal;

  Bitboard promo = bb & promoRank;
  bb &= ~promoRank;
  splat_pawn_moves<D>(bb, moveList);
  splat_promo_moves<D>(promo, moveList);
  splat_pawn_moves<static_cast<Direction>(2 * D)>(doublePushBB, moveList);
}

template <Direction D>
void gen_pawn_capture(Bitboard pawns, Bitboard legalTargets,
                      MoveList &moveList) {
  constexpr Bitboard promoRank = D == NORTH ? RANK_8 : RANK_1;
  Bitboard bb = shift<D + WEST>(pawns & ~FILE_A) & legalTargets;
  Bitboard promo = bb & promoRank;
  bb &= ~promoRank;
  splat_pawn_moves<D + WEST>(bb, moveList);
  splat_promo_moves<D + WEST>(promo, moveList);

  bb = shift<D + EAST>(pawns & ~FILE_H) & legalTargets;
  promo = bb & promoRank;
  bb &= ~promoRank;
  splat_pawn_moves<D + EAST>(bb, moveList);
  splat_promo_moves<D + EAST>(promo, moveList);
}

template <Color C>
void gen_pawn_moves(Bitboard pawns, Bitboard targets, Bitboard empty,
                    Bitboard legal, MoveList &moveList) {
  constexpr Direction D = pawn_dir<C>();
  constexpr Bitboard promoRank = D == NORTH ? RANK_7 : RANK_2;
  gen_pawn_push<D>(pawns, empty, legal, moveList);
  gen_pawn_capture<D>(pawns, legal & targets, moveList);
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
      Square from = least_square(attackers);
      moveList.add_move(Move(from, epSquare, ENPASSANT));
      return;
    }
  }
  while (attackers) { // no edge case
    Square from = pop_least_square(attackers);
    moveList.add_move(Move(from, epSquare, ENPASSANT));
  }
}

void gen_knight_moves(Bitboard knights, Bitboard blocks, MoveList &moveList) {
  while (knights) {
    Square from = pop_least_square(knights);
    Bitboard attacks = attacks::knight_attacks(from) & ~blocks;
    splat_moves(from, attacks, moveList);
  }
}

template <Piece P>
void gen_slider_moves(Bitboard pieces, Bitboard blocks, Bitboard occupied,
                      MoveList &moveList) {
  while (pieces) {
    Square cur = pop_least_square(pieces);
    Bitboard attacks = attacks::sliding_attacks<P>(cur, occupied) & ~blocks;
    splat_moves(cur, attacks, moveList);
  }
}

void gen_king_moves(Bitboard king, Bitboard blocks, MoveList &moveList) {
  Square from = least_square(king);
  Bitboard attacks = attacks::king_attacks(from) & ~blocks;
  splat_moves(from, attacks, moveList);
}

template <Color C>
void gen_castle_moves(Bitboard enemyAttacks, Board &b, MoveList &moveList) {
  if constexpr (C == WHITE) {
    if (b.can_castle<WHITE_KINGSIDE>(enemyAttacks)) {
      moveList.add_move(Move(e1, g1, CASTLE));
    }
    if (b.can_castle<WHITE_QUEENSIDE>(enemyAttacks)) {
      moveList.add_move(Move(e1, c1, CASTLE));
    }
  } else {
    if (b.can_castle<BLACK_KINGSIDE>(enemyAttacks)) {
      moveList.add_move(Move(e8, g8, CASTLE));
    }
    if (b.can_castle<BLACK_QUEENSIDE>(enemyAttacks)) {
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
    Bitboard attacker_bb = 1ULL << attacker;
    Bitboard pin_ray = attacks::from_to_bb(king_square, attacker);
    Bitboard pin_ray_with_attacker = pin_ray | attacker_bb;
    Bitboard pinned_bb = pin_ray & b.get_pinned();
    Square pinned_piece = least_square(pinned_bb);
    Piece piece = piece_type_to_piece(b.piece_on(pinned_piece));

    if (piece == PAWN) {
      if (((1ULL << b.en_passant_square()) & pin_ray_with_attacker) !=
          0) { // enPassantable
        gen_en_passant<C>(pinned_bb, b.en_passant_square(), moveList);
      }

      Bitboard pawn_moves = attacks::pawn_attacks<C>(pinned_bb) & attacker_bb;
      pawn_moves |= shift<D>(pinned_bb) & pin_ray;
      if ((pinned_bb & (is_white ? RANK_2 : RANK_7)) != 0)
        pawn_moves |= shift<static_cast<Direction>(2 * D)>(pinned_bb) & pin_ray;

      pawn_moves &= pin_ray_with_attacker; // only keep moves along pin ray
      while (pawn_moves) {
        Square to = pop_least_square(pawn_moves);
        moveList.add_move(Move(pinned_piece, to));
        // no way to promo a pinned pawn
      }

    } else {
      switch (piece) {
      case KNIGHT:
        pin_ray_with_attacker = 0;
        break;
      case BISHOP:
        pin_ray_with_attacker &= attacks::diag_rays(pinned_piece);
        break;
      case ROOK:
        // only can be pinned once rook moves so no castling
        pin_ray_with_attacker &= attacks::ortho_rays(pinned_piece);
        break;
      case QUEEN:
        pin_ray_with_attacker &= (attacks::ortho_rays(pinned_piece) |
                                  attacks::diag_rays(pinned_piece));
        break;
      default:
        break;
      }
      while (pin_ray_with_attacker) {
        moveList.add_move(
            Move(pinned_piece, pop_least_square(pin_ray_with_attacker)));
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
                      ~b.get_pieces(), ALL_SQUARES, moveList);
    if (b.en_passant_square() != NO_SQUARE) {
      gen_en_passant<C>(b.get_pieces<PAWN>(C) & nonPinned,
                        b.get_pieces<KING>(C),
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
    gen_castle_moves<C>(attackedSquares, b, moveList);

  } else if ((b.get_checkers() & (b.get_checkers() - 1)) == 0) { // one checker
    Square checker = static_cast<Square>(std::countr_zero(b.get_checkers()));
    Bitboard legal =
        attacks::from_to_bb(checker, static_cast<Square>(std::countr_zero(
                                         b.get_pieces<KING>(C)))) |
        b.get_checkers();

    Bitboard nonPinned = ~b.get_pinned();
    gen_pawn_moves<C>(b.get_pieces<PAWN>(C) & nonPinned, b.get_pieces(~C),
                      ~b.get_pieces(), legal, moveList);
    constexpr Direction D = C == WHITE ? NORTH : SOUTH;
    if ((b.en_passant_square() != NO_SQUARE) &&
        (b.en_passant_square() - D ==
         checker)) { // can only en passant in check if the checker is the
                     // double pawn push
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
    gen_king_moves(b.get_pieces<KING>(C), b.get_pieces(C) | attackedSquares,
                   moveList);
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