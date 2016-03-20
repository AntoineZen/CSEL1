EXE=app
SRCS=$(wildcard *.c) 

ifneq ($(TARGET),odroid)
CC=gcc
LD=gcc
STRIP=strip
CFLAGS=-Wall -Wextra -g -c -O0 -MD -std=c11 
OBJDIR=.obj/host
EXEC=$(EXE)_h

else
TOOLCHAIN_PATH=~/workspace/xu3/buildroot/output/host/usr/bin/
TOOLCHAIN=$(TOOLCHAIN_PATH)arm-linux-gnueabihf-
CC=$(TOOLCHAIN)gcc
LD=$(TOOLCHAIN)gcc
AR=$(TOOLCHAIN)ar
STRIP=$(TOOLCHAIN)strip
CFLAGS+=-pedantic -Wall -Wextra -g -c -mcpu=cortex-a15.cortex-a7 -O0 -MD -std=gnu11 
OBJDIR=.obj/odroid
EXEC=$(EXE)_a
endif

OBJS= $(addprefix $(OBJDIR)/, $(ASRC:.s=.o) $(SRCS:.c=.o))

all: $(OBJDIR)/ $(EXEC)
	
$(EXEC): $(OBJS) $(LINKER_SCRIPT)
	$(LD) $(OBJS) $(LDFLAGS) -o $@

$(OBJDIR)/%o: %c
	$(CC) $(CFLAGS) $< -o $@
	
$(OBJDIR)/:
	mkdir -p $(OBJDIR)

clean:
	rm -Rf $(OBJDIR) $(EXEC) $(EXEC)_s *~

clean_all: 
	rm -Rf .obj $(EXE)_a $(EXE)_h $(EXE)_a_s $(EXE)_h_s *~


.PHONY: all clean clean_all

-include $(OBJS:.o=.d)


