#include <ostream>
#include <sstream>
#include "board.h"
#include <string>


std::ostream& operator<<(std::ostream& os, const Board& b) {

    return os << "(" << p.x << ", " << p.y << ")";
}