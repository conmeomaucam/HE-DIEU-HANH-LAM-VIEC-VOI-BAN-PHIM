ENTRY(_reset_handler)
MEMORY {
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}
_estack = ORIGIN(SRAM) + LENGTH(SRAM);
SECTIONS {
    .text : { KEEP(*(.vectors)) *(.text*) *(.rodata*) } > FLASH
    _sidata = LOADADDR(.data);
    .data : { . = ALIGN(4); _sdata = .; *(.data*) _edata = .; } > SRAM AT> FLASH
    .bss : { . = ALIGN(4); _sbss = .; *(.bss*) *(COMMON) _ebss = .; } > SRAM
    . = ALIGN(4);
    _heap_start = .;
}
