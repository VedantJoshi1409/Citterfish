#include <ostream>
#include <sstream>
#include "board.h"
#include <string>


std::ostream& operator<<(std::ostream& os, const Board& b) {
    std::string board = "  a b c d e f g h\n";

    return os << "(" << p.x << ", " << p.y << ")";
}