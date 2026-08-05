#include <cassert>
#include <type_traits>
#include "../src/types.h"

static_assert(std::is_trivially_copyable_v<citterfish::Move>);
static_assert(std::is_trivially_destructible_v<citterfish::Move>);
static_assert(std::is_standard_layout_v<citterfish::Move>);
static_assert(std::is_trivially_default_constructible_v<citterfish::Move>); 
static_assert(std::is_trivial_v<citterfish::Move>);  

int main() {}
