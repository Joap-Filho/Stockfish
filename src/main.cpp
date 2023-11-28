#include <cstddef>
#include <iostream>

#include "bitboard.h"
#include "evaluate.h"
#include "misc.h"
#include "position.h"
#include "search.h"
#include "thread.h"
#include "tune.h"
#include "types.h"
#include "uci.h"

using namespace Stockfish;

int main(int argc, char* argv[]) {

    std::cout << engine_info() << std::endl;

    CommandLine::init(argc, argv);
    UCI::init(Options);
    Tune::init();
    Bitboards::init();
    Position::init();
    Threads.set(size_t(Options["Threads"]));
    Search::clear();  // After threads are up
    Eval::NNUE::init();

    UCI::loop(argc, argv);

    Threads.set(0);
    return 0;
}
