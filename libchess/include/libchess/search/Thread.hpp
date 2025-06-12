/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file defines the searcher thread class.
    @ingroup search
 */

#pragma once

#include <atomic>
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <future>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/search/Search.hpp>
#include <libchess/search/TranspositionTable.hpp>
#include <limits>
#include <optional>
#include <thread>
#include <utility>

namespace chess::search {

using game::Position;
using moves::Move;
using std::size_t;

using Milliseconds = std::chrono::milliseconds;

/** This class encapsulates a background searcher thread.
    The thread can be launched asynchronously with a given
    time limit, and interrupted when needed.

    @ingroup search
 */
class Thread final {
public:
    Thread() = default;

    Thread(const Thread&)            = delete;
    Thread& operator=(const Thread&) = delete;

    Thread(Thread&&)            = default;
    Thread& operator=(Thread&&) = default;

    ~Thread();

    /** Launches the search asynchronously. */
    [[nodiscard]] void run(
        std::promise<Move>          result,
        const Position&             position,
        size_t                      maxDepth      = std::numeric_limits<size_t>::max(),
        std::optional<Milliseconds> maxSearchTime = std::nullopt);

    /** Blocks the calling thread until the ongoing search (if any) has finished.
        Note that this function does not signal to the search that it should exit.
     */
    void wait();

    /** Signals to the ongoing search, if any, that it should exit. */
    void abort();

private:
    // this is the function that the background thread spins in
    void thread_func();

    TranspositionTable transTable;

    // used to signal to the background thread that it should exit
    std::atomic_bool threadExitFlag { false };

    // used to signal to the current search that it should exit
    std::atomic_bool searchExitFlag { false };

    // used to signal to the background thread that it should start a new search
    std::atomic_bool startNewSearchFlag { false };

    // used by the background thread to signal to other threads that is has a search in progress
    std::atomic_bool searchInProgressFlag { false };

    std::thread thread[this] { thread_func(); }

    std::promise<Move> promise;

    Position positionToSearch;

    size_t searchDepth { std::numeric_limits<size_t>::max() };

    std::optional<Milliseconds> searchTime;
};

/*
                         ___                           ,--,
      ,---,            ,--.'|_                ,--,   ,--.'|
    ,---.'|            |  | :,'             ,--.'|   |  | :
    |   | :            :  : ' :             |  |,    :  : '    .--.--.
    |   | |   ,---.  .;__,'  /    ,--.--.   `--'_    |  ' |   /  /    '
  ,--.__| |  /     \ |  |   |    /       \  ,' ,'|   '  | |  |  :  /`./
 /   ,'   | /    /  |:__,'| :   .--.  .-. | '  | |   |  | :  |  :  ;_
.   '  /  |.    ' / |  '  : |__  \__\/: . . |  | :   '  : |__ \  \    `.
'   ; |:  |'   ;   /|  |  | '.'| ," .--.; | '  : |__ |  | '.'| `----.   \
|   | '/  ''   |  / |  ;  :    ;/  /  ,.  | |  | '.'|;  :    ;/  /`--'  /__  ___  ___
|   :    :||   :    |  |  ,   /;  :   .'   \;  :    ;|  ,   /'--'.     /  .\/  .\/  .\
 \   \  /   \   \  /    ---`-' |  ,     .-./|  ,   /  ---`-'   `--'---'\  ; \  ; \  ; |
  `----'     `----'             `--`---'     ---`-'                     `--" `--" `--"

 */

inline Thread::~Thread()
{
    threadExitFlag.store(true);
    abort();
    thread.join();
}

inline void Thread::thread_func()
{
    struct ScopedSetter final {
        explicit ScopedSetter(std::atomic_bool& flagToUse)
            : flag { flagToUse }
        {
            flag.store(true);
        }

        ~ScopedSetter() { flag.store(false); }

    private:
        std::atomic_bool& flag;
    };

    while (! threadExitFlag.load()) {
        if (startNewSearchFlag.exchange(false)) {
            const ScopedSetter raii { searchInProgressFlag };

            promise.set_value(
                find_best_move(positionToSearch, transTable, searchExitFlag, searchDepth));
        } else {
            std::this_thread::sleep_for(Milliseconds { 100 });
        }
    }
}

inline void Thread::wait()
{
    while (searchInProgressFlag.load())
        std::this_thread::sleep_for(Milliseconds { 100 });
}

inline void Thread::abort()
{
    startNewSearchFlag.store(false);
    searchExitFlag.store(true);
}

inline void Thread::run(
    std::promise<Move>          result,
    const Position&             position,
    const size_t                maxDepth,
    std::optional<Milliseconds> maxSearchTime)
{
    // exit previous search, if any
    abort();
    wait();

    // store parameters for thread to read

    promise = std::move(result);

    positionToSearch = position;
    searchDepth      = maxDepth;
    searchTime       = maxSearchTime;

    // signal to start new search
    searchExitFlag.store(false);
    startNewSearchFlag.store(true);
}

} // namespace chess::search
