#pragma once

#include "board.h"
#include "movegen.h"
#include "stdio.h"
#include "types.h"
#include <cassert>
#include <chrono>
#include <cinttypes>
#include <cstdint>

namespace citterfish {
namespace detail {
inline MoveList moveStack;
}

inline uint64_t perft_helper(int depth, Board &b) {
  if (depth <= 0) {
    return 1;
  }
  MoveList moveList;
  gen_moves(b, moveList);
  if (depth == 1) {
    return moveList.count;
  }
  StateInfo st;
  uint64_t nodes = 0;

  for (int i = 0; i < moveList.count; i++) {
    Move move = moveList.moves[i];
#ifndef NDEBUG
    const Board before = b;
    const StateInfo stBefore = *b.get_state();
#endif
    b.make_move(move, &st);
    detail::moveStack.add_move(move);
    nodes += perft_helper(depth - 1, b);
    b.unmake_move(move);
    --detail::moveStack.count;

#ifndef NDEBUG
    if (before != b || stBefore != *b.get_state()) {
      std::cout << b << std::endl;
      for (int i = 0; i < detail::moveStack.count; i++) {
        std::cout << detail::moveStack.moves[i] << " ";
      }
      std::cout << std::endl;
    }
#endif
    assert(before == b);
    assert(stBefore == *b.get_state());
  }
  return nodes;
}

inline void perft(int depth, Board &b) {
  auto start = std::chrono::steady_clock::now();
  detail::moveStack.count = 0;
  MoveList moveList;
  gen_moves(b, moveList);
  if (depth <= 0) {
    std::printf("Moves Generated: 0\n");
  } else {
    StateInfo st;
    uint64_t nodes = 0;
    for (int i = 0; i < moveList.count; i++) {
      Move move = moveList.moves[i];
      if (depth > 1) {
#ifndef NDEBUG
        const Board before = b;
        const StateInfo stBefore = *b.get_state();

#endif
        std::cout << "Making move: " << move
                  << static_cast<int>(move.get_move_type()) << std::endl;
        b.make_move(move, &st);
        detail::moveStack.add_move(move);
        uint64_t count = perft_helper(depth - 1, b);
        std::printf("%s%s: %" PRIu64 "\n",
                    square_to_string(move.get_from_square()).c_str(),
                    square_to_string(move.get_to_square()).c_str(), count);
        nodes += count;
        b.unmake_move(move);
        --detail::moveStack.count;
        assert(before == b);
        assert(stBefore == *b.get_state());
      } else {
        std::printf("%s%s\n", square_to_string(move.get_from_square()).c_str(),
                    square_to_string(move.get_to_square()).c_str());
        ++nodes;
      }
    }
    printf("Moves Generated: %" PRIu64 "\n", nodes);
    auto end = std::chrono::steady_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Nodes/s: "
              << static_cast<uint64_t>(static_cast<float>(nodes) /
                                       elapsed.count() * 1000)
              << std::endl;
  }
}
} // namespace citterfish
