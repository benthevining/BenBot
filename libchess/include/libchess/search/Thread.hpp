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
#include <functional>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/search/Search.hpp>
#include <libchess/search/TranspositionTable.hpp>
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
    Thread(Thread&&)                 = delete;
    Thread& operator=(Thread&&)      = delete;

    ~Thread();

    /** Typedef for a callback that is invoked with the search result. */
    using Callback = std::function<void(Move)>;

    /** Launches the search asynchronously.
        The result callback is invoked on the background searcher thread
        once the search completes.
     */
    void run(
        const Options& options,
        Callback&&     callback);

    /** Blocks the calling thread until the ongoing search (if any) has finished.
        Note that this function does not signal to the search that it should exit.
     */
    void wait() const;

    /** Signals to the ongoing search, if any, that it should exit. */
    void interrupt();

    /** Aborts any ongoing search and clears the transposition table. Blocks while
        waiting for any ongoing search to exit.
     */
    void new_game();

private:
    // this is the function that the background thread spins in
    void thread_func();

    TranspositionTable transTable;

    Options searchOptions;

    // used to signal to the background thread that it should exit
    std::atomic_bool threadExitFlag { false };

    // used to signal to the current search that it should exit
    std::atomic_bool searchExitFlag { false };

    // used to signal to the background thread that it should start a new search
    std::atomic_bool startNewSearchFlag { false };

    // used by the background thread to signal to other threads that is has a search in progress
    std::atomic_bool searchInProgressFlag { false };

    std::thread thread { [this] { thread_func(); } };

    Callback resultCallback;
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
    interrupt();
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

            resultCallback(
                find_best_move(searchOptions, transTable, searchExitFlag));
        } else {
            std::this_thread::sleep_for(Milliseconds { 100 });
        }
    }
}

inline void Thread::wait() const
{
    while (searchInProgressFlag.load())
        std::this_thread::sleep_for(Milliseconds { 100 });
}

inline void Thread::interrupt()
{
    startNewSearchFlag.store(false);
    searchExitFlag.store(true);
}

inline void Thread::new_game()
{
    interrupt();
    wait();
    transTable.clear();
}

inline void Thread::run(
    const Options& options,
    Callback&&     callback)
{
    // exit previous search, if any
    interrupt();
    wait();

    // store parameters for thread to read

    resultCallback = std::move(callback);
    searchOptions  = options;

    // signal to start new search
    searchExitFlag.store(false);
    startNewSearchFlag.store(true);
}

} // namespace chess::search
