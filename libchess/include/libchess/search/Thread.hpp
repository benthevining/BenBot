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
#include <libchess/search/Search.hpp>
#include <thread>

namespace chess::search {

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

    /** Launches the search asynchronously.
        The background thread takes care of printing UCI "info" and "bestmove" output.
     */
    void run(const Options& options);

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

    Context<true> context;

    // used to signal to the background thread that it should exit
    std::atomic_bool threadExitFlag { false };

    // used to signal to the background thread that it should start a new search
    std::atomic_bool startNewSearchFlag { false };

    // used by the background thread to signal to other threads that is has a search in progress
    std::atomic_bool searchInProgressFlag { false };

    std::thread thread { [this] { thread_func(); } };
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

            [[maybe_unused]] const auto move = context.search();
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
    context.exitFlag.store(true);
}

inline void Thread::new_game()
{
    interrupt();
    wait();
    context.transTable.clear();
}

inline void Thread::run(const Options& options)
{
    // exit previous search, if any
    interrupt();
    wait();

    // store parameters for thread to read
    context.options = options;

    // signal to start new search
    startNewSearchFlag.store(true);
}

} // namespace chess::search
