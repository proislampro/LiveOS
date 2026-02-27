ENTRY(main)

MEMORY
{
    RAM : ORIGIN = 0x200000, LENGTH = 64M
}

SECTIONS
{
    .text : {
        *(.text*)
        *(.rodata*)
    } > RAM

    .data : {
        *(.data*)
    } > RAM

    .bss : {
        *(.bss*)
        *(COMMON)
    } > RAM
}