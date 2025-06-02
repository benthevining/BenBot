/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

// This file provides a Catch reporter that writes benchmark results in
// a format understood by CDash for historical measurement tracking

#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <catch2/reporters/catch_reporter_streaming_base.hpp>
#include <print>
#include <string>

namespace {
[[nodiscard, gnu::const]] double get_exec_ms(const Catch::BenchmarkStats<>& stats) noexcept
{
    return stats.info.estimatedDuration * 1e-6;
}
} // namespace

struct CDashBenchmarkReporter final : Catch::StreamingReporterBase {
    using StreamingReporterBase::StreamingReporterBase;

    [[nodiscard]] static std::string getDescription()
    {
        return "Output benchmark results in CDash dashboard measurement format";
    }

private:
    void benchmarkEnded(const Catch::BenchmarkStats<>& stats) override
    {
        std::println(
            R"-(<DartMeasurement name="{} (ms)" type="numeric/double">{}</DartMeasurement>)-",
            stats.info.name,
            get_exec_ms(stats));
    }
};

CATCH_REGISTER_REPORTER("cdash", CDashBenchmarkReporter)
