INC=-I./include
CC?=gcc
DEBUG=false
ifeq ($(filter $(shell whereis emcc),$(CC)),$(CC))
EMCC=$(CC)
ARGS=-O3 -std=c99 -fno-exceptions -fno-rtti -s ENVIRONMENT=web -s MALLOC=emmalloc -s DISABLE_EXCEPTION_CATCHING=1 -s FORCE_FILESYSTEM=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s MAXIMUM_MEMORY=4GB -s WASM_BIGINT -s ASYNCIFY --pre-js src/js/interact.js
OBJ_ARGS=-std=c99 -fno-exceptions -fno-rtti
else
ARGS=-O2 -lm -Wall
OBJ_ARGS=$(ARGS)
endif
INC:=$(INC)
SRCDIR=src
OBJDIR=obj
FILES=$(wildcard $(SRCDIR)/*.c)
OBJS=$(FILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
BUILD=build
ifneq ($(EMCC),$(CC))
EXEC=program
else
EXEC=program.html
endif

all: default $(OBJS) $(EXEC)

default: clean_objs setup

ifneq ($(EMCC),$(CC))
debug: ARGS += -Wall -g -ggdb
else
debug: ARGS += -g -Wall -fdebug-compilation-dir='..'
endif
debug: all

setup: $(OBJDIR)
	@mkdir -pv $(BUILD)

$(EXEC):
	$(CC) $(wildcard $(OBJDIR)/*.o) $(ARGS) -o $(BUILD)/$(EXEC)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)
	$(CC) $(OBJ_ARGS) $(INC) -c $< -o $@

$(OBJDIR):
	@mkdir -pv $@

run:
ifneq ($(EMCC),$(CC))
	@cd $(BUILD); ./$(EXEC);
else
	@cd $(BUILD); emrun $(EXEC)
endif

run_memory_check:
ifneq ($(EMCC),$(CC))
	@valgrind --leak-check=full --track-origins=yes --leak-check=full --show-leak-kinds=all -s $(BUILD)/$(EXEC)
else
	@echo "Error: valgrind memory checks can only be performed on native binaries"
	@failure
endif

clean:
	@rm -rvf $(BUILD) $(OBJDIR)

clean_objs: 
	@rm -rvf $(OBJDIR)
