/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// Record-level view of mars2grib test cases (block files or JSONL). For file-level
/// inspection of block files (info, stats, verify, select, convert) use `eckit-blockfile`.

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "eckit/blockfile/Filter.h"
#include "eckit/blockfile/Format.h"
#include "eckit/blockfile/Reader.h"
#include "eckit/blockfile/RecordReader.h"
#include "eckit/blockfile/Writer.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/utils/Tokenizer.h"

#include "metkit/mars2grib/testing-utils/FailureReport.h"
#include "metkit/mars2grib/testing-utils/Producer.h"
#include "metkit/mars2grib/testing-utils/RecordingDictionary.h"
#include "metkit/mars2grib/testing-utils/TestCaseLabels.h"
#include "metkit/mars2grib/testing-utils/TestRecord.h"
#include "metkit/tool/MetkitTool.h"

namespace {

using namespace metkit::mars2grib::testing_utils;
using namespace metkit::mars2grib::testing_utils::run_tests;

void printSection(std::ostream& out, const std::string& title, const std::string& body) {
    out << "---- " << title << '\n' << body;
    if (body.empty() || body.back() != '\n') {
        out << '\n';
    }
}

/// Pretty-print one test case: mars, misc and opt as indented JSON, out as one operation per line.
void printPretty(std::ostream& out, const std::string& record, std::size_t index, const std::string& location) {
    out << "==== test " << index << (location.empty() ? "" : " (" + location + ")") << " ====\n";
    try {
        const eckit::LocalConfiguration root = parseRecord(record, RecordParser::Json);
        for (const char* key : {"mars", "misc", "opt"}) {
            const auto section = optionalSubConfiguration(root, key);
            printSection(out, key,
                         section ? run_tests::detail::prettyJson(configurationToJson(*section)) : "(missing)");
        }
        if (const auto section = optionalSubConfiguration(root, "out")) {
            try {
                printSection(out, "out", run_tests::detail::formatOperations(RecordingDictionary{*section}));
            }
            catch (const std::exception&) {
                printSection(out, "out (not a valid recording)",
                             run_tests::detail::prettyJson(configurationToJson(*section)));
            }
        }
        else {
            printSection(out, "out", "(missing)");
        }
    }
    catch (const std::exception& e) {
        printSection(out, "unparsable record", std::string{e.what()} + "\n" + record);
    }
    out << '\n';
}

/// A CMake bracket argument, so values need no escaping.
std::string cmakeArgument(const std::string& value) {
    return "[==[" + value + "]==]";
}

/// A label value usable in a test name.
std::string nameComponent(const std::string& value) {
    std::string out;
    for (const char c : value) {
        out += std::isalnum(static_cast<unsigned char>(c)) ? c : '_';
    }
    return out.empty() ? "empty" : out;
}

std::vector<std::string> splitList(const std::string& text) {
    std::vector<std::string> items;
    eckit::Tokenizer(",")(text, items);
    return items;
}

/// Call `f(index, record)` for every record of a block file or JSONL file, in file order.
template <typename F>
void forEachRecord(const std::string& file, F&& f) {
    eckit::blockfile::RecordReader reader{eckit::PathName{file}};
    std::string record;
    std::size_t index = 0;
    while (reader.next(record)) {
        f(index++, record);
    }
}

/// The records at `indices` (file order, as forEachRecord counts them).
std::map<std::size_t, std::string> fetchRecords(const std::string& file, const std::set<std::size_t>& indices) {
    std::map<std::size_t, std::string> records;
    if (indices.empty()) {
        return records;
    }
    forEachRecord(file, [&](std::size_t index, const std::string& record) {
        if (indices.count(index) != 0) {
            records.emplace(index, record);
        }
    });
    return records;
}

/// Value spans [begin, end) of the top-level members of a JSON object, so single members can be
/// replaced without re-serialising (and possibly reformatting) the others.
std::map<std::string, std::pair<std::size_t, std::size_t>> topLevelMembers(std::string_view json) {
    std::map<std::string, std::pair<std::size_t, std::size_t>> members;
    std::size_t i         = 0;
    const auto skipSpaces = [&]() {
        while (i < json.size() && std::isspace(static_cast<unsigned char>(json[i]))) {
            ++i;
        }
    };
    const auto skipString = [&]() {  // json[i] == '"'
        for (++i; i < json.size(); ++i) {
            if (json[i] == '\\') {
                ++i;
            }
            else if (json[i] == '"') {
                ++i;
                return;
            }
        }
        throw eckit::BadValue("Unterminated string in test-case record", Here());
    };

    skipSpaces();
    if (i >= json.size() || json[i] != '{') {
        throw eckit::BadValue("Test-case record is not a JSON object", Here());
    }
    ++i;
    while (true) {
        skipSpaces();
        if (i < json.size() && json[i] == '}') {
            break;
        }
        if (i >= json.size() || json[i] != '"') {
            throw eckit::BadValue("Malformed test-case record (expected a key)", Here());
        }
        const std::size_t keyBegin = i + 1;
        skipString();
        const std::string key{json.substr(keyBegin, i - 1 - keyBegin)};
        skipSpaces();
        if (i >= json.size() || json[i] != ':') {
            throw eckit::BadValue("Malformed test-case record (expected ':')", Here());
        }
        ++i;
        skipSpaces();
        const std::size_t begin = i;
        int depth               = 0;
        while (i < json.size()) {
            const char c = json[i];
            if (c == '"') {
                skipString();
                continue;
            }
            if (c == '{' || c == '[') {
                ++depth;
            }
            else if (c == '}' || c == ']') {
                if (depth == 0) {
                    break;
                }
                --depth;
            }
            else if (c == ',' && depth == 0) {
                break;
            }
            ++i;
        }
        std::size_t end = i;
        while (end > begin && std::isspace(static_cast<unsigned char>(json[end - 1]))) {
            --end;
        }
        members[key] = {begin, end};
        if (i < json.size() && json[i] == ',') {
            ++i;
        }
    }
    return members;
}

struct ParsedTest {
    std::size_t index{0};
    eckit::LocalConfiguration mars;
    std::optional<RecordingDictionary> out;
    std::string outError;
};

class Mars2GribTestCasesTool final : public metkit::MetkitTool {
public:

    Mars2GribTestCasesTool(int argc, char** argv) : MetkitTool(argc, argv) {
        using eckit::option::SimpleOption;
        options_.push_back(new SimpleOption<long>("index", "get: index of the test (0-based, in file order)"));
        options_.push_back(new SimpleOption<bool>("pretty", "cat: pretty-print every test"));
        options_.push_back(new SimpleOption<std::string>(
            "select", "cat (block files): only blocks matching e.g. class=od,stream=enfo/oper,type=pf"));
        options_.push_back(new SimpleOption<std::string>("blocks", "cat (block files): only blocks e.g. 0,3,5-7"));
        options_.push_back(new SimpleOption<std::string>(
            "keys",
            "relabel: comma-separated block keys (default class,stream,type); timeShape is the product time "
            "spec shape type, any other name a MARS key; absent values are labelled 'missing'"));
        options_.push_back(new SimpleOption<long>("records-per-block", "relabel: tests per block (default 1000)"));
        options_.push_back(
            new SimpleOption<long>("block-bytes", "relabel: uncompressed bytes per block (default 4 MiB)"));
        options_.push_back(new SimpleOption<long>("zstd-level", "relabel: zstd compression level 1..22 (default 9)"));
        options_.push_back(new SimpleOption<std::string>(
            "remove",
            "remove-keys: comma-separated keys to delete from every record, as section.key with section "
            "mars, misc or opt, e.g. misc.shapeOfTheEarth"));
        options_.push_back(new SimpleOption<long>(
            "details", "diff: number of changed/added/removed tests to show in detail (default 20, -1 = all)"));
        options_.push_back(new SimpleOption<std::string>(
            "runner", "ctest-list: path of mars2grib-run-tests-scalar used by the generated tests"));
        options_.push_back(new SimpleOption<std::string>("output", "ctest-list: CMake file to write"));
        options_.push_back(
            new SimpleOption<std::string>("modes", "ctest-list: comma-separated test modes (default encode)"));
        options_.push_back(
            new SimpleOption<long>("blocks-per-test",
                                   "ctest-list: split each label combination into tests of at most N blocks (0 = one "
                                   "test per combination, the default)"));
        options_.push_back(new SimpleOption<std::string>("prefix", "ctest-list: test-name prefix (default mars2grib)"));
        options_.push_back(new SimpleOption<std::string>(
            "reports-dir", "ctest-list: write failure reports under <dir>/<mode>/<test name>"));
        options_.push_back(new SimpleOption<long>("timeout", "ctest-list: TIMEOUT of each test in seconds"));
        options_.push_back(new SimpleOption<std::string>(
            "environment", "ctest-list: comma-separated VAR=VALUE set for each test (ENVIRONMENT property)"));
    }

private:

    int minimumPositionalArguments() const override { return 2; }
    int numberOfPositionalArguments() const override { return -1; }
    void execute(const eckit::option::CmdArgs& args) override;
    void usage(const std::string& tool) const override;

    void get(const std::string& file, std::size_t index) const;
    void cat(const std::string& file, const eckit::option::CmdArgs& args) const;
    void ctestList(const std::string& file, const eckit::option::CmdArgs& args) const;
    void relabel(const std::string& file, const eckit::option::CmdArgs& args) const;
    void removeKeys(const std::string& file, const eckit::option::CmdArgs& args) const;
    void diff(const std::string& oldFile, const std::string& newFile, const eckit::option::CmdArgs& args) const;
};

void Mars2GribTestCasesTool::execute(const eckit::option::CmdArgs& args) {
    const std::string command = args(0);
    const std::string file    = args(1);
    if (command == "get") {
        if (!args.has("index")) {
            throw eckit::UserError("get needs --index", Here());
        }
        long index = 0;
        args.get("index", index);
        if (index < 0) {
            throw eckit::UserError("--index must not be negative", Here());
        }
        get(file, static_cast<std::size_t>(index));
        return;
    }
    if (command == "cat") {
        cat(file, args);
        return;
    }
    if (command == "ctest-list") {
        ctestList(file, args);
        return;
    }
    if (command == "relabel") {
        relabel(file, args);
        return;
    }
    if (command == "remove-keys") {
        removeKeys(file, args);
        return;
    }
    if (command == "diff") {
        if (args.count() != 3) {
            throw eckit::UserError("diff needs two files: diff <old> <new>", Here());
        }
        diff(file, args(2), args);
        return;
    }
    throw eckit::UserError("Unknown command `" + command + "` (get, cat, ctest-list, relabel, remove-keys, diff)",
                           Here());
}

void Mars2GribTestCasesTool::get(const std::string& file, std::size_t index) const {
    if (eckit::blockfile::isBlockFile(eckit::PathName{file})) {
        const eckit::blockfile::Reader reader{eckit::PathName{file}};
        std::size_t first = 0;
        for (const auto& info : reader.blocks()) {
            if (index < first + info.records) {
                const auto records = reader.readBlock(info.id);
                const auto views   = records.records();
                printPretty(std::cout, std::string{views.at(index - first)}, index,
                            "block " + std::to_string(info.id) + ": " +
                                eckit::blockfile::formatLabels(reader.keys(), info.labels));
                return;
            }
            first += info.records;
        }
        throw eckit::UserError(
            "--index " + std::to_string(index) + " is beyond the " + std::to_string(first) + " tests in `" + file + "`",
            Here());
    }

    Producer producer{file};
    std::size_t current = 0;
    while (auto record = producer.readNextRecord()) {
        if (current == index) {
            printPretty(std::cout, *record, index, "");
            return;
        }
        ++current;
    }
    throw eckit::UserError(
        "--index " + std::to_string(index) + " is beyond the " + std::to_string(current) + " tests in `" + file + "`",
        Here());
}

void Mars2GribTestCasesTool::cat(const std::string& file, const eckit::option::CmdArgs& args) const {
    const bool pretty = args.getBool("pretty", false);

    if (!eckit::blockfile::isBlockFile(eckit::PathName{file})) {
        if (args.has("select") || args.has("blocks")) {
            throw eckit::UserError("--select and --blocks need a block-file input", Here());
        }
        Producer producer{file};
        std::size_t index = 0;
        while (auto record = producer.readNextRecord()) {
            if (pretty) {
                printPretty(std::cout, *record, index, "");
            }
            else {
                std::cout << *record << '\n';
            }
            ++index;
        }
        std::cout << std::flush;
        return;
    }

    const eckit::blockfile::Reader reader{eckit::PathName{file}};
    std::vector<std::size_t> ids = args.has("blocks") ? parseBlockIds(args.getString("blocks"), reader.numberOfBlocks())
                                                      : reader.select(eckit::blockfile::Filter{});
    if (args.has("select")) {
        const auto filter = eckit::blockfile::Filter::parse(args.getString("select"));
        ids.erase(std::remove_if(ids.begin(), ids.end(),
                                 [&](std::size_t id) { return !filter.matches(reader.keys(), reader.blocks()[id]); }),
                  ids.end());
    }

    // Global test index of the first record of every block.
    std::vector<std::size_t> firstIndex(reader.numberOfBlocks(), 0);
    for (std::size_t id = 1; id < reader.numberOfBlocks(); ++id) {
        firstIndex[id] = firstIndex[id - 1] + reader.blocks()[id - 1].records;
    }

    for (std::size_t id : ids) {
        const auto block = reader.readBlock(id);
        if (!pretty) {
            std::cout.write(block.data(), static_cast<std::streamsize>(block.size()));
            continue;
        }
        const std::string location =
            "block " + std::to_string(id) + ": " + eckit::blockfile::formatLabels(reader.keys(), block.info().labels);
        std::size_t index = firstIndex[id];
        block.forEachRecord(
            [&](std::string_view record) { printPretty(std::cout, std::string{record}, index++, location); });
    }
    std::cout << std::flush;
}

void Mars2GribTestCasesTool::ctestList(const std::string& file, const eckit::option::CmdArgs& args) const {
    if (!args.has("runner") || !args.has("output")) {
        throw eckit::UserError("ctest-list needs --runner and --output", Here());
    }
    const eckit::blockfile::Reader reader{eckit::PathName{file}};
    const std::string testCases = eckit::PathName{file}.realName().asString();
    const std::string runner    = args.getString("runner");
    const std::string prefix    = args.getString("prefix", "mars2grib");
    const long blocksPerTest    = args.getLong("blocks-per-test", 0);
    const long timeout          = args.getLong("timeout", 0);
    if (blocksPerTest < 0 || timeout < 0) {
        throw eckit::UserError("--blocks-per-test and --timeout must not be negative", Here());
    }
    std::vector<std::string> modes = splitList(args.getString("modes", "encode"));
    for (const auto& mode : modes) {
        testModeFromString(mode);  // validate
    }
    const std::vector<std::string> environment = splitList(args.getString("environment", ""));

    // Blocks of every label combination, in file order.
    std::map<eckit::blockfile::Labels, std::vector<std::size_t>> combinations;
    for (const auto& block : reader.blocks()) {
        combinations[block.labels].push_back(block.id);
    }

    std::ofstream out{args.getString("output"), std::ios::out | std::ios::trunc};
    if (!out) {
        throw eckit::UserError("Unable to open `" + args.getString("output") + "`", Here());
    }
    out << "# Generated by mars2grib-testcases ctest-list from " << testCases << " - do not edit.\n"
        << "# " << reader.numberOfBlocks() << " blocks, " << reader.header().totalRecords << " tests, "
        << combinations.size() << " label combinations.\n\n";

    std::size_t tests = 0;
    for (const auto& [labels, ids] : combinations) {
        std::string name;
        std::string select;
        std::vector<std::string> labelList;
        bool selectable = true;
        for (std::size_t k = 0; k < reader.keys().size(); ++k) {
            const std::string value = eckit::blockfile::toString(labels[k]);
            name += "_" + nameComponent(value);
            select += (k == 0 ? "" : ",") + reader.keys()[k].name + "=" + value;
            labelList.push_back(reader.keys()[k].name + "=" + value);
            selectable = selectable && value.find_first_of(",/;") == std::string::npos && !value.empty();
        }

        const std::size_t chunk  = blocksPerTest == 0 ? ids.size() : static_cast<std::size_t>(blocksPerTest);
        const std::size_t chunks = (ids.size() + chunk - 1) / chunk;
        for (std::size_t c = 0; c < chunks; ++c) {
            // A whole combination is selected by its labels; chunks (or awkward values) by block ids.
            std::string blocks;
            for (std::size_t i = c * chunk; i < std::min(ids.size(), (c + 1) * chunk); ++i) {
                blocks += (blocks.empty() ? "" : ",") + std::to_string(ids[i]);
            }
            std::string suffix;
            if (chunks > 1) {
                std::ostringstream os;
                os << '_' << std::setw(3) << std::setfill('0') << c;
                suffix = os.str();
            }

            for (const auto& mode : modes) {
                // One block per test: name and label the test by its block id.
                const bool perBlock = chunk == 1;
                std::string blockTag;
                if (perBlock) {
                    std::ostringstream os;
                    os << "_b" << std::setw(5) << std::setfill('0') << ids[c];
                    blockTag = os.str();
                }
                const std::string test = prefix + "_" + mode + blockTag + name + (perBlock ? "" : suffix);
                out << "add_test(" << cmakeArgument(test) << ' ' << cmakeArgument(runner) << ' '
                    << cmakeArgument("--test-cases") << ' ' << cmakeArgument(testCases) << ' '
                    << cmakeArgument("--mode") << ' ' << cmakeArgument(mode);
                if (chunks == 1 && selectable && !perBlock) {
                    out << ' ' << cmakeArgument("--select") << ' ' << cmakeArgument(select);
                }
                else {
                    out << ' ' << cmakeArgument("--blocks") << ' ' << cmakeArgument(blocks);
                }
                if (args.has("reports-dir")) {
                    out << ' ' << cmakeArgument("--failure-reports") << ' ' << cmakeArgument("--failure-reports-dir")
                        << ' ' << cmakeArgument(args.getString("reports-dir") + "/" + mode + "/" + test);
                }
                out << ")\n";

                std::string labelsProperty = prefix + ";" + prefix + "-" + mode;
                for (const auto& label : labelList) {
                    labelsProperty += ";" + label;
                }
                if (perBlock) {
                    labelsProperty += ";block=" + std::to_string(ids[c]);
                }
                out << "set_tests_properties(" << cmakeArgument(test) << " PROPERTIES LABELS "
                    << cmakeArgument(labelsProperty);
                if (timeout > 0) {
                    out << " TIMEOUT " << timeout;
                }
                if (!environment.empty()) {
                    std::string env;
                    for (const auto& item : environment) {
                        env += (env.empty() ? "" : ";") + item;
                    }
                    out << " ENVIRONMENT " << cmakeArgument(env);
                }
                out << ")\n";
                ++tests;
            }
        }
    }

    out.close();
    if (!out) {
        throw eckit::Exception("Unable to write `" + args.getString("output") + "`", Here());
    }
    eckit::Log::info() << "Wrote " << tests << " ctests for " << combinations.size() << " label combinations to "
                       << args.getString("output") << std::endl;
}

void Mars2GribTestCasesTool::relabel(const std::string& file, const eckit::option::CmdArgs& args) const {
    if (!args.has("output")) {
        throw eckit::UserError("relabel needs --output", Here());
    }
    const std::string output = args.getString("output");
    if (eckit::PathName{output}.fullName() == eckit::PathName{file}.fullName()) {
        throw eckit::UserError("--output must differ from the input", Here());
    }

    const TestCaseLabeler labeler{args.getString("keys", "class,stream,type")};
    eckit::blockfile::WriterOptions options;
    options.keys             = labeler.keys();
    options.recordsPerBlock  = static_cast<std::size_t>(args.getLong("records-per-block", 1000));
    options.bytesPerBlock    = static_cast<std::size_t>(args.getLong("block-bytes", 4 * 1024 * 1024));
    options.compressionLevel = static_cast<int>(args.getLong("zstd-level", 9));
    if (options.compressionLevel < 1 || options.compressionLevel > 22) {
        throw eckit::UserError("--zstd-level must be in the range 1..22", Here());
    }

    eckit::blockfile::Writer writer{eckit::PathName{output}, options};
    std::size_t tests = 0;
    std::vector<std::size_t> missing(options.keys.size(), 0);
    forEachRecord(file, [&](std::size_t, const std::string& record) {
        const eckit::LocalConfiguration root = parseRecord(record, RecordParser::Json);
        const eckit::LocalConfiguration mars = requiredSubConfiguration(root, "mars");
        const auto misc   = optionalSubConfiguration(root, "misc").value_or(eckit::LocalConfiguration{});
        const auto opt    = optionalSubConfiguration(root, "opt").value_or(eckit::LocalConfiguration{});
        const auto labels = labeler.labels(mars, misc, opt);
        for (std::size_t k = 0; k < labels.size(); ++k) {
            if (std::get<std::string>(labels[k]) == missingLabel) {
                ++missing[k];
            }
        }
        writer.add(labels, record);
        if (++tests % 1000 == 0) {
            std::cerr << "\rRELABELLED: " << tests << std::flush;
        }
    });
    writer.close();
    std::cerr << "\r" << std::string(30, ' ') << '\r';

    eckit::Log::info() << "Wrote " << tests << " tests in " << writer.numberOfBlocks() << " blocks to " << output
                       << '\n';
    for (std::size_t k = 0; k < options.keys.size(); ++k) {
        eckit::Log::info() << "  " << options.keys[k].name << ": " << missing[k] << " tests labelled missing\n";
    }
    eckit::Log::info() << "Inspect with: eckit-blockfile stats " << output << std::endl;
}

void Mars2GribTestCasesTool::removeKeys(const std::string& file, const eckit::option::CmdArgs& args) const {
    if (!args.has("output") || !args.has("remove")) {
        throw eckit::UserError("remove-keys needs --output and --remove", Here());
    }
    const std::string output = args.getString("output");
    if (eckit::PathName{output}.fullName() == eckit::PathName{file}.fullName()) {
        throw eckit::UserError("--output must differ from the input", Here());
    }
    if (!eckit::blockfile::isBlockFile(eckit::PathName{file})) {
        throw eckit::UserError("remove-keys needs a block-file input", Here());
    }
    const int level = static_cast<int>(args.getLong("zstd-level", 9));
    if (level < 1 || level > 22) {
        throw eckit::UserError("--zstd-level must be in the range 1..22", Here());
    }

    // section.key pairs
    std::vector<std::pair<std::string, std::string>> keys;
    for (const auto& item : splitList(args.getString("remove"))) {
        const auto dot            = item.find('.');
        const std::string section = item.substr(0, dot);
        if (dot == std::string::npos || dot + 1 == item.size() ||
            (section != "mars" && section != "misc" && section != "opt")) {
            throw eckit::UserError("--remove entry `" + item + "` must be mars.<key>, misc.<key> or opt.<key>", Here());
        }
        keys.emplace_back(section, item.substr(dot + 1));
    }

    // Same blocks, same labels, same order: only the records change.
    const eckit::blockfile::Reader reader{eckit::PathName{file}};
    eckit::blockfile::WriterOptions options;
    options.keys             = reader.keys();
    options.compressionLevel = level;
    eckit::blockfile::Writer writer{eckit::PathName{output}, options};

    std::vector<std::size_t> removed(keys.size(), 0);
    std::size_t tests = 0;
    for (const auto& info : reader.blocks()) {
        const auto block = reader.readBlock(info.id);
        std::string records;
        std::uint32_t count = 0;
        block.forEachRecord([&](std::string_view view) {
            // Re-serialise only the sections that change; everything else (notably `out`) stays byte-identical.
            const auto members = topLevelMembers(view);
            std::map<std::size_t, std::pair<std::size_t, std::string>> replacements;  // begin -> (end, text)
            std::map<std::string, eckit::LocalConfiguration> edited;
            for (std::size_t k = 0; k < keys.size(); ++k) {
                const auto& [section, key] = keys[k];
                const auto member          = members.find(section);
                if (member == members.end()) {
                    continue;
                }
                auto it = edited.find(section);
                if (it == edited.end()) {
                    const auto [begin, end] = member->second;
                    it =
                        edited.emplace(section, parseRecord(view.substr(begin, end - begin), RecordParser::Json)).first;
                }
                if (it->second.has(key)) {
                    it->second.remove(key);
                    ++removed[k];
                    replacements[member->second.first] = {member->second.second, configurationToJson(it->second)};
                }
            }

            std::size_t position = 0;
            for (const auto& [begin, replacement] : replacements) {
                records.append(view.substr(position, begin - position));
                records += replacement.second;
                position = replacement.first;
            }
            records.append(view.substr(position));
            records += '\n';
            ++count;
        });
        writer.addCompressedBlock(eckit::blockfile::Writer::compressBlock(info.labels, records, count, level));
        tests += count;
        std::cerr << "\rBLOCKS: " << info.id + 1 << '/' << reader.numberOfBlocks() << std::flush;
    }
    writer.close();
    std::cerr << '\n';

    eckit::Log::info() << "Wrote " << tests << " tests in " << writer.numberOfBlocks() << " blocks to " << output
                       << '\n';
    for (std::size_t k = 0; k < keys.size(); ++k) {
        eckit::Log::info() << "  removed " << keys[k].first << '.' << keys[k].second << " from " << removed[k]
                           << " tests\n";
    }
    eckit::Log::info() << std::flush;
}

void Mars2GribTestCasesTool::diff(const std::string& oldFile, const std::string& newFile,
                                  const eckit::option::CmdArgs& args) const {
    const long detailsOption = args.getLong("details", 20);
    const std::size_t maxDetails =
        detailsOption < 0 ? std::numeric_limits<std::size_t>::max() : static_cast<std::size_t>(detailsOption);

    // 1. Identical records (by raw content) are unchanged and never parsed.
    std::unordered_map<std::uint64_t, std::vector<std::size_t>> oldByContent;
    std::size_t oldCount = 0;
    forEachRecord(oldFile, [&](std::size_t index, const std::string& record) {
        oldByContent[fnv1a(record)].push_back(index);
        ++oldCount;
    });

    std::set<std::size_t> newUnmatched;
    std::size_t newCount  = 0;
    std::size_t identical = 0;
    forEachRecord(newFile, [&](std::size_t index, const std::string& record) {
        ++newCount;
        auto it = oldByContent.find(fnv1a(record));
        if (it != oldByContent.end() && !it->second.empty()) {
            it->second.pop_back();
            ++identical;
        }
        else {
            newUnmatched.insert(index);
        }
    });
    std::set<std::size_t> oldUnmatched;
    for (const auto& [hash, indices] : oldByContent) {
        oldUnmatched.insert(indices.begin(), indices.end());
    }

    // 2. The remaining records are matched by test identity (mars + misc + opt).
    const auto parse = [](const std::map<std::size_t, std::string>& records) {
        std::map<std::uint64_t, ParsedTest> tests;
        for (const auto& [index, record] : records) {
            ParsedTest test;
            test.index                           = index;
            const eckit::LocalConfiguration root = parseRecord(record, RecordParser::Json);
            test.mars                            = requiredSubConfiguration(root, "mars");
            const auto misc = optionalSubConfiguration(root, "misc").value_or(eckit::LocalConfiguration{});
            const auto opt  = optionalSubConfiguration(root, "opt").value_or(eckit::LocalConfiguration{});
            try {
                if (const auto out = optionalSubConfiguration(root, "out")) {
                    test.out.emplace(*out);
                }
                else {
                    test.outError = "no `out`";
                }
            }
            catch (const std::exception& e) {
                test.outError = e.what();
            }
            tests[testCaseIdentity(test.mars, misc, opt)] = std::move(test);
        }
        return tests;
    };
    const auto oldTests = parse(fetchRecords(oldFile, oldUnmatched));
    const auto newTests = parse(fetchRecords(newFile, newUnmatched));

    std::vector<std::pair<const ParsedTest*, const ParsedTest*>> changed;
    std::size_t reformatted = 0;
    std::vector<const ParsedTest*> added;
    std::vector<const ParsedTest*> removed;
    for (const auto& [identity, test] : newTests) {
        const auto it = oldTests.find(identity);
        if (it == oldTests.end()) {
            added.push_back(&test);
        }
        else if (test.out && it->second.out && it->second.out->compare(*test.out).equal) {
            ++reformatted;  // same test and output, different text (e.g. key order)
        }
        else {
            changed.emplace_back(&it->second, &test);
        }
    }
    for (const auto& [identity, test] : oldTests) {
        if (newTests.find(identity) == newTests.end()) {
            removed.push_back(&test);
        }
    }

    std::ostream& out = std::cout;
    out << "old: " << oldFile << " (" << oldCount << " tests)\n"
        << "new: " << newFile << " (" << newCount << " tests)\n"
        << "  unchanged:   " << identical + reformatted
        << (reformatted ? " (" + std::to_string(reformatted) + " with different formatting)" : "")
        << "\n  changed:     " << changed.size() << " (same mars/misc/opt, different out)\n"
        << "  added:       " << added.size() << "\n  removed:     " << removed.size() << '\n';

    std::size_t shown = 0;
    for (const auto& [before, after] : changed) {
        if (shown++ >= maxDetails) {
            break;
        }
        out << "\n==== changed (old #" << before->index << ", new #" << after->index
            << "): " << configurationToJson(after->mars) << '\n';
        if (before->out && after->out) {
            out << run_tests::detail::diffOutputs(*before->out, *after->out);
        }
        else {
            out << "  out not comparable: old " << (before->out ? "ok" : before->outError) << ", new "
                << (after->out ? "ok" : after->outError) << '\n';
        }
    }
    for (const auto* test : added) {
        if (shown++ >= maxDetails) {
            break;
        }
        out << "\n==== added (new #" << test->index << "): " << configurationToJson(test->mars) << '\n';
    }
    for (const auto* test : removed) {
        if (shown++ >= maxDetails) {
            break;
        }
        out << "\n==== removed (old #" << test->index << "): " << configurationToJson(test->mars) << '\n';
    }
    const std::size_t differences = changed.size() + added.size() + removed.size();
    if (shown > maxDetails && differences > maxDetails) {
        out << "\n(" << differences - maxDetails << " more differences not shown; use --details -1 for all)\n";
    }
    out << std::flush;

    if (differences != 0) {
        throw eckit::Exception(std::to_string(differences) + " test differences between the files", Here());
    }
}

void Mars2GribTestCasesTool::usage(const std::string& tool) const {
    eckit::Log::info() << "Usage:\n"
                       << "  " << tool << " get <file> --index <N>\n"
                       << "  " << tool << " cat <file> [--pretty] [--select <filter>] [--blocks <ids>]\n"
                       << "  " << tool
                       << " relabel <file> --output <file.ekbf> [--keys class,stream,type,levtype,timeShape] "
                          "[--records-per-block <N>] [--block-bytes <N>] [--zstd-level <1..22>]\n"
                       << "  " << tool
                       << " remove-keys <file.ekbf> --output <file.ekbf> --remove misc.shapeOfTheEarth[,...] "
                          "[--zstd-level <1..22>]\n"
                       << "  " << tool << " diff <old> <new> [--details <N>]\n"
                       << "  " << tool
                       << " ctest-list <file> --runner <mars2grib-run-tests-scalar> --output <file.cmake> "
                          "[--modes encode,roundtrip] [--blocks-per-test <N>] [--prefix <name>] [--reports-dir <dir>] "
                          "[--timeout <s>] [--environment VAR=VALUE,...]\n"
                       << "<file> is a test-case block file or JSONL (zstd-compressed or plain).\n"
                       << "Use eckit-blockfile for info, stats, verify, select and convert." << std::endl;
}

}  // namespace

int main(int argc, char** argv) {
    Mars2GribTestCasesTool tool(argc, argv);
    return tool.start();
}
