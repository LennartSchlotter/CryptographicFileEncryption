BUILD_DIR := build

.PHONY: run build debug release lint clean

run: build
	./$(BUILD_DIR)/real

build:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug 2>&1 | head -20
	cmake --build $(BUILD_DIR)

release:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD_DIR)
	./$(BUILD_DIR)/real

lint:
	clang-tidy src/*.cpp --warnings-as-errors="*" -- -std=c++23 -Iinclude

check:
	cppcheck --enable=all --std=c++23 --suppress=missingIncludeSystem src/

clean:
	rm -rf $(BUILD_DIR)
EOF