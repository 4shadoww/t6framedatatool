BUILD_DIR=build

configure:
	cmake -G "Ninja" -S. -B$(BUILD_DIR)
	# Coopy compile commands db to root for lsp
	cp build/compile_commands.json .

build: configure
	cmake --build $(BUILD_DIR)

clean:
	rm -r $(BUILD_DIR)
