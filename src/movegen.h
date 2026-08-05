#include "types.h"
#include <array>

namespace citterfish {
struct MoveList {
    std::array<Move, MAX_MOVES> moves;
};
}