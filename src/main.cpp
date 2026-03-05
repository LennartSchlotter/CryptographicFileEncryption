#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

int main() {
    std::string file_path;
    std::cout << "Please enter the file path: " << std::flush;
    std::cin >> file_path;

    std::string encrypted_file_path = file_path + ".cfe";

    std::filesystem::copy_file(file_path, encrypted_file_path);

    std::ifstream file(encrypted_file_path, std::ios::binary);

    std::cout << "You entered: " << file_path << "\n";

    if (!file.is_open()) {
        std::cout << "Failed to open file at " << file_path;
        return EXIT_FAILURE;
    }

    std::cout << "You received: " << encrypted_file_path << "\n";

    return 0;
}