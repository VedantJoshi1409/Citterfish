#include <cassert>
#include <type_traits>
#include "../src/types.h"
#include "../src/movegen.h"

template <typename T>
void isTrivial() {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    static_assert(std::is_trivially_destructible_v<T>, "T must be trivially destructible");
    static_assert(std::is_standard_layout_v<T>, "T must have standard layout");
    static_assert(std::is_trivially_default_constructible_v<T>, "T must be trivially default constructible");
    static_assert(std::is_trivial_v<T>, "T must be trivial");
}

int main() {
    isTrivial<citterfish::Move>();
    isTrivial<citterfish::MoveList>();
}
