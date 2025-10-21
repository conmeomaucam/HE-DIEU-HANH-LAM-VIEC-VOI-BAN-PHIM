TARGET = eonos
LINKER_SCRIPT = linker.ld
# LUU Y: Loai bo shell.c de tap trung vao bai kiem tra da nhiem cot loi
SOURCES_C = main.c uart.c kernel.c heap.c task.c
SOURCES_S = startup.s
CC = arm-none-eabi-gcc
CPU_FLAGS = -mcpu=cortex-m3 -mthumb
CFLAGS = $(CPU_FLAGS) -nostdlib -ffreestanding -g -O0
OBJECTS = $(SOURCES_C:.c=.o) $(SOURCES_S:.s=.o)
all: $(TARGET).elf
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
