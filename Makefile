BUILD_DIR := build
RELEASE_DIR := build-release
BINARY := bin/CFE

.PHONY: run build release format lint check clean

run: build
	./$(BUILD_DIR)/$(BINARY) $(ARGS)

build:
	rm -rf $(BUILD_DIR)
	cmake -S . -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_COMPILER=clang++-18 \
		-DCMAKE_C_COMPILER=clang-18 \
		-DCMAKE_CXX_FLAGS="-stdlib=libc++" \
		-DCMAKE_EXE_LINKER_FLAGS="-stdlib=libc++" \
		-DCMAKE_PREFIX_PATH=$(HOME)/libsodium
	cmake --build $(BUILD_DIR) --parallel

release:
	cmake -S . -B $(RELEASE_DIR) -DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_COMPILER=clang++-18 \
		-DCMAKE_C_COMPILER=clang-18 \
		-DCMAKE_CXX_FLAGS="-stdlib=libc++" \
		-DCMAKE_EXE_LINKER_FLAGS="-stdlib=libc++" \
		-DCMAKE_PREFIX_PATH=$(HOME)/libsodium
	cmake --build $(RELEASE_DIR) --parallel
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