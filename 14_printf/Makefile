CROSS_COMPILE 	?= arm-linux-gnueabihf-
TARGET 			?= uart
CC				:= $(CROSS_COMPILE)gcc
LD				:= $(CROSS_COMPILE)ld
OBJCOPY			:= $(CROSS_COMPILE)objcopy
OBJDUMP			:= $(CROSS_COMPILE)objdump

LIBPATH			:= -lgcc -L /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/lib/gcc/arm-linux-gnueabihf/4.9.4

INCUDIRS		:= 	imx6u 		\
					bsp/clk		\
					bsp/delay	\
					bsp/led		\
					bsp/beep	\
					bsp/key		\
					bsp/gpio	\
					bsp/int		\
					bsp/exit	\
					bsp/epit	\
					bsp/keyfilter\
					bsp/uart	\
					stdio/include

SRCDIRS			:=	project		\
					bsp/clk		\
					bsp/delay	\
					bsp/led		\
					bsp/beep	\
					bsp/key		\
					bsp/gpio	\
					bsp/int		\
					bsp/exit	\
					bsp/epit	\
					bsp/keyfilter\
					bsp/uart	\
					stdio/lib

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
	$(LD) -Timx6u.lds -o $(TARGET).elf $^ $(LIBPATH)
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).dis

$(COBJS): obj/%.o : %.c
	$(CC) $(INCLUDE) -Wa,-mimplicit-it=thumb -fno-builtin -c -Wall -nostdlib -O2 -o $@ $<
$(SOBJS): obj/%.o : %.S
	$(CC) $(INCLUDE) -fno-builtin -c -Wall -nostdlib -O2 -o $@ $<

.PHONY:clean
clean:
	rm -rf $(OBJS) $(TARGET).elf $(TARGET).bin $(TARGET).dis load.imx

	