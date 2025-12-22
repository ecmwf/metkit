/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

namespace pv_detail {


/**
 * @brief Fixed-size hexadecimal representation of an IEEE754 double value.
 *
 * This alias represents the raw byte layout of a double-precision floating-point
 * value encoded as 8 bytes. It is used to store predefined floating-point
 * constants in a portable, byte-exact form, independent of the host endianness.
 *
 * The interpretation of the byte order is handled explicitly by decoding
 * utilities elsewhere in the code.
 */
using HexDouble = std::array<uint8_t, 8>;

namespace data {
/**
 * @brief Demonstration PV table for a single PV coefficient.
 *
 * This table contains a minimal, hard-coded set of PV coefficients intended
 * solely for demonstration and development purposes.
 *
 * @warning
 *   The values stored in this table are NOT representative of real production
 *   PV data. Proper, scientifically validated PV tables must be provided
 *   before this code is used in a production environment.
 *
 * @note
 *   Real PV tables are expected to be significantly larger. For this reason,
 *   production-ready tables should be defined in a dedicated header file
 *   (or set of headers) that can be generated, maintained, and updated
 *   independently of the decoding logic.
 *
 * @note
 *   This table is encoded using big-endian IEEE754 double-precision format.
 */
static constexpr std::array<HexDouble, 1> pv_1_be = {{
    {0x40, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // 10.0
}};

/**
 * @brief Demonstration PV table for three PV coefficients.
 *
 * This table contains a small, illustrative set of PV coefficients used only
 * to exercise and validate the PV lookup and decoding infrastructure.
 *
 * @warning
 *   The data contained here is purely demonstrative. Real PV coefficient
 *   tables must be supplied separately and are expected to be much larger
 *   and derived from authoritative sources.
 *
 * @note
 *   Due to the expected size of real PV tables, the final implementation
 *   should place such data in a separate include file, keeping the core
 *   lookup and decoding logic independent from the bulk data definitions.
 *
 * @note
 *   All coefficients are encoded as big-endian IEEE754 double-precision values.
 */
static constexpr std::array<HexDouble, 3> pv_3_be = {{
    {0x40, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 10.0
    {0x40, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 20.0
    {0x40, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}   // 30.0
}};

}  // namespace data

/**
 * @brief Entry describing a predefined PV coefficient table.
 *
 * This structure defines the metadata and storage required to associate a
 * logical lookup key with a statically defined array of PV coefficients.
 *
 * Members:
 *  - `key`  : Logical identifier used to select the PV table (e.g. value
 *             provided by the caller). This key is not required to match the
 *             number of coefficients stored.
 *  - `size` : Number of PV coefficients available in the referenced data array.
 *  - `data` : Pointer to the first element of a statically allocated array of
 *             `HexDouble` values encoding the PV coefficients.
 *
 * The pointer stored in `data` is non-owning and is expected to reference
 * memory with static storage duration.
 */
struct PvEntry {
    long key;
    long size;
    const HexDouble* data;
};


/**
 * @brief Compile-time lookup table mapping PV logical keys to predefined PV data arrays.
 *
 * This table defines the association between a logical PV lookup key and a
 * statically defined array of PV coefficients encoded as hexadecimal
 * IEEE754 double-precision values.
 *
 * Each entry in the table specifies:
 *  - a logical lookup key (`PvEntry::key`) used to identify the PV array,
 *  - the number of PV coefficients stored in the array (`PvEntry::size`),
 *  - a pointer to the first element of the corresponding static data array.
 *
 * The data arrays referenced by this table are expected to:
 *  - have static storage duration,
 *  - contain exactly `size` elements,
 *  - store each coefficient as an 8-byte IEEE754 double in big-endian byte order.
 *
 * This table is defined as `constexpr` to ensure zero runtime initialization
 * overhead and to guarantee that the mapping is immutable.
 *
 * @todo [owner: mival,dgov][scope: deduction][reason: completeness][prio: low]
 * - Extend this table with additional PV arrays as required by the
 *   application domain.
 *
 * @note
 *   The logical lookup key (`key`) is not required to match the number of
 *   coefficients (`size`). This allows decoupling of external identifiers
 *   from the physical layout of the PV data.
 *
 * @note
 *   The lifetime of the referenced data arrays is guaranteed for the entire
 *   program execution, and the pointers stored in this table are non-owning.
 *
 * @note
 *   Data that are currently injected in the table are for demonstration and
 *   testing purposes only.
 */
constexpr std::array<PvEntry, 2> pv_tables = {{{1, 1, data::pv_1_be.data()}, {3, 3, data::pv_3_be.data()}}};

/**
 * @brief Decode a double value from an 8-byte sequence in native byte order.
 *
 * This helper function converts a sequence of 8 bytes into a native
 * double-precision floating-point value by copying the raw byte representation
 * directly into a `double` object.
 *
 * The conversion is performed using `std::memcpy` to avoid strict-aliasing
 * violations and undefined behavior associated with pointer reinterpreting.
 *
 * @param[in] p
 *   Pointer to an array of 8 bytes representing an IEEE754 double-precision
 *   floating-point value in the host native byte order.
 *
 * @return
 *   The decoded double-precision floating-point value.
 *
 * @note
 *   This function assumes:
 *   - `sizeof(double) == 8`
 *   - IEEE754 binary64 floating-point format
 *
 * @note
 *   The caller is responsible for ensuring that `p` points to at least
 *   `sizeof(double)` valid bytes.
 */
inline double bytes_to_double(const uint8_t* p) {
    double v;
    std::memcpy(&v, p, sizeof(double));
    return v;
}

/**
 * @brief Decode a double value from an 8-byte sequence with reversed byte order.
 *
 * This helper function converts an array of 8 bytes into a native
 * double-precision floating-point value, first reversing the byte order.
 * It is intended to handle conversion from a byte sequence whose endianness
 * differs from the host representation.
 *
 * The input byte sequence is copied into a temporary buffer with its byte
 * order reversed, after which it is decoded using `bytes_to_double()`.
 *
 * @param[in] p
 *   Pointer to an array of 8 bytes representing an IEEE754 double-precision
 *   floating-point value in non-native byte order.
 *
 * @return
 *   The decoded double-precision floating-point value in native representation.
 *
 * @note
 *   This function assumes:
 *   - `sizeof(double) == 8`
 *   - IEEE754 binary64 floating-point format
 *
 * @note
 *   No validation of the input byte sequence is performed. The caller is
 *   responsible for ensuring that `p` points to at least 8 valid bytes.
 */
inline double bytes_to_double_swapped(const uint8_t* p) {
    uint8_t tmp[8];
    std::reverse_copy(p, p + 8, tmp);
    return bytes_to_double(tmp);
}


/**
 * @brief Determine the host byte order for double-precision floating-point values.
 *
 * This function detects the native endianness of the host system by interpreting
 * a known IEEE754 double-precision value encoded in big-endian byte order.
 * The detection is performed by decoding the sentinel byte sequence both with
 * native byte order and with reversed byte order, and comparing the results
 * against the known reference value.
 *
 * The function returns whether the host system uses little-endian byte order
 * for `double` values. If neither interpretation matches the reference value,
 * the host platform is assumed to be unsupported (e.g. non-IEEE754 or unexpected
 * `double` layout), and an exception is thrown.
 *
 * @return
 *   `true` if the host uses little-endian representation for `double`,
 *   `false` if the host uses big-endian representation.
 *
 * @throws Mars2GribDeductionException
 *   If the host floating-point representation is incompatible with IEEE754
 *   binary64 or cannot be reliably interpreted.
 *
 * @note
 *   This function assumes:
 *   - `sizeof(double) == 8`
 *   - IEEE754 binary64 floating-point format
 *
 * @note
 *   The detection is runtime-based and should typically be executed once and
 *   cached, as the result is invariant for the lifetime of the process.
 */
inline bool host_is_little_endian_or_throw() {

    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    constexpr double sentinel                    = 1.23456789;
    constexpr std::array<uint8_t, 8> sentinel_be = {0x3F, 0xF3, 0xC0, 0xCA, 0x42, 0x83, 0xDE, 0x1B};

    const double v0 = bytes_to_double(sentinel_be.data());
    if (v0 == sentinel)
        return false;  // host BE

    const double v1 = bytes_to_double_swapped(sentinel_be.data());
    if (v1 == sentinel)
        return true;  // host LE

    throw Mars2GribDeductionException("Unsupported floating-point representation (non IEEE754 double?)", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

/**
 * @brief Decode a double value from an 8-byte hexadecimal representation, with optional byte swapping.
 *
 * This helper function converts a fixed-size array of 8 bytes representing an
 * IEEE754 double-precision floating-point value into a native `double`.
 * The input byte sequence is interpreted either directly or with reversed byte
 * order depending on the `swap` flag.
 *
 * The function is typically used when decoding statically defined, big-endian
 * hexadecimal tables on hosts whose native endianness may differ.
 *
 * @param[in] p
 *   Array of 8 bytes containing the raw IEEE754 representation of a double value.
 *
 * @param[in] swap
 *   If `true`, the byte order of `p` is reversed before decoding. If `false`,
 *   the byte order is used as-is.
 *
 * @return
 *   The decoded double-precision floating-point value in native representation.
 *
 * @note
 *   This function assumes:
 *   - `sizeof(double) == 8`
 *   - IEEE754 binary64 floating-point format
 *
 * @note
 *   No validation of the floating-point representation is performed here; the
 *   correctness of the input bytes is assumed.
 */
inline double read_double_maybe_swapped(const std::array<uint8_t, 8>& p, bool swap) {
    return swap ? bytes_to_double_swapped(p.data()) : bytes_to_double(p.data());
}

/**
 * @brief Lookup and decode a predefined PV coefficient array from its logical size.
 *
 * This function performs a lookup in a compile-time table of predefined PV
 * coefficient arrays using the provided logical key (`pvArraySize`). Each table
 * entry maps a logical key to a statically-defined array of IEEE754 double values
 * encoded as big-endian hexadecimal bytes.
 *
 * The function:
 * 1. Searches the PV lookup table for an entry whose key matches `pvArraySize`.
 * 2. Determines the host endianness at runtime using a sentinel-based check.
 * 3. Decodes the corresponding hexadecimal byte arrays into native `double`
 *    values, applying byte-swapping if required.
 * 4. Returns the decoded PV coefficients as a `std::vector<double>`.
 *
 * If the lookup fails or the host floating-point representation is unsupported,
 * a domain-specific exception is thrown. All errors are wrapped using
 * `std::throw_with_nested` to preserve the full error context.
 *
 * @param[in] pvArraySize
 *   Logical lookup key identifying the PV array to retrieve. This key does not
 *   necessarily correspond to the number of coefficients stored in the array.
 *
 * @return
 *   A vector containing the decoded PV coefficients associated with the given
 *   lookup key.
 *
 * @throws Mars2GribDeductionException
 *   - If no PV array is associated with the provided lookup key.
 *   - If the host floating-point representation is unsupported (non-IEEE754 or
 *     incompatible endianness).
 *   - If any error occurs during lookup or decoding; in this case, the original
 *     exception is preserved via nested exceptions.
 *
 * @note
 *   The PV tables are assumed to be stored in big-endian IEEE754 format.
 *   Host endianness is detected at runtime and decoding is adapted accordingly.
 *
 * @note
 *   This function is intended for use at API or backend boundaries and follows
 *   a fail-fast strategy with rich error context propagation.
 */
std::vector<double> loopUp_PvArrayFromSize_or_throw(long pvArraySize) {

    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        // 1) Lookup
        const PvEntry* entry = nullptr;
        for (const auto& e : pv_tables) {
            if (e.key == pvArraySize) {
                entry = &e;
                break;
            }
        }

        // Not found
        if (!entry) {
            std::string errMsg = "No PV array found for size: " + std::to_string(pvArraySize);
            errMsg += ". Supported sizes are: {1,3} only for debug reasons.";
            throw Mars2GribDeductionException(errMsg, Here());
        }

        // 2) Sentinel endian detection (need to be done only once, hence `static`)
        static const bool swap = host_is_little_endian_or_throw();

        // 3) Decode
        std::vector<double> out;
        out.reserve(entry->size);

        for (long i = 0; i < entry->size; ++i) {
            out.push_back(read_double_maybe_swapped(entry->data[i], swap));
        }

        // RReturn decoded sentinel
        return out;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribDeductionException("Unable to lookup PV array from size", Here()));
    }

    // Remove compiler warning
    __builtin_unreachable();
}

/**
 * @brief Convert a vector of double values into big-endian hexadecimal form.
 *
 * This function converts each input double into its IEEE754 binary64
 * representation encoded in big-endian byte order, independently of the
 * host native endianness.
 *
 * @param[in] values
 *   Vector of double-precision floating-point values.
 *
 * @return
 *   Vector of HexDouble values encoded in big-endian byte order.
 *
 * @throws Mars2GribDeductionException
 *   If the host floating-point representation is unsupported.
 */
inline std::vector<HexDouble> to_hex_double_be(const std::vector<double>& values) {

    static_assert(sizeof(double) == 8, "Unsupported double size");

    // Determine host endian once
    static const bool host_is_le = host_is_little_endian_or_throw();

    std::vector<HexDouble> out;
    out.reserve(values.size());

    for (double v : values) {
        uint8_t raw[8];
        std::memcpy(raw, &v, 8);

        HexDouble h{};

        if (host_is_le) {
            // native LE → write BE
            for (int i = 0; i < 8; ++i) {
                h[i] = raw[7 - i];
            }
        }
        else {
            // native BE → already BE
            for (int i = 0; i < 8; ++i) {
                h[i] = raw[i];
            }
        }

        out.push_back(h);
    }

    return out;
}

/**
 * @brief Generate a C++ include file containing a constexpr HexDouble table.
 *
 * This function writes a header file defining a `static constexpr`
 * `std::array<HexDouble, N>` initialized with the provided hexadecimal data.
 *
 * The generated file is intended to be included by the PV lookup
 * infrastructure and should contain data only (no logic).
 *
 * @param[in] hex_data
 *   Vector of HexDouble values to emit.
 *
 * @param[in] array_name
 *   Name of the generated C++ array (e.g. "pv_137_be").
 *
 * @param[in] filename
 *   Path to the output include file.
 *
 * @throws std::runtime_error
 *   If the output file cannot be created or written.
 */
inline void write_hex_table_include(const std::vector<HexDouble>& hex_data, const std::string& array_name,
                                    const std::string& filename) {

    std::ofstream os(filename);
    if (!os) {
        throw std::runtime_error("Cannot open output file: " + filename);
    }

    os << "#pragma once\n\n";
    os << "#include <array>\n";
    os << "#include <cstdint>\n\n";
    os << "using HexDouble = std::array<uint8_t, 8>;\n\n";
    os << "static constexpr std::array<HexDouble, " << hex_data.size() << "> " << array_name << " = {{\n";

    os << std::hex << std::setfill('0');

    for (const auto& h : hex_data) {
        os << "    {";
        for (size_t i = 0; i < 8; ++i) {
            os << "0x" << std::setw(2) << static_cast<int>(h[i]);
            if (i != 7)
                os << ", ";
        }
        os << "},\n";
    }

    os << "}};\n";
}

}  // namespace pv_detail


/**
 * @brief Resolve the GRIB `pv` (vertical coordinate parameters) array.
 *
 * This deduction resolves the GRIB `pv` array, which contains the vertical
 * coordinate parameters associated with hybrid or generalized vertical
 * coordinate systems.
 *
 * The array is obtained **exclusively** from the parameter dictionary (`par`)
 * under the key `pv`.
 *
 * Currently, only the direct provision of the full `pv` array is supported.
 * Resolution based on a declared size (`pvSize`) or indirect reconstruction
 * is **not implemented** and results in an error.
 *
 * @important
 * This function does not attempt to infer, generate, or validate the
 * contents of the PV array. It assumes that the provided values are already
 * consistent with the chosen vertical coordinate definition.
 *
 * @tparam MarsDict_t Type of the MARS dictionary (unused)
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary (unused)
 * @param[in] par  Parameter dictionary; must contain the key `pv`
 * @param[in] opt  Options dictionary (unused)
 *
 * @return A vector of doubles representing the GRIB `pv` array
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - the key `pv` is missing from the parameter dictionary
 *         - `pvSize` is provided without a corresponding `pv` array
 *         - reconstruction of the PV array from size metadata is requested
 *         - any unexpected error occurs during deduction
 *
 * @note
 * - This deduction does not rely on any pre-existing GRIB header state.
 * - No validation is performed on the size or semantic correctness of
 *   the returned PV array.
 *
 * @todo [owner: mival,dgov][scope: deduction][reason: completeness][prio: medium]
 * - Implement reconstruction or lookup of the PV array when only
 *   `pvSize` is provided.
 * - Add optional consistency checks between the PV array size and
 *   the associated vertical coordinate definition.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::vector<double> resolve_PvArray_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Checks for presence of keys
        bool hasPV     = has(par, "pv");
        bool hasPVSize = has(par, "pvSize");

        std::vector<double> pvArrayVal;

        if (hasPV) {
            // Get the pv array directly
            pvArrayVal = get_or_throw<std::vector<double>>(par, "pv");
            // Logging of the par::lengthOfTimeWindow
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "pvArray: looked up from Par dictionary with size: ";
                logMsg += std::to_string(pvArrayVal.size());
                return logMsg;
            }());
        }
        else if (!hasPV && hasPVSize) {

            // Get the pvArray size for lookup
            long pvArraySize = get_or_throw<long>(par, "pvSize");

            // Lookup of the pv array from size not implemented
            pvArrayVal = pv_detail::loopUp_PvArrayFromSize_or_throw(pvArraySize);

            // Logging of the par::lengthOfTimeWindow
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "pvArray: look up of Pv array from a map still not supported: ";
                logMsg += std::to_string(pvArrayVal.size());
                return logMsg;
            }());
        }
        else {
            throw Mars2GribDeductionException("Unable to lookup pv array", Here());
        }

        // Exit with success
        return pvArrayVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to deduce `pvArray`", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions
