INC=-I./include
GCC=gcc
EMCC=emcc
ARGS=-Wall -g -ggdb
WASM_ARGS=-fno-exceptions -fno-rtti -s ENVIRONMENT=web -s MALLOC=dlmalloc -s DISABLE_EXCEPTION_CATCHING=1 -s NO_FILESYSTEM=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s MAXIMUM_MEMORY=4GB -s WASM_BIGINT -g
LIBS=
SRCDIR=src
OBJDIR=obj
FILES=$(wildcard $(SRCDIR)/*.c)
OBJS=$(FILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
BUILD=build
EXEC=program

all: clean_objs setup $(OBJS) $(EXEC)

setup:
	@mkdir -pv $(BUILD)

$(EXEC):
	$(GCC) $(OBJS) $(ARGS) -lm -o $(BUILD)/$(EXEC)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)
	$(GCC) $(ARGS) $(INC) -c $< -o $@

$(OBJDIR):
	@mkdir -pv $@

wasm:
	@mkdir -pv $(BUILD)
	$(EMCC) -Wall -std=c99 $(INC) $(LIBS) $(FILES) $(WASM_ARGS) -o $(BUILD)/$(EXEC).html -fdebug-compilation-dir='..'

wasm_run:
	@cd $(BUILD); emrun $(EXEC).html

run:
	@cd $(BUILD);./$(EXEC);

run_memory_check:
	@valgrind --leak-check=full --track-origins=yes --leak-check=full --show-leak-kinds=all -s $(BUILD)/$(EXEC)

clean:
	@rm -rvf $(BUILD) $(OBJDIR)

clean_objs: 
	@rm -rvf $(OBJDIR)
