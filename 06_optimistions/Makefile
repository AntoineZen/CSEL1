EXE=prime_opt
SRCS=$(wildcard *.c) 


TARGET := arm
TOOLCHAIN_PATH=~/workspace/xu3/buildroot/output/host/usr/bin/
ifeq ($(TARGET), arm)
	TOOLCHAIN=$(TOOLCHAIN_PATH)arm-linux-gnueabihf-
else
	TOOLCHAIN=/usr/bin/
endif

CC=$(TOOLCHAIN)gcc
LD=$(TOOLCHAIN)gcc
AR=$(TOOLCHAIN)ar
STRIP=$(TOOLCHAIN)strip
ifeq ($(TARGET), arm)
	CFLAGS+=-pedantic -Wall -Wextra -g -c -mcpu=cortex-a15.cortex-a7 -O2 -MD -std=gnu11
else
	CFLAGS+=-pedantic -Wall -Wextra -g -c -O2 -MD -std=gnu11 -pthread
endif
LDFLAGS=-pthread
OBJDIR=.obj/odroid
EXEC=$(EXE)

OBJS= $(addprefix $(OBJDIR)/, $(ASRC:.s=.o) $(SRCS:.c=.o))

all: $(OBJDIR)/ $(EXEC)
	
$(EXEC): $(OBJS) $(LINKER_SCRIPT)
	$(LD) $(OBJS) $(LDFLAGS) -o $@
	@echo Build success

$(OBJDIR)/%o: %c
	$(CC) $(CFLAGS) $< -o $@
	
$(OBJDIR)/:
	mkdir -p $(OBJDIR)

clean:
	rm -Rf $(OBJDIR) $(EXEC) $(EXEC)_s *~

clean_all: 
	rm -Rf .obj $(EXE)_a $(EXE)_h $(EXE)_a_s $(EXE)_h_s *~

deploy: $(EXEC)
	cp $(EXEC) ~/workspace

.PHONY: all clean clean_all

-include $(OBJS:.o=.d)


