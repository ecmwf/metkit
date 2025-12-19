#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/testing/Test.h"

#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"


CASE("Test generic get_or_throw<T> valid key") {

#define TEST_GET_OR_THROW_VALID(TYPE, TYPENAME_STRING, KEY, EXPECTED) \
    SECTION("get_or_throw<" TYPENAME_STRING "> valid key") {          \
        using metkit::mars2grib::utils::dict_traits::get_or_throw;    \
        EXPECT_NO_THROW({                                             \
            TYPE expected_result = (EXPECTED);                        \
            TYPE actual_result   = get_or_throw<TYPE>(cfg, (KEY));    \
            EXPECT_EQUAL(actual_result, expected_result);             \
        });                                                           \
    }

    // Used symbols
    using metkit::mars2grib::utils::dict_traits::get_or_throw;

    // Prepare a custom dictionary to test long
    const std::string yaml(R"json({
 bool_scalar_var: true,
 int_scalar_var: 7,
 long_scalar_var: 12,
 float_scalar_var: 3.14,
 double_scalar_var: 3.14,
 string_scalar_var: "abc",
 int_vec_var: [7,6],
 long_vec_var: [12,13],
 float_vec_var: [3.14, 2.71],
 double_vec_var: [3.14, 2.71],
 string_vec_var: ["abc", "def"]
 })json");


    // Initialize the configuration
    const eckit::YAMLConfiguration root(yaml);
    eckit::LocalConfiguration cfg(root);

    TEST_GET_OR_THROW_VALID(bool, "bool", "bool_scalar_var", true);
    TEST_GET_OR_THROW_VALID(int, "int", "int_scalar_var", 7);
    TEST_GET_OR_THROW_VALID(long, "long", "long_scalar_var", 12);
    TEST_GET_OR_THROW_VALID(float, "float", "float_scalar_var", 3.14f);
    TEST_GET_OR_THROW_VALID(double, "double", "double_scalar_var", 3.14)
    TEST_GET_OR_THROW_VALID(std::string, "std::string", "string_scalar_var", std::string("abc"))
    TEST_GET_OR_THROW_VALID(std::vector<int>, "std::vector<int>", "int_vec_var", (std::vector<int>{7, 6}))
    TEST_GET_OR_THROW_VALID(std::vector<long>, "std::vector<long>", "long_vec_var", (std::vector<long>{12, 13}))
    TEST_GET_OR_THROW_VALID(std::vector<float>, "std::vector<float>", "float_vec_var",
                            (std::vector<float>{3.14f, 2.71f}))
    TEST_GET_OR_THROW_VALID(std::vector<double>, "std::vector<double>", "double_vec_var",
                            (std::vector<double>{3.14, 2.71}))
    TEST_GET_OR_THROW_VALID(std::vector<std::string>, "std::vector<std::string>", "string_vec_var",
                            (std::vector<std::string>{"abc", "def"}))

#undef TEST_GET_OR_THROW_VALID
}

CASE("Test generic get_or_throw<T> missing key") {


#define TEST_GET_OR_THROW_MISSING(TYPE, TYPENAME_STRING, KEY)                    \
    SECTION("get_or_throw<" TYPENAME_STRING "> missing key") {                   \
        using metkit::mars2grib::utils::dict_traits::get_or_throw;               \
        EXPECT_THROWS({ TYPE actual_result = get_or_throw<TYPE>(cfg, (KEY)); }); \
    }

    // Used symbols
    using metkit::mars2grib::utils::dict_traits::get_or_throw;

    // Prepare a custom dictionary to test long
    const std::string yaml(R"json({
 bool_scalar_var: true,
 int_scalar_var: 7,
 long_scalar_var: 12,
 float_scalar_var: 3.14,
 double_scalar_var: 3.14,
 string_scalar_var: "abc",
 int_vec_var: [7,6],
 long_vec_var: [12,13],
 float_vec_var: [3.14, 2.71],
 double_vec_var: [3.14, 2.71],
 string_vec_var: ["abc", "def"]
 })json");


    // Initialize the configuration
    const eckit::YAMLConfiguration root(yaml);
    eckit::LocalConfiguration cfg(root);

    TEST_GET_OR_THROW_MISSING(bool, "bool", "missing_key");
    TEST_GET_OR_THROW_MISSING(int, "int", "missing_key");
    TEST_GET_OR_THROW_MISSING(long, "long", "missing_key");
    TEST_GET_OR_THROW_MISSING(float, "float", "missing_key");
    TEST_GET_OR_THROW_MISSING(double, "double", "missing_key")
    TEST_GET_OR_THROW_MISSING(std::string, "std::string", "missing_key")
    TEST_GET_OR_THROW_MISSING(std::vector<int>, "std::vector<int>", "missing_key")
    TEST_GET_OR_THROW_MISSING(std::vector<long>, "std::vector<long>", "missing_key")
    TEST_GET_OR_THROW_MISSING(std::vector<float>, "std::vector<float>", "missing_key")
    TEST_GET_OR_THROW_MISSING(std::vector<double>, "std::vector<double>", "missing_key")
    TEST_GET_OR_THROW_MISSING(std::vector<std::string>, "std::vector<std::string>", "missing_key")

#undef TEST_GET_OR_THROW_MISSING
}

CASE("Test generic get_opt<T> valid key") {

#define TEST_GET_OPT_VALID(TYPE, TYPENAME_STRING, KEY, EXPECTED)           \
    SECTION("get_opt<" TYPENAME_STRING "> valid key") {                    \
        using metkit::mars2grib::utils::dict_traits::get_opt;              \
        EXPECT_NO_THROW({                                                  \
            TYPE expected_result              = (EXPECTED);                \
            std::optional<TYPE> actual_result = get_opt<TYPE>(cfg, (KEY)); \
            EXPECT(actual_result.has_value());                             \
            EXPECT_EQUAL(actual_result.value(), expected_result);          \
        });                                                                \
    }

    // Used symbols
    using metkit::mars2grib::utils::dict_traits::get_opt;

    // Prepare a custom dictionary to test long
    const std::string yaml(R"json({
 bool_scalar_var: true,
 int_scalar_var: 7,
 long_scalar_var: 12,
 float_scalar_var: 3.14,
 double_scalar_var: 3.14,
 string_scalar_var: "abc",
 int_vec_var: [7,6],
 long_vec_var: [12,13],
 float_vec_var: [3.14, 2.71],
 double_vec_var: [3.14, 2.71],
 string_vec_var: ["abc", "def"]
 })json");


    // Initialize the configuration
    const eckit::YAMLConfiguration root(yaml);
    eckit::LocalConfiguration cfg(root);

    // Try to get the value of "step" using the generic get function
    // ---- bool ----
    TEST_GET_OPT_VALID(bool, "bool", "bool_scalar_var", true)
    TEST_GET_OPT_VALID(int, "int", "int_scalar_var", 7)
    TEST_GET_OPT_VALID(long, "long", "long_scalar_var", 12)
    TEST_GET_OPT_VALID(float, "float", "float_scalar_var", 3.14f)
    TEST_GET_OPT_VALID(double, "double", "double_scalar_var", 3.14)
    TEST_GET_OPT_VALID(std::string, "std::string", "string_scalar_var", std::string("abc"))
    TEST_GET_OPT_VALID(std::vector<int>, "std::vector<int>", "int_vec_var", (std::vector<int>{7, 6}))
    TEST_GET_OPT_VALID(std::vector<long>, "std::vector<long>", "long_vec_var", (std::vector<long>{12, 13}))
    TEST_GET_OPT_VALID(std::vector<float>, "std::vector<float>", "float_vec_var", (std::vector<float>{3.14f, 2.71f}))
    TEST_GET_OPT_VALID(std::vector<double>, "std::vector<double>", "double_vec_var", (std::vector<double>{3.14, 2.71}))
    TEST_GET_OPT_VALID(std::vector<std::string>, "std::vector<std::string>", "string_vec_var",
                       (std::vector<std::string>{"abc", "def"}))

#undef TEST_GET_OPT_VALID
}


CASE("Test generic get_opt<T> missing key") {

#define TEST_GET_OPT_MISSING(TYPE, TYPENAME_STRING, KEY)                   \
    SECTION("get_opt<" TYPENAME_STRING "> missing key") {                  \
        using metkit::mars2grib::utils::dict_traits::get_opt;              \
        EXPECT_NO_THROW({                                                  \
            std::optional<TYPE> actual_result = get_opt<TYPE>(cfg, (KEY)); \
            EXPECT(!actual_result.has_value());                            \
        });                                                                \
    }

    // Used symbols
    using metkit::mars2grib::utils::dict_traits::get_opt;

    // Prepare a custom dictionary to test long
    const std::string yaml(R"json({
 bool_scalar_var: true,
 int_scalar_var: 7,
 long_scalar_var: 12,
 float_scalar_var: 3.14,
 double_scalar_var: 3.14,
 string_scalar_var: "abc",
 int_vec_var: [7,6],
 long_vec_var: [12,13],
 float_vec_var: [3.14, 2.71],
 double_vec_var: [3.14, 2.71],
 string_vec_var: ["abc", "def"]
 })json");


    // Initialize the configuration
    const eckit::YAMLConfiguration root(yaml);
    eckit::LocalConfiguration cfg(root);

    TEST_GET_OPT_MISSING(bool, "bool", "missing_key")
    TEST_GET_OPT_MISSING(int, "int", "missing_key")
    TEST_GET_OPT_MISSING(long, "long", "missing_key")
    TEST_GET_OPT_MISSING(float, "float", "missing_key")
    TEST_GET_OPT_MISSING(double, "double", "missing_key")
    TEST_GET_OPT_MISSING(std::string, "std::string", "missing_key")
    TEST_GET_OPT_MISSING(std::vector<int>, "std::vector<int>", "missing_key")
    TEST_GET_OPT_MISSING(std::vector<long>, "std::vector<long>", "missing_key")
    TEST_GET_OPT_MISSING(std::vector<float>, "std::vector<float>", "missing_key")
    TEST_GET_OPT_MISSING(std::vector<double>, "std::vector<double>", "missing_key")
    TEST_GET_OPT_MISSING(std::vector<std::string>, "std::vector<std::string>", "missing_key")

#undef TEST_GET_OPT_MISSING
}

CASE("Test generic set_or_throw<T>") {

#define TEST_SET_OR_THROW(TYPE, TYPENAME_STRING, KEY, EXPECTED)                \
    SECTION("set_or_throw<" TYPENAME_STRING ">") {                             \
        using metkit::mars2grib::utils::dict_traits::set_or_throw;             \
        using metkit::mars2grib::utils::dict_traits::get_or_throw;             \
        TYPE expected_result = (EXPECTED);                                     \
        EXPECT_NO_THROW({ set_or_throw<TYPE>(cfg, (KEY), expected_result); }); \
        EXPECT_NO_THROW({                                                      \
            TYPE actual_result = get_or_throw<TYPE>(cfg, (KEY));               \
            EXPECT_EQUAL(actual_result, expected_result);                      \
        });                                                                    \
    }

    // Prepare a custom dictionary to test long
    const std::string yaml(R"json({})json");

    // Initialize the configuration
    const eckit::YAMLConfiguration root(yaml);
    eckit::LocalConfiguration cfg(root);


    TEST_SET_OR_THROW(bool, "bool", "bool_scalar_var", true);
    TEST_SET_OR_THROW(int, "int", "int_scalar_var", 7);
    TEST_SET_OR_THROW(long, "long", "long_scalar_var", 12);
    TEST_SET_OR_THROW(float, "float", "float_scalar_var", 3.14f);
    TEST_SET_OR_THROW(double, "double", "double_scalar_var", 3.14);
    TEST_SET_OR_THROW(std::string, "std::string", "string_scalar_var", std::string("abc"));
    TEST_SET_OR_THROW(std::vector<int>, "std::vector<int>", "int_vec_var", (std::vector<int>{7, 6}));
    TEST_SET_OR_THROW(std::vector<long>, "std::vector<long>", "long_vec_var", (std::vector<long>{12, 13}));
    TEST_SET_OR_THROW(std::vector<float>, "std::vector<float>", "float_vec_var", (std::vector<float>{3.14f, 2.71f}));
    TEST_SET_OR_THROW(std::vector<double>, "std::vector<double>", "double_vec_var", (std::vector<double>{3.14, 2.71}));
    TEST_SET_OR_THROW(std::vector<std::string>, "std::vector<std::string>", "string_vec_var",
                      (std::vector<std::string>{"abc", "def"}));

#undef TEST_SET_OR_THROW
}


CASE("Test generic set_or_ignore<T>") {

#define TEST_SET_OR_IGNORE(TYPE, TYPENAME_STRING, KEY, EXPECTED)                \
    SECTION("set_or_ignore<" TYPENAME_STRING ">") {                             \
        using metkit::mars2grib::utils::dict_traits::set_or_ignore;             \
        using metkit::mars2grib::utils::dict_traits::get_or_throw;              \
        TYPE expected_result = (EXPECTED);                                      \
        EXPECT_NO_THROW({ set_or_ignore<TYPE>(cfg, (KEY), expected_result); }); \
        EXPECT_NO_THROW({                                                       \
            TYPE actual_result = get_or_throw<TYPE>(cfg, (KEY));                \
            EXPECT_EQUAL(actual_result, expected_result);                       \
        });                                                                     \
    }

    // Used symbols

    // Prepare a custom dictionary to test long
    const std::string yaml(R"json({})json");

    // Initialize the configuration
    const eckit::YAMLConfiguration root(yaml);
    eckit::LocalConfiguration cfg(root);

    TEST_SET_OR_IGNORE(bool, "bool", "bool_scalar_var", true);
    TEST_SET_OR_IGNORE(int, "int", "int_scalar_var", 7);
    TEST_SET_OR_IGNORE(long, "long", "long_scalar_var", 12);
    TEST_SET_OR_IGNORE(float, "float", "float_scalar_var", 3.14f);
    TEST_SET_OR_IGNORE(double, "double", "double_scalar_var", 3.14);
    TEST_SET_OR_IGNORE(std::string, "std::string", "string_scalar_var", std::string("abc"));
    TEST_SET_OR_IGNORE(std::vector<int>, "std::vector<int>", "int_vec_var", (std::vector<int>{7, 6}));
    TEST_SET_OR_IGNORE(std::vector<long>, "std::vector<long>", "long_vec_var", (std::vector<long>{12, 13}));
    TEST_SET_OR_IGNORE(std::vector<float>, "std::vector<float>", "float_vec_var", (std::vector<float>{3.14f, 2.71f}));
    TEST_SET_OR_IGNORE(std::vector<double>, "std::vector<double>", "double_vec_var", (std::vector<double>{3.14, 2.71}));
    TEST_SET_OR_IGNORE(std::vector<std::string>, "std::vector<std::string>", "string_vec_var",
                       (std::vector<std::string>{"abc", "def"}));

#undef TEST_SET_OR_IGNORE
}


CASE("Test generic has valid") {

    // Used symbols
    using metkit::mars2grib::utils::dict_traits::has;

    // Prepare a custom dictionary to test long
    const std::string yaml(R"json({step: 12})json");

    // Initialize the configuration
    const eckit::YAMLConfiguration root(yaml);
    eckit::LocalConfiguration cfg(root);

    SECTION("has existing key") {
        EXPECT_NO_THROW({
            bool actual_has = has(cfg, "step");
            EXPECT(actual_has);
        });
    }
}
CASE("Test generic has missing") {

    // Used symbols
    using metkit::mars2grib::utils::dict_traits::has;

    // Prepare a custom dictionary to test long
    const std::string yaml(R"json({step: 12})json");

    // Initialize the configuration
    const eckit::YAMLConfiguration root(yaml);
    eckit::LocalConfiguration cfg(root);

    SECTION("has missing key") {
        EXPECT_NO_THROW({
            bool actual_has = has(cfg, "missing_key");
            EXPECT(!actual_has);
        });
    }
}


CASE("Test generic typed has valid") {

#define TEST_TYPED_HAS_VALID(TYPE, TYPENAME_STRING, KEY)                                         \
    {SECTION("has<" TYPENAME_STRING "> valid"){using metkit::mars2grib::utils::dict_traits::has; \
    EXPECT_NO_THROW({                                                                            \
        bool actual_has = has<TYPE>(cfg, (KEY));                                                 \
        EXPECT(actual_has);                                                                      \
    });                                                                                          \
    }                                                                                            \
    }

    // Prepare a custom dictionary to test long
    const std::string yaml(R"json({
 bool_scalar_var: true,
 int_scalar_var: 7,
 long_scalar_var: 12,
 float_scalar_var: 3.14,
 double_scalar_var: 3.14,
 string_scalar_var: "abc",
 int_vec_var: [7,6],
 long_vec_var: [12,13],
 float_vec_var: [3.14, 2.71],
 double_vec_var: [3.14, 2.71],
 string_vec_var: ["abc", "def"]
 })json");

    // Initialize the configuration
    const eckit::YAMLConfiguration root(yaml);
    eckit::LocalConfiguration cfg(root);

    TEST_TYPED_HAS_VALID(bool, "bool", "bool_scalar_var");
    TEST_TYPED_HAS_VALID(int, "int", "int_scalar_var");
    TEST_TYPED_HAS_VALID(long, "long", "long_scalar_var");
    TEST_TYPED_HAS_VALID(float, "float", "float_scalar_var");
    TEST_TYPED_HAS_VALID(double, "double", "double_scalar_var");
    TEST_TYPED_HAS_VALID(std::string, "std::string", "string_scalar_var");
    TEST_TYPED_HAS_VALID(std::vector<int>, "std::vector<int>", "int_vec_var");
    TEST_TYPED_HAS_VALID(std::vector<long>, "std::vector<long>", "long_vec_var");
    TEST_TYPED_HAS_VALID(std::vector<float>, "std::vector<float>", "float_vec_var");
    TEST_TYPED_HAS_VALID(std::vector<double>, "std::vector<double>", "double_vec_var");
    TEST_TYPED_HAS_VALID(std::vector<std::string>, "std::vector<std::string>", "string_vec_var");

#undef TEST_TYPED_HAS_VALID
}


CASE("Test generic typed has missing") {

#define TEST_TYPED_HAS_MISSING(TYPE, TYPENAME_STRING, KEY)                    \
    {                                                                         \
        using metkit::mars2grib::utils::dict_traits::has;                     \
        SECTION("has<" TYPENAME_STRING ">") {                                 \
            bool actual_has;                                                  \
            EXPECT_NO_THROW({ actual_has = has<TYPE>(cfg, "missing_key"); }); \
            EXPECT(!actual_has);                                              \
        }                                                                     \
    }

    // Prepare a custom dictionary to test long
    const std::string yaml(R"json({
 bool_scalar_var: true,
 int_scalar_var: 7,
 long_scalar_var: 12,
 float_scalar_var: 3.14,
 double_scalar_var: 3.14,
 string_scalar_var: "abc",
 int_vec_var: [7,6],
 long_vec_var: [12,13],
 float_vec_var: [3.14, 2.71],
 double_vec_var: [3.14, 2.71],
 string_vec_var: ["abc", "def"]
 })json");

    // Initialize the configuration
    const eckit::YAMLConfiguration root(yaml);
    eckit::LocalConfiguration cfg(root);

    TEST_TYPED_HAS_MISSING(bool, "bool", "bool_scalar_var");
    TEST_TYPED_HAS_MISSING(int, "int", "int_scalar_var");
    TEST_TYPED_HAS_MISSING(long, "long", "long_scalar_var");
    TEST_TYPED_HAS_MISSING(float, "float", "float_scalar_var");
    TEST_TYPED_HAS_MISSING(double, "double", "double_scalar_var");
    TEST_TYPED_HAS_MISSING(std::string, "std::string", "string_scalar_var");
    TEST_TYPED_HAS_MISSING(std::vector<int>, "std::vector<int>", "int_vec_var");
    TEST_TYPED_HAS_MISSING(std::vector<long>, "std::vector<long>", "long_vec_var");
    TEST_TYPED_HAS_MISSING(std::vector<float>, "std::vector<float>", "float_vec_var");
    TEST_TYPED_HAS_MISSING(std::vector<double>, "std::vector<double>", "double_vec_var");
    TEST_TYPED_HAS_MISSING(std::vector<std::string>, "std::vector<std::string>", "string_vec_var");

#undef TEST_TYPED_HAS_MISSING
}


CASE("Test generic check valid") {

#define TEST_CHECK_VALID(TYPE, TYPENAME_STRING, KEY)                                         \
    SECTION("check<" TYPENAME_STRING "> valid") {                                            \
        using metkit::mars2grib::utils::dict_traits::check;                                  \
        SECTION("check<" TYPENAME_STRING ">") {                                              \
            EXPECT_NO_THROW({                                                                \
                bool actual_has = check<TYPE>(cfg, (KEY), [](const auto&) { return true; }); \
                EXPECT(actual_has);                                                          \
            });                                                                              \
        }                                                                                    \
    }

    // Prepare a custom dictionary to test long
    const std::string yaml(R"json({
 bool_scalar_var: true,
 int_scalar_var: 7,
 long_scalar_var: 12,
 float_scalar_var: 3.14,
 double_scalar_var: 3.14,
 string_scalar_var: "abc",
 int_vec_var: [7,6],
 long_vec_var: [12,13],
 float_vec_var: [3.14, 2.71],
 double_vec_var: [3.14, 2.71],
 string_vec_var: ["abc", "def"]
 })json");

    // Initialize the configuration
    const eckit::YAMLConfiguration root(yaml);
    eckit::LocalConfiguration cfg(root);

    TEST_CHECK_VALID(bool, "bool", "bool_scalar_var");
    TEST_CHECK_VALID(int, "int", "int_scalar_var");
    TEST_CHECK_VALID(long, "long", "long_scalar_var");
    TEST_CHECK_VALID(float, "float", "float_scalar_var");
    TEST_CHECK_VALID(double, "double", "double_scalar_var");
    TEST_CHECK_VALID(std::string, "std::string", "string_scalar_var");
    TEST_CHECK_VALID(std::vector<int>, "std::vector<int>", "int_vec_var");
    TEST_CHECK_VALID(std::vector<long>, "std::vector<long>", "long_vec_var");
    TEST_CHECK_VALID(std::vector<float>, "std::vector<float>", "float_vec_var");
    TEST_CHECK_VALID(std::vector<double>, "std::vector<double>", "double_vec_var");
    TEST_CHECK_VALID(std::vector<std::string>, "std::vector<std::string>", "string_vec_var");

#undef TEST_CHECK_VALID
}


CASE("Test generic check missing") {

#define TEST_CHECK_MISSING(TYPE, TYPENAME_STRING, KEY)                                        \
    SECTION("check<" TYPENAME_STRING "> missing") {                                           \
        using metkit::mars2grib::utils::dict_traits::check;                                   \
        SECTION("check<" TYPENAME_STRING ">") {                                               \
            EXPECT_NO_THROW({                                                                 \
                bool actual_has = check<TYPE>(cfg, (KEY), [](const auto&) { return false; }); \
                EXPECT(actual_has);                                                           \
            });                                                                               \
        }                                                                                     \
    }

    // Prepare a custom dictionary to test long
    const std::string yaml(R"json({
 bool_scalar_var: true,
 int_scalar_var: 7,
 long_scalar_var: 12,
 float_scalar_var: 3.14,
 double_scalar_var: 3.14,
 string_scalar_var: "abc",
 int_vec_var: [7,6],
 long_vec_var: [12,13],
 float_vec_var: [3.14, 2.71],
 double_vec_var: [3.14, 2.71],
 string_vec_var: ["abc", "def"]
 })json");

    // Initialize the configuration
    const eckit::YAMLConfiguration root(yaml);
    eckit::LocalConfiguration cfg(root);

    TEST_CHECK_MISSING(bool, "bool", "bool_scalar_var");
    TEST_CHECK_MISSING(int, "int", "int_scalar_var");
    TEST_CHECK_MISSING(long, "long", "long_scalar_var");
    TEST_CHECK_MISSING(float, "float", "float_scalar_var");
    TEST_CHECK_MISSING(double, "double", "double_scalar_var");
    TEST_CHECK_MISSING(std::string, "std::string", "string_scalar_var");
    TEST_CHECK_MISSING(std::vector<int>, "std::vector<int>", "int_vec_var");
    TEST_CHECK_MISSING(std::vector<long>, "std::vector<long>", "long_vec_var");
    TEST_CHECK_MISSING(std::vector<float>, "std::vector<float>", "float_vec_var");
    TEST_CHECK_MISSING(std::vector<double>, "std::vector<double>", "double_vec_var");
    TEST_CHECK_MISSING(std::vector<std::string>, "std::vector<std::string>", "string_vec_var");

#undef TEST_CHECK_MISSING
}

CASE("Test sub-configuration") {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;

    // Prepare a custom dictionary to test long
    const std::string yaml(R"json({
 bool_scalar_var: true,
 int_scalar_var: 7,
 long_scalar_var: 12,
 float_scalar_var: 3.14,
 double_scalar_var: 3.14,
 string_scalar_var: "abc",
 int_vec_var: [7,6],
 long_vec_var: [12,13],
 float_vec_var: [3.14, 2.71],
 double_vec_var: [3.14, 2.71],
 string_vec_var: ["abc", "def"]
 })json");

    // Initialize the configuration
    const eckit::YAMLConfiguration root(yaml);
    eckit::LocalConfiguration cfg(root);
    eckit::LocalConfiguration n1(root);
    eckit::LocalConfiguration n2(root);
    std::vector<eckit::LocalConfiguration> n3{n1, n2};

    SECTION("set sub-configuration n1") {
        EXPECT_NO_THROW({ set_or_throw<eckit::LocalConfiguration>(cfg, "n1", n1); });
        EXPECT_NO_THROW({ eckit::LocalConfiguration actual_n1 = get_or_throw<eckit::LocalConfiguration>(cfg, "n1"); });
    }

    SECTION("set sub-configuration n2") {
        EXPECT_NO_THROW({ set_or_throw<eckit::LocalConfiguration>(cfg, "n2", n2); });
        EXPECT_NO_THROW({ eckit::LocalConfiguration actual_n2 = get_or_throw<eckit::LocalConfiguration>(cfg, "n2"); });
    }
    SECTION("set sub-configurations") {
        EXPECT_NO_THROW({ set_or_throw<std::vector<eckit::LocalConfiguration>>(cfg, "n3", n3); });
        EXPECT_NO_THROW({
            std::vector<eckit::LocalConfiguration> actual_n3 =
                get_or_throw<std::vector<eckit::LocalConfiguration>>(cfg, "n3");
        });
    }

#undef TEST_CHECK_MISSING
}


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
