INC=-I./include
CC?=gcc
ifeq ($(filter $(shell whereis emcc),$(CC)),$(CC))
EMCC=$(CC)
ARGS=-std=c99 -fno-exceptions -fno-rtti -s ENVIRONMENT=web -s MALLOC=dlmalloc -s DISABLE_EXCEPTION_CATCHING=1 -s NO_FILESYSTEM=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s MAXIMUM_MEMORY=4GB -s WASM_BIGINT -g -Wall -fdebug-compilation-dir='..'
else
ARGS=-Wall -g -ggdb
endif
LIBDIR=libs
LIBFILES=$(wildcard $(LIBDIR)/*/src/*.c)
LIBS=$(wildcard $(LIBDIR)/*)
INC:=$(INC) -I$(wildcard $(LIBDIR)/*/include)
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
.PHONY: $(LIBS)

all: default $(OBJS) $(EXEC)

default: clean_objs setup libraries

setup: $(OBJDIR)
	@mkdir -pv $(BUILD)

$(EXEC):
	$(CC) $(wildcard $(OBJDIR)/*.o) $(ARGS) -lm -o $(BUILD)/$(EXEC)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)
	$(CC) $(ARGS) $(INC) -c $< -o $@

libraries: $(LIBS)
	@mv libs/*/obj/* $(OBJDIR) -v

$(LIBS):
	cd $@ && $(MAKE)

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
