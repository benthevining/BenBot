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

#include <cstddef> // IWYU pragma: keep - for size_t
#include <exception>
#include <functional>
#include <libutil/Threading.hpp>
#include <memory>
#include <system_error>
#include <utility>

#ifdef _WIN32
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN 1
#    endif
#    ifndef NOMINMAX
#        define NOMINMAX 1
#    endif
#    include <Windows.h>
#else
#    include <pthread.h>
#endif

namespace util {

struct LargeStackThread::Impl final {
    std::function<void()> func;

#ifdef _WIN32
    HANDLE handle { nullptr };
#else
    pthread_t handle { };
#endif

    // the function must not throw, just like the function run by a std::thread
    void run() noexcept
    {
        try {
            func();
        } catch (...) {
            std::terminate();
        }
    }

#ifdef _WIN32
    static DWORD WINAPI entry(LPVOID self)
    {
        static_cast<Impl*>(self)->run();
        return 0;
    }
#else
    static auto entry(void* self) -> void*
    {
        static_cast<Impl*>(self)->run();
        return nullptr;
    }
#endif
};

LargeStackThread::LargeStackThread(const std::size_t stackSize, std::function<void()> func)
    : impl { std::make_unique<Impl>() }
{
    impl->func = std::move(func);

#ifdef _WIN32
    // without this flag, the stack size would be the amount of memory committed up front,
    // instead of the amount reserved
    impl->handle = CreateThread(
        nullptr, stackSize, &Impl::entry, impl.get(), STACK_SIZE_PARAM_IS_A_RESERVATION, nullptr);

    if (impl->handle == nullptr) {
        throw std::system_error {
            static_cast<int>(GetLastError()), std::system_category(), "CreateThread() failed"
        };
    }
#else
    pthread_attr_t attributes;

    if (const auto result = pthread_attr_init(&attributes); result != 0)
        throw std::system_error { result, std::generic_category(), "pthread_attr_init() failed" };

    auto result = pthread_attr_setstacksize(&attributes, stackSize);

    if (result == 0)
        result = pthread_create(&impl->handle, &attributes, &Impl::entry, impl.get());

    pthread_attr_destroy(&attributes);

    if (result != 0)
        throw std::system_error { result, std::generic_category(), "Creating a thread failed" };
#endif
}

LargeStackThread::~LargeStackThread() = default;

void LargeStackThread::join()
{
#ifdef _WIN32
    WaitForSingleObject(impl->handle, INFINITE);
    CloseHandle(impl->handle);
#else
    pthread_join(impl->handle, nullptr);
#endif
}

} // namespace util
