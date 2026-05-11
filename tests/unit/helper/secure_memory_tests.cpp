#include "gtest/gtest.h"
#include "helper/secure_memory.h"
#include <algorithm>
#include <vector>

constexpr size_t DEFAULT_BUFFER_SIZE = 64;
const unsigned char DEFAULT_BUFFER_INPUT = 0xAA;

template<typename T>
struct TestValue {
    static T Default() {
        return T{};
    }
};

template <>
struct TestValue<float> {
    static float Default() { return 1.0F; }
};

template <>
struct TestValue<double> {
    static double Default() { return 1.0; }
};

template <typename T>
class VectorTest : public ::testing::Test {
    protected: 
        std::vector<T> vec; //NOLINT
        void SetUp() override {
            vec.resize(DEFAULT_BUFFER_SIZE, TestValue<T>::Default());
        }

        void TearDown() override {
            vec.clear();
        }
};

using TestTypes = ::testing::Types<unsigned char, char, std::byte, uint32_t, uint64_t>;

TYPED_TEST_SUITE(VectorTest, TestTypes);

class SizeVariationTests : public ::testing::TestWithParam<size_t> {

};

INSTANTIATE_TEST_SUITE_P(
    SizeVariations,
    SizeVariationTests,
    ::testing::Values(1, 2, 3, 4, 8, 16, 32, 64)
);

TEST(SecureMemoryTests, SecureZeroNulls) {
    std::vector<unsigned char> buffer(DEFAULT_BUFFER_SIZE, DEFAULT_BUFFER_INPUT);

    secure_zero(buffer.data(), buffer.size());

    EXPECT_TRUE(std::ranges::all_of(buffer, [](unsigned char character) {return character == 0; }));
}

TEST(SecureMemoryTests, SecureZeroNoOpsCorrectly) {
    std::vector<unsigned char> buffer(DEFAULT_BUFFER_SIZE, DEFAULT_BUFFER_INPUT);
    
    secure_zero(buffer.data(), 0);
    
    EXPECT_TRUE(std::ranges::all_of(buffer, [](unsigned char character) {return character == 0xAA; }));
}

TEST(SecureMemoryTests, SecureZeroDoesntCrashOnNullPtr) {
    secure_zero(nullptr, 0);
}

TYPED_TEST(VectorTest, DifferentPointerTypes) {
    secure_zero(this->vec.data(), this->vec.size() * sizeof(TypeParam));

    EXPECT_TRUE(std::ranges::all_of(this->vec, [](const TypeParam& value) {return value == TypeParam{}; }));
}

TEST_P(SizeVariationTests, RangedSizes) {
    size_t value = GetParam();
    std::vector<unsigned char> buffer(DEFAULT_BUFFER_SIZE, DEFAULT_BUFFER_INPUT);

    secure_zero(buffer.data(), value);

    EXPECT_TRUE(std::ranges::all_of(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(value), [](unsigned char character) { return character == 0; }));
    EXPECT_TRUE(std::ranges::all_of(buffer.begin() + static_cast<std::ptrdiff_t>(value), buffer.end(), [](unsigned char character) { return character == DEFAULT_BUFFER_INPUT; }));
}