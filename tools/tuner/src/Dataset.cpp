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

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <istream>
#include <libbenbot/eval/DefaultWeights.hpp>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/Trace.hpp>
#include <libbenbot/eval/Weights.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <limits>
#include <optional>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <tuner/Dataset.hpp>
#include <tuner/Model.hpp>
#include <tuner/Parallel.hpp>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ben_bot::tuner {

namespace {

    using std::string;
    using std::string_view;

    [[nodiscard]] auto is_space(const char character) noexcept -> bool
    {
        return std::isspace(static_cast<unsigned char>(character)) != 0;
    }

    [[nodiscard]] auto trim(string_view text) noexcept -> string_view
    {
        while (not text.empty() and is_space(text.front()))
            text.remove_prefix(1uz);

        while (not text.empty() and is_space(text.back()))
            text.remove_suffix(1uz);

        return text;
    }

    [[nodiscard]] auto split_words(const string_view text) -> std::vector<string_view>
    {
        std::vector<string_view> words;

        auto start = 0uz;

        while (start < text.size()) {
            while (start < text.size() and is_space(text[start]))
                ++start;

            auto end = start;

            while (end < text.size() and not is_space(text[end]))
                ++end;

            if (end > start)
                words.emplace_back(text.substr(start, end - start));

            start = end;
        }

        return words;
    }

    [[nodiscard]] auto parse_result_word(const string_view word) noexcept -> std::optional<double>
    {
        if (word == "1-0")
            return 1.;

        if (word == "0-1")
            return 0.;

        if (word == "1/2-1/2" or word == "1/2")
            return 0.5;

        return std::nullopt;
    }

    [[nodiscard]] auto parse_number(const string_view text) -> std::optional<double>
    {
        if (text.empty())
            return std::nullopt;

        const string copy { text };

        char* end { nullptr };

        const auto value = std::strtod(copy.c_str(), &end);

        if (end != copy.c_str() + copy.size() or not std::isfinite(value))
            return std::nullopt;

        return value;
    }

    [[nodiscard]] auto is_integer(const string_view word) noexcept -> bool
    {
        return not word.empty()
           and std::ranges::all_of(word, [](const char character) {
                   return std::isdigit(static_cast<unsigned char>(character)) != 0;
               });
    }

    struct SplitLine final {
        string_view fen;

        std::optional<double> result;
    };

    // splits a line into the text before the result, and the result
    [[nodiscard]] auto split_label(const string_view line) -> std::expected<SplitLine, string>
    {
        // [1.0]
        if (const auto open = line.find('['); open != string_view::npos) {
            const auto close = line.find(']', open);

            if (close == string_view::npos)
                return std::unexpected { string { "unterminated [" } };

            const auto content = trim(line.substr(open + 1uz, close - open - 1uz));

            auto result = parse_number(content);

            if (not result.has_value())
                result = parse_result_word(content);

            return SplitLine { .fen = line.substr(0uz, open), .result = result };
        }

        // "1-0"
        if (const auto open = line.find('"'); open != string_view::npos) {
            const auto close = line.find('"', open + 1uz);

            if (close == string_view::npos)
                return std::unexpected { string { "unterminated quote" } };

            const auto content = trim(line.substr(open + 1uz, close - open - 1uz));

            return SplitLine { .fen = line.substr(0uz, open), .result = parse_result_word(content) };
        }

        // a bare 1-0 as the last word
        const auto lastSpace = line.find_last_of(" \t");

        if (lastSpace == string_view::npos)
            return std::unexpected { string { "no result found" } };

        auto lastWord = line.substr(lastSpace + 1uz);

        while (not lastWord.empty() and (lastWord.back() == ';' or lastWord.back() == ','))
            lastWord.remove_suffix(1uz);

        return SplitLine { .fen = line.substr(0uz, lastSpace), .result = parse_result_word(lastWord) };
    }

    // the eval's weights as tuner parameters, & which of them are tunable
    [[nodiscard]] auto default_params() -> const Params&
    {
        static const auto params = to_params(eval::DEFAULT_WEIGHTS);

        return params;
    }

    [[nodiscard]] auto weight_is_tunable() -> const std::vector<bool>&
    {
        static const auto mask = tunable_mask();

        return mask;
    }

    // traces one position and appends it to the dataset
    void trace_into(Dataset& dataset, const LabelledPosition& labelled)
    {
        const auto& position = labelled.position;

        const auto trace = eval::evaluate_traced(position);

        const auto engineEval = static_cast<int>(eval::evaluate(position));

        const auto describe = [&position] { return chess::notation::to_fen(position); };

        if (const auto traced = static_cast<int>(trace.score(eval::DEFAULT_WEIGHTS)); traced != engineEval) {
            throw std::runtime_error {
                std::format(
                    "The eval rebuilt from the trace ({}) isn't the eval ({}) for {}",
                    traced, engineEval, describe())
            };
        }

        const auto& tunable = weight_is_tunable();

        Entry entry {
            .result     = static_cast<float>(labelled.result),
            .phase      = static_cast<std::uint16_t>(trace.phase),
            .firstTuple = static_cast<std::uint32_t>(dataset.tuples.size()),
            .baseMg     = trace.base.mg,
            .baseEg     = trace.base.eg
        };

        if (dataset.tuples.size() > std::numeric_limits<std::uint32_t>::max())
            throw std::runtime_error { "Too many coefficients for one dataset" };

        auto numTuples = 0uz;

        for (auto index = 0uz; index < eval::NUM_WEIGHTS; ++index) {
            const auto coeff = trace.coefficient(index);

            if (coeff == 0)
                continue;

            if (not tunable[index]) {
                // a non-tunable term never changes, so it's part of the base
                const auto weight = eval::DEFAULT_WEIGHTS.values.at(index);

                entry.baseMg += weight.mg * coeff;
                entry.baseEg += weight.eg * coeff;

                continue;
            }

            if (coeff < std::numeric_limits<std::int16_t>::min()
                or coeff > std::numeric_limits<std::int16_t>::max()) {
                throw std::runtime_error { std::format("Coefficient {} out of range for {}", coeff, describe()) };
            }

            dataset.tuples.push_back({ .index = static_cast<std::uint16_t>(index),
                .coeff                        = static_cast<std::int16_t>(coeff) });

            ++numTuples;
        }

        entry.numTuples = static_cast<std::uint16_t>(numTuples);

        // the model doesn't truncate when it blends the phases, so it can be up to 1 cp from the engine
        const auto whiteEval = model_eval(dataset, entry, default_params());

        const auto modelEval = position.sideToMove == chess::pieces::Color::White ? whiteEval : -whiteEval;

        if (std::abs(modelEval - engineEval) >= 1.) {
            throw std::runtime_error {
                std::format(
                    "The tuner's model ({}) isn't within 1 cp of the eval ({}) for {}",
                    modelEval, engineEval, describe())
            };
        }

        dataset.entries.push_back(entry);
    }

    class Loader final {
    public:
        Loader(const LoadOptions& opts, LoadStats& statistics)
            : options { opts }
            , stats { statistics }
            , rng { opts.seed }
        {
            if (not(options.validationFraction >= 0. and options.validationFraction < 1.))
                throw std::invalid_argument { "The validation fraction must be in [0, 1)" };
        }

        void read(std::istream& input)
        {
            string line;

            while (not is_full() and std::getline(input, line)) {
                process(line);

                if (training.size() + validation.size() >= BATCH_SIZE)
                    flush();
            }
        }

        [[nodiscard]] auto finish() -> SplitDataset
        {
            flush();

            return std::move(result);
        }

    private:
        // the positions are traced in batches of this many, so that they're never all in memory
        static constexpr auto BATCH_SIZE = 1uz << 16uz;

        static constexpr auto MAX_ERRORS_KEPT = 10uz;

        [[nodiscard]] auto is_full() const noexcept -> bool
        {
            return options.limit != 0uz and stats.accepted >= options.limit;
        }

        void process(const string_view rawLine)
        {
            ++stats.lines;

            const auto line = trim(rawLine);

            if (line.empty() or line.front() == '#') {
                ++stats.skipped;
                return;
            }

            auto parsed = parse_labelled_line(line);

            if (not parsed.has_value()) {
                ++stats.parseErrors;

                if (stats.firstErrors.size() < MAX_ERRORS_KEPT)
                    stats.firstErrors.emplace_back(std::format("line {}: {}", stats.lines, parsed.error()));

                return;
            }

            if (parsed->position.is_check()) {
                ++stats.inCheck;
                return;
            }

            if (not seen.insert(parsed->position.hash).second) {
                ++stats.duplicates;
                return;
            }

            ++stats.accepted;

            auto& batch = splitter(rng) < options.validationFraction ? validation : training;

            batch.push_back(std::move(*parsed));
        }

        void flush()
        {
            result.training.append(build_dataset(training, options.numThreads));
            result.validation.append(build_dataset(validation, options.numThreads));

            training.clear();
            validation.clear();
        }

        LoadOptions options;

        LoadStats& stats;

        std::unordered_set<Position::Hash> seen;

        std::mt19937_64 rng;

        std::uniform_real_distribution<double> splitter { 0., 1. };

        std::vector<LabelledPosition> training;

        std::vector<LabelledPosition> validation;

        SplitDataset result;
    };

} // namespace

auto parse_labelled_line(string_view line)
    -> std::expected<LabelledPosition, string>
{
    line = trim(line);

    const auto split = split_label(line);

    if (not split.has_value())
        return std::unexpected { split.error() };

    if (not split->result.has_value())
        return std::unexpected { string { "unrecognised result" } };

    const auto result = *split->result;

    if (not(result >= 0. and result <= 1.))
        return std::unexpected { std::format("result {} isn't in [0, 1]", result) };

    // keep the 4 FEN fields, plus the move counters if they're there; ignore anything else, such as EPD opcodes
    auto words = split_words(split->fen);

    for (auto& word : words)
        while (not word.empty() and (word.back() == ';' or word.back() == ','))
            word.remove_suffix(1uz);

    std::erase_if(words, [](const string_view word) { return word.empty(); });

    static constexpr auto MIN_FEN_FIELDS = 4uz;

    if (words.size() < MIN_FEN_FIELDS)
        return std::unexpected { string { "too few FEN fields" } };

    auto numFields = MIN_FEN_FIELDS;

    if (words.size() >= MIN_FEN_FIELDS + 2uz and is_integer(words.at(4uz)) and is_integer(words.at(5uz)))
        numFields += 2uz;

    string fen;

    for (auto i = 0uz; i < numFields; ++i) {
        if (i > 0uz)
            fen.push_back(' ');

        fen.append(words.at(i));
    }

    auto position = chess::notation::from_fen(fen);

    if (not position.has_value())
        return std::unexpected { std::format("invalid FEN '{}': {}", fen, position.error()) };

    return LabelledPosition { .position = std::move(*position), .result = result };
}

auto parse_line_score(const string_view line) -> std::optional<int>
{
    const auto close = line.find(']');

    if (close == string_view::npos)
        return std::nullopt;

    const auto text = trim(line.substr(close + 1uz));

    if (text.empty())
        return std::nullopt;

    int score { 0 };

    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), score);

    if (error != std::errc { } or end != text.data() + text.size())
        return std::nullopt;

    return score;
}

auto format_labelled_line(const Position& position, const double result, const std::optional<int> score)
    -> string
{
    auto line = std::format("{} [{:.1f}]", chess::notation::to_fen(position), result);

    if (score.has_value())
        line.append(std::format(" {}", *score));

    return line;
}

void Dataset::append(const Dataset& other)
{
    const auto offset = tuples.size();

    if (offset + other.tuples.size() > std::numeric_limits<std::uint32_t>::max())
        throw std::runtime_error { "Too many coefficients for one dataset" };

    tuples.insert(tuples.end(), other.tuples.begin(), other.tuples.end());

    entries.reserve(entries.size() + other.entries.size());

    for (auto entry : other.entries) {
        entry.firstTuple += static_cast<std::uint32_t>(offset);
        entries.push_back(entry);
    }
}

auto build_dataset(const std::span<const LabelledPosition> positions, const size_t numThreads)
    -> Dataset
{
    std::vector<Dataset> parts(std::max(1uz, numThreads));

    parallel_for(numThreads, positions.size(), [&parts, positions](const size_t thread, const size_t begin, const size_t end) {
        auto& part = parts.at(thread);

        for (auto i = begin; i < end; ++i)
            trace_into(part, positions[i]);
    });

    // the parts are joined in thread order, so the dataset is in the same order as the positions
    Dataset dataset;

    for (const auto& part : parts)
        dataset.append(part);

    return dataset;
}

auto load_dataset(std::istream& input, const LoadOptions& options, LoadStats& stats)
    -> SplitDataset
{
    Loader loader { options, stats };

    loader.read(input);

    return loader.finish();
}

auto load_dataset(
    const std::span<const std::filesystem::path> files, const LoadOptions& options, LoadStats& stats)
    -> SplitDataset
{
    Loader loader { options, stats };

    for (const auto& path : files) {
        std::ifstream input { path };

        if (not input.is_open())
            throw std::runtime_error { std::format("Can't open dataset file '{}'", path.string()) };

        loader.read(input);
    }

    return loader.finish();
}

} // namespace ben_bot::tuner
