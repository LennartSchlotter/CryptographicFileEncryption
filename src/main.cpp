#include <iostream>
#include <string>

int main() {
    std::string file_path;
    std::cout << "Please enter the file path: " << std::flush;

    std::cin >> file_path;

    std::cout << "You entered: " << file_path << "\n";
    return 0;
}