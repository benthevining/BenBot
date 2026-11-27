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

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <libbenbot/eval/DefaultWeights.hpp>
#include <libbenbot/eval/Weights.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <limits>
#include <optional>
#include <print>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuner/Dataset.hpp>
#include <tuner/Extract.hpp>
#include <tuner/Model.hpp>
#include <tuner/Optimiser.hpp>
#include <tuner/Parallel.hpp>
#include <tuner/Resolve.hpp>
#include <tuner/Writer.hpp>
#include <utility>
#include <vector>

namespace {

using std::size_t;
using std::string_view;

using namespace ben_bot::tuner; // NOLINT

constexpr string_view USAGE = R"(Usage: benbot_tuner [tune] --data <file> [options]
       benbot_tuner extract --pgn <file> [options]
       benbot_tuner resolve --data <file> [options]

tune (the default): tunes BenBot's eval weights on a dataset of labelled positions, starting
from the current default weights, and writes the tuned weights as a new DefaultWeights.hpp.

Options:
  --data <file>         A dataset file (one labelled position per line). Can be repeated.
  --epochs <n>          The maximum number of epochs (default 1000).
  --lr <x>              The learning rate, in centipawns per epoch (default 1).
  --lr-drop-every <n>   Halve the learning rate every n epochs (default 1000, 0 = never).
  --threads <n>         The number of threads (default: the number of hardware threads).
  --validation <x>      The fraction of positions held out for validation (default 0.1).
  --output <file>       Where to write the tuned weights (default DefaultWeights.hpp).
  --save-every <n>      Also write the weights every n epochs (default 100, 0 = only at the end).
  --loss <mse|ce>       The loss: mean squared error or cross-entropy (default mse).
  --k <x>               Use this K instead of fitting it to the starting weights.
  --l2 <x>              Pull each weight towards its starting value by x * (w - w0)^2 (default 0).
  --patience <n>        Stop if the validation loss hasn't improved for n epochs (default 200, 0 = never).
  --seed <n>            The seed used to choose the validation positions (default 1).
  --limit <n>           Only load the first n positions that pass the filters (default 0 = all).
  --help                Print this message.

extract: samples positions from PGNs of self-play games (such as the tuner_datagen target's)
and writes them as a dataset, labelled with each game's result.

Options:
  --pgn <file>          A PGN file. Can be repeated.
  --output <file>       Where to write the positions (default extracted.book).
  --per-game <n>        The most positions to take from each game (default 10, 0 = all).
  --skip-plies <n>      The number of plies to skip after the book moves (default 8).
  --seed <n>            The seed used to choose the positions (default 1).

resolve: makes each position in a dataset quiet, by following a quiescence search's PV to its end.

Options:
  --data <file>         A dataset file. Can be repeated.
  --output <file>       Where to write the resolved positions (default resolved.book).
  --threads <n>         The number of threads (default: the number of hardware threads).
)";

struct Options final {
    std::vector<std::filesystem::path> dataFiles;

    size_t epochs { 1000uz };

    double learningRate { 1. };

    size_t lrDropEvery { 1000uz };

    size_t threads { default_num_threads() };

    double validationFraction { 0.1 };

    std::filesystem::path output { "DefaultWeights.hpp" };

    size_t saveEvery { 100uz };

    Loss loss { Loss::MeanSquaredError };

    std::optional<double> k;

    double l2 { 0. };

    size_t patience { 200uz };

    std::uint64_t seed { 1 };

    size_t limit { 0uz };

    bool help { false };
};

[[nodiscard]] auto parse_size(const string_view name, const string_view text) -> size_t
{
    const std::string copy { text };

    std::size_t used { 0 };

    try {
        const auto value = std::stoull(copy, &used);

        if (used == copy.size() and not copy.starts_with('-'))
            return static_cast<size_t>(value);
    } catch (const std::exception&) { // NOLINT(bugprone-empty-catch)
    }

    throw std::invalid_argument { std::format("{} needs a non-negative integer, not '{}'", name, text) };
}

[[nodiscard]] auto parse_double(const string_view name, const string_view text) -> double
{
    const std::string copy { text };

    std::size_t used { 0 };

    try {
        const auto value = std::stod(copy, &used);

        if (used == copy.size())
            return value;
    } catch (const std::exception&) { // NOLINT(bugprone-empty-catch)
    }

    throw std::invalid_argument { std::format("{} needs a number, not '{}'", name, text) };
}

// calls handle(name, value) for each option, where value() returns the option's value.
// Returns true if --help was given
template <typename Handler>
[[nodiscard]] auto parse_args(const std::span<const char* const> args, const Handler& handle) -> bool
{
    auto help = false;

    for (auto i = 0uz; i < args.size(); ++i) {
        string_view name { args[i] };

        std::optional<string_view> inlineValue;

        // accept --name=value as well as --name value
        if (const auto equals = name.find('='); name.starts_with("--") and equals != string_view::npos) {
            inlineValue = name.substr(equals + 1uz);
            name        = name.substr(0uz, equals);
        }

        if (name == "--help" or name == "-h") {
            help = true;
            continue;
        }

        const auto value = [&]() -> string_view {
            if (inlineValue.has_value())
                return *inlineValue;

            if (i + 1uz >= args.size())
                throw std::invalid_argument { std::format("{} needs a value", name) };

            return args[++i];
        };

        handle(name, value);
    }

    return help;
}

[[noreturn]] void unknown_option(const string_view name)
{
    throw std::invalid_argument { std::format("Unknown option '{}'", name) };
}

[[nodiscard]] auto parse_options(const std::span<const char* const> args) -> Options
{
    Options options;

    options.help = parse_args(args, [&options](const string_view name, const auto& value) {
        if (name == "--data")
            options.dataFiles.emplace_back(value());
        else if (name == "--epochs")
            options.epochs = parse_size(name, value());
        else if (name == "--lr")
            options.learningRate = parse_double(name, value());
        else if (name == "--lr-drop-every")
            options.lrDropEvery = parse_size(name, value());
        else if (name == "--threads")
            options.threads = parse_size(name, value());
        else if (name == "--validation")
            options.validationFraction = parse_double(name, value());
        else if (name == "--output")
            options.output = value();
        else if (name == "--save-every")
            options.saveEvery = parse_size(name, value());
        else if (name == "--loss") {
            if (const auto text = value(); not parse_loss(text, options.loss))
                throw std::invalid_argument { std::format("--loss must be mse or ce, not '{}'", text) };
        } else if (name == "--k")
            options.k = parse_double(name, value());
        else if (name == "--l2")
            options.l2 = parse_double(name, value());
        else if (name == "--patience")
            options.patience = parse_size(name, value());
        else if (name == "--seed")
            options.seed = parse_size(name, value());
        else if (name == "--limit")
            options.limit = parse_size(name, value());
        else
            unknown_option(name);
    });

    if (options.help)
        return options;

    if (options.dataFiles.empty())
        throw std::invalid_argument { "At least one --data file is needed" };

    if (options.threads == 0uz)
        throw std::invalid_argument { "--threads must be at least 1" };

    if (not(options.learningRate > 0.))
        throw std::invalid_argument { "--lr must be positive" };

    if (not(options.validationFraction >= 0. and options.validationFraction < 1.))
        throw std::invalid_argument { "--validation must be in [0, 1)" };

    if (options.k.has_value() and not(*options.k > 0.))
        throw std::invalid_argument { "--k must be positive" };

    if (not(options.l2 >= 0.))
        throw std::invalid_argument { "--l2 can't be negative" };

    return options;
}

struct ExtractCommand final {
    std::vector<std::filesystem::path> pgnFiles;

    std::filesystem::path output { "extracted.book" };

    ExtractOptions options;

    std::uint64_t seed { 1 };

    bool help { false };
};

[[nodiscard]] auto parse_extract_options(const std::span<const char* const> args) -> ExtractCommand
{
    ExtractCommand command;

    command.help = parse_args(args, [&command](const string_view name, const auto& value) {
        if (name == "--pgn")
            command.pgnFiles.emplace_back(value());
        else if (name == "--output")
            command.output = value();
        else if (name == "--per-game")
            command.options.positionsPerGame = parse_size(name, value());
        else if (name == "--skip-plies")
            command.options.skipPlies = parse_size(name, value());
        else if (name == "--seed")
            command.seed = parse_size(name, value());
        else
            unknown_option(name);
    });

    if (not command.help and command.pgnFiles.empty())
        throw std::invalid_argument { "At least one --pgn file is needed" };

    return command;
}

struct ResolveCommand final {
    std::vector<std::filesystem::path> dataFiles;

    std::filesystem::path output { "resolved.book" };

    size_t threads { default_num_threads() };

    bool help { false };
};

[[nodiscard]] auto parse_resolve_options(const std::span<const char* const> args) -> ResolveCommand
{
    ResolveCommand command;

    command.help = parse_args(args, [&command](const string_view name, const auto& value) {
        if (name == "--data")
            command.dataFiles.emplace_back(value());
        else if (name == "--output")
            command.output = value();
        else if (name == "--threads")
            command.threads = parse_size(name, value());
        else
            unknown_option(name);
    });

    if (command.help)
        return command;

    if (command.dataFiles.empty())
        throw std::invalid_argument { "At least one --data file is needed" };

    if (command.threads == 0uz)
        throw std::invalid_argument { "--threads must be at least 1" };

    return command;
}

[[nodiscard]] auto seconds_since(const std::chrono::steady_clock::time_point start) -> double
{
    return std::chrono::duration<double> { std::chrono::steady_clock::now() - start }.count();
}

void print_material(const Params& params)
{
    using ben_bot::eval::Term;
    using chess::pieces::Type;

    const auto weights = round_params(params);

    static constexpr std::array<std::pair<Type, string_view>, 5uz> pieces { {
        { Type::Pawn, "pawn" },
        { Type::Knight, "knight" },
        { Type::Bishop, "bishop" },
        { Type::Rook, "rook" },
        { Type::Queen, "queen" },
    } };

    std::print("Material (mg, eg):");

    for (const auto& [type, name] : pieces) {
        const auto value = weights.material(type);

        std::print(" {} ({}, {})", name, value.mg, value.eg);
    }

    std::println("");
}

void run(const Options& options)
{
    const auto start = std::chrono::steady_clock::now();

    std::println("Loading {} file(s) with {} thread(s)...", options.dataFiles.size(), options.threads);

    LoadStats stats;

    const auto [training, validation] = load_dataset(
        options.dataFiles,
        LoadOptions {
            .validationFraction = options.validationFraction,
            .seed               = options.seed,
            .numThreads         = options.threads,
            .limit              = options.limit },
        stats);

    std::println(
        "Read {} lines: kept {} positions, dropped {} in check & {} duplicates, skipped {} blank or comment "
        "lines, {} unparsable",
        stats.lines, stats.accepted, stats.inCheck, stats.duplicates, stats.skipped, stats.parseErrors);

    for (const auto& error : stats.firstErrors)
        std::println("  {}", error);

    if (training.empty())
        throw std::runtime_error { "No training positions" };

    const auto numTuples = training.tuples.size() + validation.tuples.size();

    std::println(
        "{} training & {} validation positions, {} coefficients ({:.1f} per position), traced & checked "
        "against evaluate() in {:.1f} s",
        training.size(), validation.size(), numTuples,
        static_cast<double>(numTuples) / static_cast<double>(training.size() + validation.size()),
        seconds_since(start));

    const auto startParams = to_params(ben_bot::eval::DEFAULT_WEIGHTS);

    auto params = startParams;

    const auto k = options.k.has_value() ? *options.k : fit_k(training, params, options.loss, options.threads);

    std::println("K = {:.6f}{}", k, options.k.has_value() ? " (given)" : " (fitted to the default weights)");

    const auto trainLoss = [&](const Params& p) { return compute_loss(training, p, k, options.loss, options.threads); };
    const auto validLoss = [&](const Params& p) { return compute_loss(validation, p, k, options.loss, options.threads); };

    std::println("Starting loss: training {:.8f}, validation {:.8f}", trainLoss(params), validLoss(params));

    if (options.l2 > 0.)
        std::println("Pulling the weights towards their starting values with an L2 penalty of {:g}", options.l2);

    Adam adam { tunable_mask() };

    auto learningRate = options.learningRate;

    auto       bestValidation  = std::numeric_limits<double>::infinity();
    auto       epochsSinceBest = 0uz;
    auto       epochsRun       = 0uz;
    const auto tuneStart       = std::chrono::steady_clock::now();
    const auto useValidation   = not validation.empty();

    for (auto epoch = 1uz; epoch <= options.epochs; ++epoch) {
        // the training & validation losses printed are both for the weights before this epoch's step
        auto [loss, gradient] = compute_loss_and_gradient(training, params, k, options.loss, options.threads);

        // the losses printed are without the L2 penalty, so that they can be compared between runs
        if (options.l2 > 0.)
            add_l2_pull(gradient, params, startParams, options.l2);

        const auto validation_loss = validLoss(params);

        std::println(
            "epoch {:5}  training {:.8f}  validation {:.8f}  lr {:.4f}  {:.1f} s",
            epoch, loss, validation_loss, learningRate, seconds_since(tuneStart));

        std::fflush(stdout);

        adam.step(params, gradient, learningRate);

        epochsRun = epoch;

        if (options.lrDropEvery > 0uz and epoch % options.lrDropEvery == 0uz)
            learningRate /= 2.;

        if (options.saveEvery > 0uz and epoch % options.saveEvery == 0uz)
            write_weights(options.output, recentre(params));

        if (useValidation and options.patience > 0uz) {
            if (validation_loss < bestValidation) {
                bestValidation  = validation_loss;
                epochsSinceBest = 0uz;
            } else if (++epochsSinceBest >= options.patience) {
                std::println("The validation loss hasn't improved for {} epochs, stopping", options.patience);
                break;
            }
        }
    }

    std::println(
        "Final loss after {} epochs: training {:.8f}, validation {:.8f}", epochsRun, trainLoss(params), validLoss(params));

    // the weights are written with each piece square & mobility table's average moved into the
    // material, which doesn't change any eval (see recentre())
    const auto recentred = recentre(params);

    std::println(
        "Final loss with the tables re-centred: training {:.8f}, validation {:.8f}",
        trainLoss(recentred), validLoss(recentred));

    const auto rounded = to_params(round_params(recentred));

    std::println(
        "Final loss with the weights rounded to integers: training {:.8f}, validation {:.8f}",
        trainLoss(rounded), validLoss(rounded));

    print_material(recentred);

    write_weights(options.output, recentred);

    std::println("Wrote {} ({:.1f} s in total)", options.output.string(), seconds_since(start));
}

[[nodiscard]] auto open_input(const std::filesystem::path& path) -> std::ifstream
{
    std::ifstream input { path };

    if (not input.is_open())
        throw std::runtime_error { std::format("Can't open '{}'", path.string()) };

    return input;
}

[[nodiscard]] auto open_output(const std::filesystem::path& path) -> std::ofstream
{
    std::ofstream output { path, std::ios::binary | std::ios::trunc };

    if (not output.is_open())
        throw std::runtime_error { std::format("Can't write to '{}'", path.string()) };

    return output;
}

void close_output(std::ofstream& output, const std::filesystem::path& path)
{
    output.close();

    if (output.fail())
        throw std::runtime_error { std::format("Couldn't write all of '{}'", path.string()) };
}

void run_extract(const ExtractCommand& command)
{
    const auto start = std::chrono::steady_clock::now();

    auto output = open_output(command.output);

    std::mt19937_64 rng { command.seed };

    ExtractStats stats;

    for (const auto& path : command.pgnFiles) {
        auto input = open_input(path);

        extract(input, output, command.options, rng, stats);
    }

    close_output(output, command.output);

    std::println(
        "Read {} games ({} skipped with no result or unparsable). Of their positions, skipped {} in the "
        "opening, {} in check & {} with mate scores, leaving {}",
        stats.games, stats.skippedGames, stats.openingPositions, stats.inCheck, stats.mateScores,
        stats.candidates);

    std::println(
        "Wrote {} positions to {} ({:.1f} s)", stats.extracted, command.output.string(), seconds_since(start));
}

void run_resolve(const ResolveCommand& command)
{
    const auto start = std::chrono::steady_clock::now();

    auto output = open_output(command.output);

    ResolveStats stats;

    for (const auto& path : command.dataFiles) {
        auto input = open_input(path);

        resolve_dataset(input, output, command.threads, stats);
    }

    close_output(output, command.output);

    std::println(
        "Read {} lines: skipped {} blank or comment lines, {} unparsable",
        stats.lines, stats.skipped, stats.parseErrors);

    for (const auto& error : stats.firstErrors)
        std::println("  {}", error);

    std::println(
        "{} positions were already quiet, {} were resolved, and {} were dropped because their PV ended in mate",
        stats.unchanged, stats.changed, stats.inCheck);

    std::println(
        "Wrote {} positions to {} ({:.1f} s)", stats.unchanged + stats.changed, command.output.string(),
        seconds_since(start));
}

} // namespace

int main(const int argc, const char** argv)
try {
    auto args = std::span { argv, static_cast<size_t>(argc) }.subspan(1uz);

    // the command is optional, and defaults to tune
    const auto command = args.empty() ? string_view { } : string_view { args.front() };

    if (command == "tune" or command == "extract" or command == "resolve")
        args = args.subspan(1uz);

    const auto print_usage = [] { std::print("{}", USAGE); };

    if (command == "extract") {
        const auto options = parse_extract_options(args);

        if (options.help)
            print_usage();
        else
            run_extract(options);

        return 0;
    }

    if (command == "resolve") {
        const auto options = parse_resolve_options(args);

        if (options.help)
            print_usage();
        else
            run_resolve(options);

        return 0;
    }

    const auto options = parse_options(args);

    if (options.help) {
        print_usage();
        return 0;
    }

    run(options);

    return 0;
} catch (const std::invalid_argument& error) {
    std::println(stderr, "{}\n\n{}", error.what(), USAGE);
    return 1;
} catch (const std::exception& error) {
    std::println(stderr, "Error: {}", error.what());
    return 1;
}
