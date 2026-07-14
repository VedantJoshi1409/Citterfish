#include <cstdint>

typedef std::uint64_t ll;

class Board {

  static const ll wSKing = 4611686018427387904ULL,
                  wLKing = 288230376151711744ULL;
  static const ll wSRook = 11529215046068469760ULL,
                  wLRook = 648518346341351424ULL;
  static const ll bSKing = 64ULL, bLKing = 4ULL;
  static const ll bSRook = 160ULL, bLRook = 9ULL;
};