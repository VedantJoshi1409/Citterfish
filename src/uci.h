#include "board.h"
#include "perft.h"
#include <iostream>
#include <string_view>

namespace citterfish {

inline void temp_perft() {
  std::string lineIn;
  std::getline(std::cin, lineIn);
  Board board;
  while (lineIn != "quit") {
    size_t start = 0;
    size_t end = lineIn.find(' ');
    std::string type = lineIn.substr(start, end - start);
    start = end + 1;
    end = lineIn.find(' ', start);
    if (type == "position") {
      board = *new Board(lineIn.substr(end + 1));
      std::cout << board;
    } else if (type == "go") {
      int depth = std::stoi(lineIn.substr(end + 1));
      perft(depth, board);
    }
    std::getline(std::cin, lineIn);
  }
}
} // namespace citterfish