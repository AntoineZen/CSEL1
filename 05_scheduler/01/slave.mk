EXE=pwm_slave
SRCS=$(wildcard *.c) 


TOOLCHAIN_PATH=~/workspace/xu3/buildroot/output/host/usr/bin/
TOOLCHAIN=$(TOOLCHAIN_PATH)arm-linux-gnueabihf-
CC=$(TOOLCHAIN)gcc
LD=$(TOOLCHAIN)gcc
AR=$(TOOLCHAIN)ar
STRIP=$(TOOLCHAIN)strip
CFLAGS+=-pedantic -Wall -Wextra -g -c -mcpu=cortex-a15.cortex-a7 -O0 -MD -std=gnu11 
OBJDIR=.obj/odroid
EXEC=$(EXE)

OBJS_SLAVE= $(addprefix $(OBJDIR)/, slave.o)


$(EXEC): $(OBJS_SLAVE) $(LINKER_SCRIPT)
	$(LD) $(OBJS_SLAVE) $(LDFLAGS) -o $@
	@echo Build success

$(OBJDIR)/%o: %c
	$(CC) $(CFLAGS) $< -o $@
	


-include $(OBJS:.o=.d)


