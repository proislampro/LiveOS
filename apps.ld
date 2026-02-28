/* Linker script for applications */

MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 256K
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

SECTIONS
{
    .text :
    {
        *(.text*)
        *(.rodata*)
    } > FLASH

    .data :
    {
        *(.data*)
    } > RAM AT > FLASH

    .bss :
    {
        *(.bss*)
        *(COMMON)
    } > RAM

    .heap :
    {
        . = ALIGN(8);
        _heap_start = .;
        . += 8K;
        _heap_end = .;
    } > RAM

    .stack :
    {
        . = ALIGN(8);
        _stack_end = .;
        . += 8K;
        _stack_start = .;
    } > RAM
}