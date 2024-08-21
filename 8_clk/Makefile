CROSS_COMPILE 	?= arm-linux-gnueabihf-
TARGET 			?= clk
CC				:= $(CROSS_COMPILE)gcc
LD				:= $(CROSS_COMPILE)ld
OBJCOPY			:= $(CROSS_COMPILE)objcopy
OBJDUMP			:= $(CROSS_COMPILE)objdump

INCUDIRS		:= 	imx6u 		\
					bsp/clk		\
					bsp/delay	\
					bsp/led		\
					bsp/beep	\
					bsp/key		\
					bsp/gpio

SRCDIRS			:=	project		\
					bsp/clk		\
					bsp/delay	\
					bsp/led		\
					bsp/beep	\
					bsp/key		\
					bsp/gpio

INCLUDE			:= $(patsubst %, -I %, $(INCUDIRS))

SFILES			:= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.S))
CFILES			:= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))

SFILENDIR		:= $(notdir $(SFILES))
CFILENDIR		:= $(notdir $(CFILES))

SOBJS			:= $(patsubst %, obj/%, $(SFILENDIR:.S=.o))
COBJS			:= $(patsubst %, obj/%, $(CFILENDIR:.c=.o))

OBJS 			:= $(SOBJS)$(COBJS)

VPATH			:= $(SRCDIRS)

$(TARGET).bin: $(OBJS)
	$(LD) -Timx6u.lds $^ -o $(TARGET).elf
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).dis

$(COBJS): obj/%.o : %.c
	$(CC) $(INCLUDE) -c -Wall -nostdlib -O2 -o $@ $<
$(SOBJS): obj/%.o : %.S
	$(CC) $(INCLUDE) -c -Wall -nostdlib -O2 -o $@ $<

.PHONY:clean
clean:
	rm -rf $(OBJS) $(TARGET).elf $(TARGET).bin $(TARGET).dis load.imx

	