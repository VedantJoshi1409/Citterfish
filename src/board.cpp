#include <ostream>
#include <sstream>
#include "board.h"
#include <string>
#include <ranges>

Board::Board(const std::string& fen) {
    // Implementation for initializing the board from a FEN string
}

Board::Board() {
    pieces[WHITE] = {65280ULL, 66ULL, 36ULL, 129ULL, 8ULL, 16ULL};
    pieces[BLACK] = {71776119061217280ULL, 4755801206503243776ULL, 2594073385365405696ULL, 9295429630892703744ULL, 576460752303423488ULL, 1152921504606846976ULL};
    occupied[WHITE] = 0;
    occupied[BLACK] = 0;
    castlingRights = 0;
    whiteToMove = true;
    enPassantSquare = 0;
    halfmoveClock = 0;
    zobristHash = 0;
}

std::ostream& operator<<(std::ostream& os, const Board& b) {
    auto stringBoard = b.toStringBoard();
    constexpr std::string_view separator = "+---+---+---+---+---+---+---+---+\n";
    std::string output{separator};
    for (const auto& row : std::views::reverse(stringBoard)) {
        output +="| ";
        for (const auto& square : row) {
            output += square == "" ? " " : square;
            output += " | ";
        }
        output += "\n" + std::string(separator);
    }
    
    os << output;
    return os;
}