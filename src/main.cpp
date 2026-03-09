#include "handler.h"

int main(int argc, char *argv[]) {
    std::span args{argv, static_cast<std::size_t>(argc)};
    parse(args);

    return 0;
}