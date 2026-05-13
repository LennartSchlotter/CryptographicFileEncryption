BUILD_DIR := build
RELEASE_DIR := build-release
BINARY := bin/CFE
COVERAGE_DIR := build-coverage

.PHONY: run build release format lint check clean

run: build
	./$(BUILD_DIR)/$(BINARY) $(ARGS)

build:
	rm -rf $(BUILD_DIR)
	cmake -S . -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_COMPILER=clang++-18 \
		-DCMAKE_C_COMPILER=clang-18 \
		-DCMAKE_PREFIX_PATH=$(HOME)/libsodium
	cmake --build $(BUILD_DIR) --parallel

release:
	cmake -S . -B $(RELEASE_DIR) -DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_COMPILER=clang++-18 \
		-DCMAKE_C_COMPILER=clang-18 \
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

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

test-unit: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure -R "unit"

test-integration: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure -R "integration"

coverage:
	cmake -S . -B $(COVERAGE_DIR) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_COMPILER=clang++-18 \
		-DCMAKE_C_COMPILER=clang-18 \
		-DCMAKE_CXX_FLAGS="-stdlib=libc++ -fprofile-instr-generate -fcoverage-mapping" \
		-DCMAKE_EXE_LINKER_FLAGS="-stdlib=libc++" \
		-DCMAKE_PREFIX_PATH=$(HOME)/libsodium
	cmake --build $(COVERAGE_DIR) --parallel
	cd $(COVERAGE_DIR) && \
		LLVM_PROFILE_FILE="cfe.profraw" ./tests/CFE_tests && \
		llvm-profdata-18 merge -sparse cfe.profraw -o cfe.profdata && \
		llvm-cov-18 show ./tests/CFE_tests \
			-instr-profile=cfe.profdata \
			-format=html \
			-output-dir=coverage-report \
			--ignore-filename-regex="(tests/|_deps/)"
	@echo "Coverage report: $(COVERAGE_DIR)/coverage-report/index.html"

clean:
	rm -rf $(BUILD_DIR) $(RELEASE_DIR)