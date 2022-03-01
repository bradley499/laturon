INC=-I./include
CC?=gcc
DEBUG=false
ifeq ($(filter $(shell whereis emcc),$(CC)),$(CC))
EMCC=$(CC)
ARGS=-Wall -O3 -std=c99 -fno-exceptions -fno-rtti -s ENVIRONMENT=worker -s MALLOC=emmalloc -s DISABLE_EXCEPTION_CATCHING=1 -s FORCE_FILESYSTEM=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s MAXIMUM_MEMORY=4GB -s WASM_BIGINT -s ASYNCIFY --pre-js src/js/interact.js --post-js src/js/files.js
OBJ_ARGS=-Wall -std=c99 -fno-exceptions -fno-rtti
else
ARGS=-O2 -lm -Wall
OBJ_ARGS=-Wall
endif
INC:=$(INC)
SRCDIR=src
OBJDIR=obj
FILES=$(wildcard $(SRCDIR)/*.c)
OBJS=$(FILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
BUILD=build
EXECNAME=program
EXECEXTENSION=
ifeq ($(EMCC),$(CC))
EXECEXTENSION=.js
endif
VERSION_FILE=VERSION
VERSION=$(shell head -n 1 $(VERSION_FILE))
PREVIOUS_BUILDNUMBER=$(shell tail -n 1 $(VERSION_FILE))
BUILDNUMBER=$(shell echo $$(( $${c:-$(PREVIOUS_BUILDNUMBER)}+1 )))

all: default build_num $(OBJS) $(EXECNAME)

default: clean_objs setup

ifneq ($(EMCC),$(CC))
debug: ARGS += -g -ggdb
else
debug: ARGS += -g -fdebug-compilation-dir='..'
endif
debug: all

build_num:
	@echo "$(VERSION)\n$(BUILDNUMBER)" > $(VERSION_FILE)

setup: $(OBJDIR)
	@mkdir -pv $(BUILD)

$(EXECNAME):
	$(CC) $(wildcard $(OBJDIR)/*.o) $(ARGS) -o $(BUILD)/$(EXECNAME)-$(VERSION).$(BUILDNUMBER)$(EXECEXTENSION)
ifeq ($(EMCC),$(CC))
	@echo "{\n\
		\"author\": [\n\
			{\n\
				\"name\": \"Bradley Marshall\",\n\
				\"handle\": \"bradley499\"\n\
			}\n\
		],\n\
		\"version\": $(VERSION),\n\
		\"build\": $$(echo $(BUILDNUMBER)),\n\
		\"worker\": \"$(EXECNAME)-$(VERSION).$(BUILDNUMBER)$(EXECEXTENSION)\",\n\
		\"wasm\": \"$(EXECNAME)-$(VERSION).$(BUILDNUMBER).wasm\"\n\
	}" > $(BUILD)/interpreter.json
endif

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)
	$(CC) $(OBJ_ARGS) $(INC) -DVERSION=$(VERSION) -DBUILDNUMBER=$(BUILDNUMBER) -c $< -o $@

$(OBJDIR):
	@mkdir -pv $@

wasm_linkage:
	sed -i 's/wasmBinaryFile="$(shell cat CURRENT_BUILD)"/wasmBinaryFile="..\/wasm\/$(shell cat CURRENT_BUILD)"/' $(BUILD)/*.js

current_build:
	@echo "$(EXECNAME)-$(VERSION).$(BUILDNUMBER).wasm" > CURRENT_BUILD

clean_current_build:
	@rm -vf CURRENT_BUILD

run:
ifneq ($(EMCC),$(CC))
	@cd $(BUILD); ./$(EXECNAME);
else
	@cd $(BUILD); emrun $(EXECNAME)
endif

run_memory_check:
ifneq ($(EMCC),$(CC))
	@valgrind --leak-check=full --track-origins=yes --leak-check=full --show-leak-kinds=all -s $(BUILD)/$(EXEC)
else
	@echo "Error: valgrind memory checks can only be performed on native binaries"
	@failure
endif

clean: clean_current_build
	@rm -rvf $(BUILD) $(OBJDIR)

clean_objs: 
	@rm -rvf $(OBJDIR)
