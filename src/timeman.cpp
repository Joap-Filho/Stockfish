#include "timeman.h"

#include <algorithm>
#include <cmath>

#include "search.h"
#include "uci.h"

namespace Stockfish {

TimeManagement Time;  // Our global time management object


// Called at the beginning of the search and calculates
// the bounds of time allowed for the current game ply. We currently support:
//      1) x basetime (+ z increment)
//      2) x moves in y seconds (+ z increment)
void TimeManagement::init(Search::LimitsType& limits, Color us, int ply) {

    // If we have no time, no need to initialize TM, except for the start time,
    // which is used by movetime.
    startTime = limits.startTime;
    if (limits.time[us] == 0)
        return;

    TimePoint moveOverhead = TimePoint(Options["Move Overhead"]);
    TimePoint slowMover    = TimePoint(Options["Slow Mover"]);
    TimePoint npmsec       = TimePoint(Options["nodestime"]);

    // optScale is a percentage of available time to use for the current move.
    // maxScale is a multiplier applied to optimumTime.
    double optScale, maxScale;

    // If we have to play in 'nodes as time' mode, then convert from time
    // to nodes, and use resulting values in time management formulas.
    // WARNING: to avoid time losses, the given npmsec (nodes per millisecond)
    // must be much lower than the real engine speed.
    if (npmsec)
    {
        if (!availableNodes)                            // Only once at game start
            availableNodes = npmsec * limits.time[us];  // Time is in msec

        // Convert from milliseconds to nodes
        limits.time[us] = TimePoint(availableNodes);
        limits.inc[us] *= npmsec;
        limits.npmsec = npmsec;
    }

    // Maximum move horizon of 50 moves
    int mtg = limits.movestogo ? std::min(limits.movestogo, 50) : 50;

    // Make sure timeLeft is > 0 since we may use it as a divisor
    TimePoint timeLeft = std::max(TimePoint(1), limits.time[us] + limits.inc[us] * (mtg - 1)
                                                  - moveOverhead * (2 + mtg));

    // Use extra time with larger increments
    double optExtra = std::clamp(1.0 + 12.5 * limits.inc[us] / limits.time[us], 1.0, 1.12);

    // Calculate time constants based on current time left.
    double optConstant = std::min(0.00335 + 0.0003 * std::log10(limits.time[us] / 1000.0), 0.0048);
    double maxConstant = std::max(3.6 + 3.0 * std::log10(limits.time[us] / 1000.0), 2.7);

    // A user may scale time usage by setting UCI option "Slow Mover"
    // Default is 100 and changing this value will probably lose elo.
    timeLeft = slowMover * timeLeft / 100;

    // x basetime (+ z increment)
    // If there is a healthy increment, timeLeft can exceed actual available
    // game time for the current move, so also cap to 20% of available game time.
    if (limits.movestogo == 0)
    {
        optScale = std::min(0.0120 + std::pow(ply + 3.3, 0.44) * optConstant,
                            0.2 * limits.time[us] / double(timeLeft))
                 * optExtra;
        maxScale = std::min(6.8, maxConstant + ply / 12.2);
    }

    // x moves in y seconds (+ z increment)
    else
    {
        optScale = std::min((0.88 + ply / 116.4) / mtg, 0.88 * limits.time[us] / double(timeLeft));
        maxScale = std::min(6.3, 1.5 + 0.11 * mtg);
    }

    // Limit the maximum possible time for this move
    optimumTime = TimePoint(optScale * timeLeft);
    maximumTime =
      TimePoint(std::min(0.84 * limits.time[us] - moveOverhead, maxScale * optimumTime)) - 10;

    if (Options["Ponder"])
        optimumTime += optimumTime / 4;
}

}  // namespace Stockfish
