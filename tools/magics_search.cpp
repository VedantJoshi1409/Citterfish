/*
Leaving this here for now
void recordMagic(uint64_t state, uint64_t magic, Square square,
                 int constructiveCollisions, uint64_t maxIdx, int shiftOffset,
                 std::ofstream &outputFile) {
  outputFile << std::format("State: {}\t\tSquare: {}\t\tMagic: {}\t\tGood "
                            "Collisions: {}\t\tMax Index: {}\t\tSaved Bits: {}",
                            state, squareToString(square), magic,
                            constructiveCollisions, maxIdx, shiftOffset)
             << std::endl;
}

template <Piece P>
void fillMagicSearchTable(Square square, Bitboard attackMask,
                          std::array<Bitboard, 4096> &occupancyTable,
                          std::array<Bitboard, 4096> &attackTable) {
  int bitcount = std::popcount(attackMask);
  int tableSize = 1 << bitcount;
  for (int i = 0; i < tableSize; ++i) {
    occupancyTable[i] = PDEP(i, attackMask); // compute occupancies
    attackTable[i] =
        (P == ROOK) ? slowRookAttack(square, occupancyTable[i])
                    : slowBishopAttack(
                          square, occupancyTable[i]); // find slow attack mask
                                                      // for each occupancy
  }
}

template <Piece P>
void findMagics(Square square, int epochs, uint64_t seed = 0xDEADBEEF) {
  PRNG rng(seed);
  Bitboard mask = (P == ROOK) ? rookMasks[static_cast<int>(square)]
                              : bishopMasks[static_cast<int>(square)];
  int bitcount = std::popcount(mask);
  int tableSize = 1 << bitcount;
  uint64_t stamp = 0;

  std::array<Bitboard, 4096>
      map; // map between the hashed idx and the attack masks

  std::array<Bitboard, 4096>
      occupancyTable; // contains <tableSize> occupancy masks
  std::array<Bitboard, 4096>
      attackTable; // contains the attackMask for each occupancy
  std::array<Bitboard, 4096>
      lastUpdated{}; // contains when the last time the map was updated
  fillMagicSearchTable<P>(square, mask, occupancyTable, attackTable);

  std::ofstream outputFile("magics.txt", std::ios::app);

  for (int epoch = 0; epoch < epochs; ++epoch) {
    stamp++;
    if (epoch % 1000000 == 0)
      std::cout << "On epoch " << epoch << std::endl;
    uint64_t state = rng.getState();
    uint64_t magic = rng.sparse();
    while (std::popcount((mask * magic) >> 56) < 8) {
      state = rng.getState();
      magic = rng.sparse();
    }
    bool failed = false;
    for (int shiftOffset = 0; shiftOffset < bitcount && !failed;
         ++shiftOffset) { // trying to find best magics
      stamp++;
      int constructiveCollisions = 0;
      uint64_t maxIdx = 0;
      for (int occIdx = 0; occIdx < tableSize;
           ++occIdx) { // loop through each possible occupancy mask
        uint64_t idx =
            (occupancyTable[occIdx] * magic) >> (64 - bitcount + shiftOffset);
        maxIdx = std::max(maxIdx, idx);
        if (lastUpdated[idx] < stamp) { // fresh hash
          map[idx] = attackTable[occIdx];
          lastUpdated[idx] = stamp;
        } else if (map[idx] != attackTable[occIdx]) { // destructive collision
          failed = true;
          break;
        } else {
          ++constructiveCollisions;
        }
      }
      if (!failed)
        recordMagic(state, magic, square, constructiveCollisions, maxIdx,
                    shiftOffset, outputFile);
    }
  }
  outputFile.close();
}

template <Piece P> void testMagic(Square square, uint64_t magic) {
  Bitboard mask = (P == ROOK) ? rookMasks[static_cast<int>(square)]
                              : bishopMasks[static_cast<int>(square)];
  int bitcount = std::popcount(mask);
  int tableSize = 1 << bitcount;

  std::array<Bitboard, 4096> map{
      0}; // map between the hashed idx and the attack masks

  std::array<Bitboard, 4096>
      occupancyTable; // contains <tableSize> occupancy masks
  std::array<Bitboard, 4096>
      attackTable; // contains the attackMask for each occupancy
  fillMagicSearchTable<P>(square, mask, occupancyTable, attackTable);
  bool failed = false;
  for (int shiftOffset = 0; shiftOffset < bitcount && !failed;
       ++shiftOffset) { // trying to find best magics
    std::cout << "Attempting shift offset " << shiftOffset << std::endl;
    int constructiveCollisions = 0;
    uint64_t maxIdx = 0;
    for (int occIdx = 0; occIdx < tableSize;
         ++occIdx) { // loop through each possible occupancy mask
      uint64_t idx =
          (occupancyTable[occIdx] * magic) >> (64 - bitcount + shiftOffset);
      maxIdx = std::max(maxIdx, idx);
      if (map[idx] == 0) { // fresh hash
        map[idx] = attackTable[occIdx];
      } else if (map[idx] != attackTable[occIdx]) { // destructive collision
        failed = true;
        break;
      } else {
        ++constructiveCollisions;
      }
    }
    if (!failed) {
      std::cout << std::format("Square: {}\t\tMagic: {}\t\tGood "
                               "Collisions: {}\t\tMax Index: {}",
                               squareToString(square), magic,
                               constructiveCollisions, maxIdx)
                << std::endl;
    } else {
      std::cout << "FAILED" << std::endl;
    }
    map.fill(0); // if checking for best magic, reset
  }
}

*/