// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Simon Smart
/// @date May 2019

#pragma once

#include <algorithm>

#include "eckit/exception/Exceptions.h"
#include "metkit/mars/StepRange.h"

namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

// It would be nice to put this in StepRange (as is done with ParamID), but this seems
// to break the generation of the StepRange.b file.
//
// -> Split it out

class StepRangeNormalise {
public:

    template <typename AXIS_T>
    static void normalise(std::vector<StepRange>& v, const AXIS_T& axis);
};

//----------------------------------------------------------------------------------------------------------------------

template <typename AXIS_T>
void StepRangeNormalise::normalise(std::vector<StepRange>& values, const AXIS_T& axis) {

    std::vector<StepRange> outputValues;

    for (eckit::Ordinal i = 0; i < values.size(); ++i) {

        // If the supplied range is found in the axis, then use that

        auto j = std::find(axis.begin(), axis.end(), values[i]);
        if (j != axis.end()) {
            outputValues.push_back(values[i]);

            // If specified, and matched, a RANGE, then use that
            if (values[i].from() != values[i].to()) {
                eckit::Log::info() << "Matched range: " << values[i] << std::endl;
                continue;
            }
        }

        bool matched       = false;
        double singleValue = values[i].from();

        if (values[i].from() != values[i].to()) {
            j = std::find(axis.begin(), axis.end(), StepRange(singleValue, singleValue));
            if (j != axis.end()) {
                outputValues.push_back(*j);
                matched = true;
            }
        }

        // If singleValue == 0, this test is the same as the previous one...

        if (singleValue != 0) {
            j = std::find(axis.begin(), axis.end(), StepRange(0, singleValue));
            if (j != axis.end()) {

                if (matched) {
                    eckit::Log::userWarning()
                        << "Step " << values[i] << " matches " << values[i] << " and " << (*j) << std::endl;
                }

                outputValues.push_back(*j);
            }
        }
    }

    std::swap(values, outputValues);
}

//----------------------------------------------------------------------------------------------------------------------
}  // namespace mars
}  // namespace metkit
