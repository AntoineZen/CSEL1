EXEC=app
SRCS=$(wildcard *.c) 

TOOLCHAIN_PATH=~/workspace/xu3/buildroot/output/host/usr/bin/
TOOLCHAIN=$(TOOLCHAIN_PATH)arm-linux-gnueabihf-
CFLAGS+=-pedantic -Wall -Wextra -g -c -mcpu=cortex-a15.cortex-a7 -Og -MD -std=gnu11
OBJDIR=.obj

CC=$(TOOLCHAIN)gcc
LD=$(TOOLCHAIN)gcc
AR=$(TOOLCHAIN)ar
STRIP=$(TOOLCHAIN)strip
OBJS+=$(addprefix $(OBJDIR)/, $(SRCS:.c=.o))

.PHONY: all clean 

$(OBJDIR)/%o: %c
	$(CC) $(CFLAGS) $< -o $@
	

all: $(OBJDIR)/ $(EXEC)

clean: $(EXTRA_CLEAN)
	rm -Rf .obj 
	rm -Rf $(EXEC) *.map *~ 

$(EXEC): $(OBJS) $(LINKER_SCRIPT)
	$(LD) $(OBJS) $(LDFLAGS) -o $@ 
			
$(OBJDIR)/:
	mkdir -p $(OBJDIR)

-include $(OBJS:.o=.d)


