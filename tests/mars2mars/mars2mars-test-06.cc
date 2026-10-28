// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// Tests for the timespan of the "since previous post-processing" params (10fg, mx2t, mn2t, mxtpr, mntpr):
/// on a single step the window comes from the output schedule; a step range keeps its own window; step 0
/// has no window and the field is flagged for dropping; accumulations keep `timespan=fs`. Also: any fixed
/// window (`<n>h`) at step 0 is flagged for dropping.

#include <string>
#include <utility>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"

#include "metkit/mars2mars/api/Mars2Mars.h"

namespace {

const std::vector<long> sinceLastPostProcessing{49, 201, 202, 228226, 228227};

eckit::LocalConfiguration field(const std::string& stream, const std::string& type, long param, long time) {
    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", stream);
    mars.set("type", type);
    mars.set("expver", "0001");
    mars.set("date", 20261013L);
    mars.set("time", time);
    mars.set("levtype", "sfc");
    mars.set("param", param);
    return mars;
}

eckit::LocalConfiguration operField(long param, long step, long time = 0L) {
    auto mars = field("oper", "fc", param, time);
    mars.set("step", step);
    return mars;
}

eckit::LocalConfiguration operFieldRange(long param, const std::string& stepRange) {
    auto mars = field("oper", "fc", param, 0L);
    mars.set("step", stepRange);
    return mars;
}

metkit::mars2mars::Mars2MarsResult<eckit::LocalConfiguration> convert(const eckit::LocalConfiguration& in) {
    return metkit::mars2mars::Mars2Mars{}.convert(in);
}

bool dropped(const metkit::mars2mars::Mars2MarsResult<eckit::LocalConfiguration>& result) {
    return result.misc.getBool("dropField", false);
}

}  // namespace

CASE("since-last-pp params on a single step: the window is the output interval (oper 00 UTC)") {
    const std::vector<std::pair<long, std::string>> expected{{1, "1h"},   {6, "1h"},   {90, "1h"},  {93, "3h"},
                                                             {144, "3h"}, {150, "6h"}, {240, "6h"}, {360, "6h"}};
    for (const long param : sinceLastPostProcessing) {
        for (const auto& e : expected) {
            const long step           = e.first;
            const std::string& window = e.second;
            const auto result         = convert(operField(param, step));
            EXPECT_EQUAL(result.mars.getString("timespan"), window);
            EXPECT_EQUAL(result.mars.getLong("step"), step);
            EXPECT(!dropped(result));
        }
    }
}

CASE("since-last-pp params: the params are mapped as before") {
    EXPECT_EQUAL(convert(operField(49, 6)).mars.getLong("param"), 237318L);
    EXPECT_EQUAL(convert(operField(201, 6)).mars.getLong("param"), 237167L);
    EXPECT_EQUAL(convert(operField(202, 6)).mars.getLong("param"), 238167L);
    EXPECT_EQUAL(convert(operField(228226, 6)).mars.getLong("param"), 237055L);
    EXPECT_EQUAL(convert(operField(228227, 6)).mars.getLong("param"), 238055L);
}

CASE("since-last-pp params: 12 UTC follows the same schedule") {
    EXPECT_EQUAL(convert(operField(201, 6, 1200L)).mars.getString("timespan"), "1h");
    EXPECT_EQUAL(convert(operField(201, 93, 1200L)).mars.getString("timespan"), "3h");
}

CASE("since-last-pp params: the 06 and 18 UTC runs stop at step 144") {
    for (const long time : {600L, 1800L}) {
        EXPECT_EQUAL(convert(operField(228226, 6, time)).mars.getString("timespan"), "1h");
        EXPECT_EQUAL(convert(operField(228226, 90, time)).mars.getString("timespan"), "1h");
        EXPECT_EQUAL(convert(operField(228226, 93, time)).mars.getString("timespan"), "3h");
        EXPECT_EQUAL(convert(operField(228226, 144, time)).mars.getString("timespan"), "3h");
        EXPECT_THROWS(convert(operField(228226, 150, time)));
    }
}

CASE("since-last-pp params: a step range keeps the window of the range (data path)") {
    EXPECT_EQUAL(convert(operFieldRange(228226, "5-6")).mars.getString("timespan"), "1h");
    EXPECT_EQUAL(convert(operFieldRange(228226, "90-93")).mars.getString("timespan"), "3h");
    EXPECT_EQUAL(convert(operFieldRange(201, "144-150")).mars.getString("timespan"), "6h");
}

CASE("since-last-pp params: step 0 has no window, the field is flagged for dropping") {
    for (const long param : sinceLastPostProcessing) {
        const auto result = convert(operField(param, 0));
        EXPECT(dropped(result));
    }
}

CASE("since-last-pp params: a step outside the schedule is an error, never fs") {
    EXPECT_THROWS(convert(operField(201, 91)));
    EXPECT_THROWS(convert(operField(201, 146)));
    EXPECT_THROWS(convert(operField(201, 366)));
}

CASE("since-last-pp params: a context without a schedule is an error, never fs") {
    auto enfo = field("enfo", "cf", 201, 0L);
    enfo.set("step", 6L);
    EXPECT_THROWS(convert(enfo));
}

CASE("accumulations since the start keep timespan=fs, including step 0") {
    for (const long param : {228L, 142L, 143L, 144L, 189L, 228216L}) {
        for (const long step : {0L, 6L, 93L}) {
            const auto result = convert(operField(param, step));
            EXPECT_EQUAL(result.mars.getString("timespan"), "fs");
            EXPECT(!dropped(result));
        }
    }
}

CASE("fixed-window params are unchanged") {
    EXPECT_EQUAL(convert(operField(121, 6)).mars.getString("timespan"), "6h");
    EXPECT_EQUAL(convert(operField(228026, 6)).mars.getString("timespan"), "3h");
    EXPECT_EQUAL(convert(operField(123, 6)).mars.getString("timespan"), "6h");
    EXPECT_EQUAL(convert(operField(228222, 6)).mars.getString("timespan"), "3h");
}

CASE("instantaneous params are unchanged") {
    const auto result = convert(operField(167, 6));
    EXPECT_EQUAL(result.mars.getString("timespan"), "none");
    EXPECT(!dropped(result));
}

CASE("fixed-window params at step 0 are flagged for dropping") {
    // 6 h, 3 h and 1 h windows: mx2t6, mn2t6, 10fg6, mx2t3, mn2t3, 10fg3, mxtpr3/6, mntpr3/6, mxcap6, mxcapes6,
    // litota1/3/6, ptype most frequent / most severe (1 h, 3 h, 6 h)
    for (const long param :
         {121L,    122L,    123L,    228026L, 228027L, 228028L, 228222L, 228223L, 228224L, 228225L, 228035L,
          228036L, 228051L, 228057L, 228058L, 260320L, 260321L, 260339L, 260318L, 260319L, 260338L}) {
        const auto atZero = convert(operField(param, 0));
        EXPECT(dropped(atZero));

        const auto atSix = convert(operField(param, 6));
        EXPECT(!dropped(atSix));
        EXPECT_EQUAL(atSix.mars.getString("timespan").back(), 'h');
    }
}

CASE("a step range ending after step 0 is kept (data path)") {
    const auto result = convert(operFieldRange(121, "0-6"));
    EXPECT_EQUAL(result.mars.getString("timespan"), "6h");
    EXPECT_EQUAL(result.mars.getLong("step"), 6L);
    EXPECT(!dropped(result));
}

CASE("a legacy ep step range on enfo is kept") {
    auto ep = field("enfo", "ep", 131060, 0L);
    ep.set("step", "0-24");
    const auto result = convert(ep);
    EXPECT_EQUAL(result.mars.getString("timespan"), "24h");
    EXPECT(!dropped(result));
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
