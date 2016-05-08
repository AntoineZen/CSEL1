OBJDIR=.obj/odroid

OBJS= $(addprefix $(OBJDIR)/, $(ASRC:.s=.o) $(SRCS:.c=.o))

all: $(OBJDIR)/ pwm_master pwm_slave


$(OBJDIR)/:
	mkdir -p $(OBJDIR)

include master.mk
include slave.mk

clean:
	rm -Rf $(OBJDIR) pwm_master pwm_slave s *~

clean_all: 
	rm -Rf .obj pwm_master pwm_slave *~

deploy: pwm_master pwm_slave
	cp pwm_master pwm_slave ~/workspace

.PHONY: all clean clean_all



