#include "helper/password.h"
#include "helper/secure_allocator.h"
#include <gtest/gtest.h>

TEST(PasswordTests, ReadPasswordTerminates) {
    std::istringstream input("Password1\n23");
    auto* old_buf = std::cin.rdbuf(input.rdbuf());

    const std::string expected_str = "Password1";
    const std::vector<char, SecureAllocator<char>> expected(expected_str.begin(), expected_str.end());

    std::vector<char, SecureAllocator<char>> result = read_password("Enter Password: ");
    std::cin.rdbuf(old_buf);

    EXPECT_EQ(result, expected);
}

TEST(PasswordTests, ReadPasswordTerminatesWindows) {
    std::istringstream input("Password1\r\n23");
    auto* old_buf = std::cin.rdbuf(input.rdbuf());

    const std::string expected_str = "Password1";
    const std::vector<char, SecureAllocator<char>> expected(expected_str.begin(), expected_str.end());

    std::vector<char, SecureAllocator<char>> result = read_password("Enter Password: ");
    std::cin.rdbuf(old_buf);

    EXPECT_EQ(result, expected);
}

TEST(PasswordTests, ReadPasswordTerminatesEOF) {
    std::istringstream input("Password1");
    auto* old_buf = std::cin.rdbuf(input.rdbuf());

    const std::string expected_str = "Password1";
    const std::vector<char, SecureAllocator<char>> expected(expected_str.begin(), expected_str.end());

    std::vector<char, SecureAllocator<char>> result = read_password("Enter Password: ");
    std::cin.rdbuf(old_buf);

    EXPECT_EQ(result, expected);
}

TEST(PasswordTests, ReadPasswordEmptyInput) {
    std::istringstream input("\n");
    auto* old_buf = std::cin.rdbuf(input.rdbuf());

    const std::vector<char, SecureAllocator<char>> expected;

    std::vector<char, SecureAllocator<char>> result = read_password("Enter Password: ");
    std::cin.rdbuf(old_buf);

    EXPECT_EQ(result, expected);
}

TEST(PasswordTests, ReadPasswordHappyPath) {
    std::istringstream input("Password1\n");
    auto* old_buf = std::cin.rdbuf(input.rdbuf());

    const std::string expected_str = "Password1";
    const std::vector<char, SecureAllocator<char>> expected(expected_str.begin(), expected_str.end());

    std::vector<char, SecureAllocator<char>> result = read_password("Enter Password: ");
    std::cin.rdbuf(old_buf);

    EXPECT_EQ(result, expected);
}