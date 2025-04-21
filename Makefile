BUILD_DIR=build
.DEFAULT_GOAL := build

test_deps:
	mkdir -p 3rdparty
	cd 3rdparty && \
	wget https://github.com/google/googletest/releases/download/v1.16.0/googletest-1.16.0.tar.gz && \
	tar -xf googletest-1.16.0.tar.gz && \
	mv googletest-1.16.0 googletest

configure:
	cmake -G "Ninja" -S. -B$(BUILD_DIR)
	# Coopy compile commands db to root for lsp
	cp build/compile_commands.json .

configure_debug:
	cmake -G "Ninja" -S. -B$(BUILD_DIR) -DDEBUG=ON
	# Coopy compile commands db to root for lsp
	cp build/compile_commands.json .

configure_test:
	cmake -G "Ninja" -S. -B$(BUILD_DIR) -DUNIT_TESTING=ON
	# Coopy compile commands db to root for lsp
	cp build/compile_commands.json .

debug: configure_debug
	cmake --build $(BUILD_DIR)

release: configure
	cmake --build $(BUILD_DIR)

test: configure_test
	cmake --build $(BUILD_DIR)

run_test: build_test
	ctest --output-on-failure --test-dir $(BUILD_DIR)

clean:
	rm -r $(BUILD_DIR)
