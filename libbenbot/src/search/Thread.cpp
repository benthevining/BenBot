/*
 * ======================================================================================
 *
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
 *
 * ======================================================================================
 */

#include <libbenbot/search/Thread.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/util/Threading.hpp>
#include <utility>

namespace ben_bot::search {

Thread::Thread(Callbacks&& callbacksToUse)
    : context { std::move(callbacksToUse) }
{
}

Thread::~Thread()
{
    threadShouldExit.store(true);
    context.abort();
    searcherThread.join();
}

void Thread::set_position(const Position& pos)
{
    context.wait();

    context.options.position = pos;

    // clear this so that all legal moves will be searched by default
    context.options.movesToSearch.clear();
}

void Thread::start(
    const chess::uci::GoCommandOptions& options,
    const milliseconds                  moveOverheadTime)
{
    context.set_pondering(options.ponderMode);

    context.wait(); // shouldn't have been searching, but better safe than sorry

    context.options.moveOverhead = moveOverheadTime;
    context.options.update_from(options);

    startSearch.store(true);
}

void Thread::start()
{
    context.wait(); // shouldn't have been searching, but better safe than sorry

    startSearch.store(true);
}

void Thread::thread_func()
{
    while (true) {
        // we want to use progressive backoff to wait on the startSearch flag,
        // but we also need to exit the PB loop if the threadShouldExit flag
        // gets set
        chess::util::progressive_backoff([this] {
            return threadShouldExit.load() or startSearch.exchange(false);
        });

        if (threadShouldExit.load()) {
            return;
        }

        context.search();
    }
}

} // namespace ben_bot::search
