#ifndef BENCHMARK_H_INCLUDED
#define BENCHMARK_H_INCLUDED

#include <iosfwd>
#include <string>
#include <vector>

namespace Stockfish {

class Position;

std::vector<std::string> setup_bench(const Position&, std::istream&);

}  // namespace Stockfish

#endif  // #ifndef BENCHMARK_H_INCLUDED
