BUILD_DIR := build
RELEASE_DIR := build-release
BINARY := bin/CFE

.PHONY: run build release format lint check clean

run: build
	./$(BUILD_DIR)/$(BINARY) $(ARGS)

build:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug 2>&1
	cmake --build $(BUILD_DIR)

release:
	cmake -S . -B $(RELEASE_DIR) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(RELEASE_DIR)
	./$(RELEASE_DIR)/$(BINARY)

format:
	find src -type f \( -name "*.cpp" -o -name "*.h" \) -exec sh -c 'clang-format "{}" | diff -u --color=always "{}" - || true' \;

lint:
	find src -type f -name "*.cpp" -exec clang-tidy -p $(BUILD_DIR) {} --warnings-as-errors="*" \;

check:
	cppcheck --enable=all --std=c++23 --inconclusive \
			--suppress=missingIncludeSystem \
			--suppressions-list=suppress-checkers.txt \
			-I src/ src/

clean:
	rm -rf $(BUILD_DIR) $(RELEASE_DIR)