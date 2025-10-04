BUILD_DIR=build
3RDPARTY_DIR=3rdparty
BUILD_DIR_RELEASE=$(BUILD_DIR)/release
BUILD_DIR_DEBUG=$(BUILD_DIR)/debug
BUILD_DIR_TEST=$(BUILD_DIR)/test
GENERATED_SRC_DIR=generated-src
FONT_SOURCE=$(GENERATED_SRC_DIR)/fonts.h
.DEFAULT_GOAL := all

.PHONY: test_deps 3rdparty

ignore_3rdparty:
	mkdir -p $(3RDPARTY_DIR)
	echo "Checks: '-*'" > $(3RDPARTY_DIR)/.clang-tidy

ignore_build:
	mkdir -p $(3RDPARTY_DIR)
	echo "Checks: '-*'" > $(BUILD_DIR)/.clang-tidy

test_deps:
	cd $(3RDPARTY_DIR) && \
	wget https://github.com/google/googletest/releases/download/v1.16.0/googletest-1.16.0.tar.gz && \
	tar -xf googletest-1.16.0.tar.gz && \
	mv googletest-1.16.0 googletest

3rdparty:
	cd $(3RDPARTY_DIR) && \
	wget https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip && \
	rm -rf glfw-3.4 glfw && \
	unzip glfw-3.4.zip && \
	mv glfw-3.4 glfw
	
	cd $(3RDPARTY_DIR) && \
	wget https://github.com/Dav1dde/glad/archive/refs/tags/v2.0.8.zip && \
	rm -rf glad-2.0.8 glad && \
	unzip v2.0.8.zip && \
	mv glad-2.0.8 glad
	
	cd $(3RDPARTY_DIR) && \
	wget https://github.com/freetype/freetype/archive/refs/tags/VER-2-14-1.zip && \
	rm -rf freetype-VER-2-14-1 freetype && \
	unzip VER-2-14-1.zip && \
	mv freetype-VER-2-14-1 freetype
	
	cd $(3RDPARTY_DIR) && \
	wget https://github.com/g-truc/glm/releases/download/1.0.1/glm-1.0.1-light.zip && \
	rm -rf glm && \
	unzip glm-1.0.1-light.zip

configure_cli: fonts
	cmake -G "Ninja" -S. -B$(BUILD_DIR_RELEASE)
	# Copy compile commands db to root for lsp
	cp $(BUILD_DIR_RELEASE)/compile_commands.json .

configure_cli_debug: fonts
	cmake -G "Ninja" -S. -B$(BUILD_DIR_DEBUG) -DDEBUG=ON
	# Copy compile commands db to root for lsp
	cp $(BUILD_DIR_DEBUG)/compile_commands.json .

configure_debug: fonts
	cmake -G "Ninja" -S. -B$(BUILD_DIR_DEBUG) -DDEBUG=ON -DGUI=ON
	# Copy compile commands db to root for lsp
	cp $(BUILD_DIR_DEBUG)/compile_commands.json .

configure_release: fonts
	cmake -G "Ninja" -S. -B$(BUILD_DIR_RELEASE) -DGUI=ON
	# Copy compile commands db to root for lsp
	cp $(BUILD_DIR_RELEASE)/compile_commands.json .

configure_test:
	cmake -G "Ninja" -S. -B$(BUILD_DIR_TEST) -DDEBUG=ON -DUNIT_TESTING=ON
	# Copy compile commands db to root for lsp
	cp $(BUILD_DIR_TEST)/compile_commands.json .

cli_debug: configure_cli_debug ignore_build
	cmake --build $(BUILD_DIR_DEBUG)

cli_release: configure_cli ignore_build
	cmake --build $(BUILD_DIR_RELEASE)

$(GENERATED_SRC_DIR):
	mkdir -p $(GENERATED_SRC_DIR)

$(FONT_SOURCE): $(GENERATED_SRC_DIR)
	xxd -i fonts/DejaVuSansMono.ttf > $(FONT_SOURCE)

fonts: $(FONT_SOURCE)

debug: configure_debug ignore_build
	cmake --build $(BUILD_DIR_DEBUG)

release: configure_release ignore_build
	cmake --build $(BUILD_DIR_RELEASE)

test: configure_test ignore_build
	cmake --build $(BUILD_DIR_TEST)

run_test: test
	ctest --output-on-failure --test-dir $(BUILD_DIR_TEST)

lint:
	run-clang-tidy -j 8 -allow-no-checks

format:
	find src -iname '*.h' -o -iname '*.cpp' -o -iname '*.hpp' | clang-format --style=file -i --files=/dev/stdin

check-format:
	find src -iname '*.h' -o -iname '*.cpp' -o -iname '*.hpp' | clang-format --style=file -i -n --Werror --files=/dev/stdin

check: check-format lint

clean_generated:
	rm -rf $(GENERATED_SRC_DIR)

clean: clean_fonts
	rm -rf $(BUILD_DIR)

clean_3rdaprty:
	rm -rf $(3RDPARTY_DIR)

clean_all: clean clean_3rdaprty

all: test debug release cli_debug cli_release
