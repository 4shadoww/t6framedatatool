BUILD_DIR=build
BUILD_DIR_RELEASE=$(BUILD_DIR)/release
BUILD_DIR_DEBUG=$(BUILD_DIR)/debug
BUILD_DIR_TEST=$(BUILD_DIR)/test
.DEFAULT_GOAL := build

test_deps:
	mkdir -p 3rdparty
	cd 3rdparty && \
	wget https://github.com/google/googletest/releases/download/v1.16.0/googletest-1.16.0.tar.gz && \
	tar -xf googletest-1.16.0.tar.gz && \
	mv googletest-1.16.0 googletest

configure_cli:
	cmake -G "Ninja" -S. -B$(BUILD_DIR_RELEASE)
	# Copy compile commands db to root for lsp
	cp $(BUILD_DIR_RELEASE)/compile_commands.json .

configure_cli_debug:
	cmake -G "Ninja" -S. -B$(BUILD_DIR_DEBUG) -DDEBUG=ON
	# Copy compile commands db to root for lsp
	cp $(BUILD_DIR_DEBUG)/compile_commands.json .

configure_debug:
	cmake -G "Ninja" -S. -B$(BUILD_DIR_DEBUG) -DDEBUG=ON -DGUI=ON
	# Copy compile commands db to root for lsp
	cp $(BUILD_DIR_DEBUG)/compile_commands.json .

configure_release:
	cmake -G "Ninja" -S. -B$(BUILD_DIR_RELEASE) -DGUI=ON
	# Copy compile commands db to root for lsp
	cp $(BUILD_DIR_RELEASE)/compile_commands.json .

configure_test:
	cmake -G "Ninja" -S. -B$(BUILD_DIR_TEST) -DDEBUG=ON -DUNIT_TESTING=ON
	# Copy compile commands db to root for lsp
	cp $(BUILD_DIR_TEST)/compile_commands.json .

cli_debug: configure_cli_debug
	cmake --build $(BUILD_DIR_DEBUG)

cli_release: configure_cli
	cmake --build $(BUILD_DIR_RELEASE)

debug: configure_debug
	cmake --build $(BUILD_DIR_DEBUG)

release: configure_release
	cmake --build $(BUILD_DIR_RELEASE)

test: configure_test
	cmake --build $(BUILD_DIR_TEST)

run_test: test
	ctest --output-on-failure --test-dir $(BUILD_DIR_TEST)

lint:
	run-clang-tidy

format:
	bash -c "find src -iname '*.h' -o -iname '*.cpp' -o -iname '*.hpp' | clang-format --style=file -i --files=/dev/stdin"

check-format:
	bash -c "find src -iname '*.h' -o -iname '*.cpp' -o -iname '*.hpp' | clang-format --style=file -i -n --Werror --files=/dev/stdin"

check: check-format lint

clean:
	rm -r $(BUILD_DIR)

all: test debug release
