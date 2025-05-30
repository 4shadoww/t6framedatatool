BUILD_DIR=build
.DEFAULT_GOAL := build

test_deps:
	mkdir -p 3rdparty
	cd 3rdparty && \
	wget https://github.com/google/googletest/releases/download/v1.16.0/googletest-1.16.0.tar.gz && \
	tar -xf googletest-1.16.0.tar.gz && \
	mv googletest-1.16.0 googletest

configure_cli:
	cmake -G "Ninja" -S. -B$(BUILD_DIR)
	# Copy compile commands db to root for lsp
	cp build/compile_commands.json .

configure_cli_debug:
	cmake -G "Ninja" -S. -B$(BUILD_DIR) -DDEBUG=ON
	# Copy compile commands db to root for lsp
	cp build/compile_commands.json .

configure_debug:
	cmake -G "Ninja" -S. -B$(BUILD_DIR) -DDEBUG=ON -DGUI=ON
	# Copy compile commands db to root for lsp
	cp build/compile_commands.json .

configure_release:
	cmake -G "Ninja" -S. -B$(BUILD_DIR) -DGUI=ON
	# Copy compile commands db to root for lsp
	cp build/compile_commands.json .

configure_test:
	cmake -G "Ninja" -S. -B$(BUILD_DIR) -DDEBUG=ON -DUNIT_TESTING=ON
	# Copy compile commands db to root for lsp
	cp build/compile_commands.json .

cli_debug: configure_cli_debug
	cmake --build $(BUILD_DIR)

cli_release: configure_cli
	cmake --build $(BUILD_DIR)

debug: configure_debug
	cmake --build $(BUILD_DIR)

release: configure_debug
	cmake --build $(BUILD_DIR)

test: configure_test
	cmake --build $(BUILD_DIR)

run_test: test
	ctest --output-on-failure --test-dir $(BUILD_DIR)

clean:
	rm -r $(BUILD_DIR)
