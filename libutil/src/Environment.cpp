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

#include <cstdlib> // IWYU pragma: keep - for std::getenv()
#include <libutil/Environment.hpp>
#include <optional>
#include <string>
#include <string_view>

namespace util {

using std::optional;
using std::string;
using std::string_view;

namespace {
#ifdef _MSC_VER
    [[nodiscard]] auto get_env_var_internal(const char* name) -> optional<string>
    {
        char*  value { nullptr };
        size_t len { 0uz };

        [[maybe_unused]] const auto err = _dupenv_s(&value, &len, name);

        if (value != nullptr and len > 0uz)
            return string { value, len };

        return std::nullopt;
    }
#else
    [[nodiscard]] auto get_env_var_internal(const char* name) -> optional<string>
    {
        if (const auto* value = std::getenv(name))
            return { value };

        return std::nullopt;
    }
#endif
} // namespace

auto get_environment_variable(const string_view name) -> optional<string>
{
    // This is needed because string_view::data() may not be null-terminated
    const string nameStr { name };

    return get_env_var_internal(nameStr.c_str());
}

} // namespace util
