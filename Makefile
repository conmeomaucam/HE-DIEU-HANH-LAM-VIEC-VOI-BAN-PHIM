TARGET = eonos
LINKER_SCRIPT = linker.ld

SOURCES_C = main.c uart.c kernel.c heap.c task.c
SOURCES_S = startup.s
CC = arm-none-eabi-gcc

CFLAGS = -mcpu=cortex-m3 -mthumb -nostdlib -ffreestanding -g -O0
OBJECTS = $(SOURCES_C:.c=.o) $(SOURCES_S:.s=.o)

$(TARGET).elf: $(OBJECTS)
	$(CC) $(CFLAGS) -T $(LINKER_SCRIPT) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

qemu: all
	qemu-system-arm -M lm3s6965evb -kernel $(TARGET).elf -serial stdio -monitor none

clean:
	rm -f *.o *.elf

.PHONY: all qemu clean

