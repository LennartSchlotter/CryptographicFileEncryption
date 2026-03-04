BUILD_DIR := build
RELEASE_DIR := build-release
BINARY := bin/CFE

.PHONY: run build release format lint check clean

run: build
	./$(BUILD_DIR)/$(BINARY)

build:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug 2>&1 | head -20
	cmake --build $(BUILD_DIR)

release:
	cmake -S . -B $(RELEASE_DIR) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(RELEASE_DIR)
	./$(RELEASE_DIR)/$(BINARY)

format:
	clang-format -i src/*.cpp include/*.hpp

lint:
	clang-tidy -p $(BUILD_DIR) src/*.cpp --warnings-as-errors="*"

check:
	cppcheck --enable=all --std=c++23 --suppress=missingIncludeSystem src/

clean:
	rm -rf $(BUILD_DIR) $(RELEASE_DIR)