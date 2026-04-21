/* Linker script for STM32F407VG (1MB Flash, 192KB SRAM) */

SEARCH_DIR("/usr/arm-none-eabi/newlib/thumb/v7e-m+fp/hard");
SEARCH_DIR("/usr/arm-none-eabi/lib");

ENTRY(Reset_Handler)

_estack = 0x20030000;

MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 0x100000
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x30000
}

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        _svector = .;
        KEEP(*(.isr_vector))
        . = ALIGN(4);
        _evector = .;
    } >FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text) *(.text.*)
        *(.rodata) *(.rodata.*)
        . = ALIGN(4);
        _etext = .;
    } >FLASH

    .ARM.exidx :
    {
        *(.ARM.exidx*) *(.gnu.linkonce.armexidx.*)
    } >FLASH

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data) *(.data.*)
        . = ALIGN(4);
        _edata = .;
    } >SRAM AT>FLASH

    _sidata = LOADADDR(.data);

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss) *(.bss.*) *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } >SRAM

    _end = .;

    /DISCARD/ :
    {
        libc.a (*)
        libm.a (*)
        libgcc.a (*)
    }
}
