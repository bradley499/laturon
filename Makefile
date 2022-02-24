ifneq ($(filter $(shell whereis emcc),$(CC)),$(CC))
$(error Unable to compile to WebAssembly (WASM) as Emscripten toolchain is not loaded)
endif

BUILD=build

.PHONY: ide interpreter

all: default

default: clean setup ide interpreter_linkage

ide:
	@cp -vr ide/* $(BUILD)

interpreter:
	@$(MAKE) -C interpreter all current_build

interpreter_linkage: interpreter
	@$(MAKE) -C interpreter wasm_linkage
	@cp -vr interpreter/build/*.js* $(BUILD)/assets/js
	@cp -vr interpreter/build/*.wasm $(BUILD)/assets/wasm

setup:
	@mkdir -pv $(BUILD)
	@mkdir -pv $(BUILD)/assets/
	@mkdir -pv $(BUILD)/assets/js
	@mkdir -pv $(BUILD)/assets/wasm

clean:
	@rm -rvf $(BUILD)
	@$(MAKE) -C interpreter clean 