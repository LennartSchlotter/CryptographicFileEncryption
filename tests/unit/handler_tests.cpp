#include "gtest/gtest.h"
#include "handler.h"
#include <__expected/expected.h>
#include <span>
#include <string>
#include <string_view>

struct TestParam {
    CryptoAlgorithms algorithm;
    std::string expectedString;
};

class AlgorithmTestFixture : public ::testing::TestWithParam<TestParam> {
};

TEST(HandlerTests, FlagsVerboseEnablesVerboseMode) {
    std::vector<std::string_view> args = {"--verbose"};
    std::string tool = "encrypt";
    bool verbose = false;
    std::string path = "test.txt";
    CryptoAlgorithms algorithm = CryptoAlgorithms::AEGIS_256;

    std::expected<void, Result> result = parse_flags(tool, args, path, verbose, algorithm);

    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(verbose);
}

TEST(HandlerTests, FlagsOutputChangesPath) {
    std::vector<std::string_view> args = {"-o", "test2.txt"};
    std::string tool = "encrypt";
    bool verbose = false;
    std::string path = "test.txt";
    CryptoAlgorithms algorithm = CryptoAlgorithms::AEGIS_256;

    std::expected<void, Result> result = parse_flags(tool, args, path, verbose, algorithm);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(path, "test2.txt");
}

TEST(HandlerTests, FlagsMissingOutputArgFails) {
    std::vector<std::string_view> args = {"-o"};
    std::string tool = "encrypt";
    bool verbose = false;
    std::string path = "test.txt";
    CryptoAlgorithms algorithm = CryptoAlgorithms::AEGIS_256;

    std::expected<void, Result> result = parse_flags(tool, args, path, verbose, algorithm);

    EXPECT_FALSE(result);
}

TEST(HandlerTests, FlagsUnknownCipherAlgFails) {
    std::vector<std::string_view> args = {"-c", "test"};
    std::string tool = "encrypt";
    bool verbose = false;
    std::string path = "test.txt";
    CryptoAlgorithms algorithm = CryptoAlgorithms::AEGIS_256;

    std::expected<void, Result> result = parse_flags(tool, args, path, verbose, algorithm);

    EXPECT_FALSE(result);
}

TEST(HandlerTests, DecryptCipherPassedFails) {
    std::vector<std::string_view> args = {"-c", "ml_kem"};
    std::string tool = "decrypt";
    bool verbose = false;
    std::string path = "test.txt";
    CryptoAlgorithms algorithm = CryptoAlgorithms::AEGIS_256;

    std::expected<void, Result> result = parse_flags(tool, args, path, verbose, algorithm);

    EXPECT_FALSE(result);
}

TEST_P(AlgorithmTestFixture, FlagsValidAlgorithmMapsCorrectly) {
    TestParam parameters = GetParam();
    std::vector<std::string_view> args = {"-c", parameters.expectedString};
    std::string tool = "encrypt";
    bool verbose = false;
    std::string path = "test.txt";
    CryptoAlgorithms algorithm = CryptoAlgorithms::ECDH_X25519;

    std::expected<void, Result> result = parse_flags(tool, args, path, verbose, algorithm);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(algorithm, parameters.algorithm);
}

INSTANTIATE_TEST_SUITE_P(
    AlgorithmTestsSuite,
    AlgorithmTestFixture,
    ::testing::Values(
        TestParam{CryptoAlgorithms::AEGIS_256, "aegis"},
        TestParam{CryptoAlgorithms::ChaCha20_POLY1305, "chacha20"},
        TestParam{CryptoAlgorithms::ECDH_X25519, "ecdh"},
        TestParam{CryptoAlgorithms::ML_KEM_768, "ml_kem"}
    )
);

TEST(HandlerTests, UnknownFlagFails) {
    std::vector<std::string_view> args = {"--verbose_level", "severe"};
    std::string tool = "encrypt";
    bool verbose = false;
    std::string path = "test.txt";
    CryptoAlgorithms algorithm = CryptoAlgorithms::AEGIS_256;

    std::expected<void, Result> result = parse_flags(tool, args, path, verbose, algorithm);

    EXPECT_FALSE(result);
}

TEST(HandlerTests, ParseInvalidToolFails) {
    std::vector<std::string_view> input = {"analyze", "test.txt"};

    std::expected<void, Result> result = parse(input);

    EXPECT_FALSE(result);
}
