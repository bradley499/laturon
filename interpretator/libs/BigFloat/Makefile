GCC=gcc
LIBNAME=BigFloat_
INC=-I./include
SRCDIR=src
OBJDIR=obj
FILES=$(wildcard $(SRCDIR)/*.c)
OBJS=$(FILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: clean setup $(OBJS)

setup:
	@mkdir -pv $(OBJDIR)

clean:
	@rm -rvf $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(GCC) $(INC) -c $< -o $@ -g -ggdb