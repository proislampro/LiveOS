ENTRY(_start)

SECTIONS
{
    . = 1M;

    .multiboot ALIGN(4) :
    {
        KEEP(*(.multiboot))
    }

    .text ALIGN(4) :
    {
        *(.text*)
    }

    .rodata ALIGN(4) :
    {
        *(.rodata*)
    }

    .data ALIGN(4) :
    {
        *(.data*)
    }

    .bss ALIGN(4) :
    {
        *(COMMON)
        *(.bss*)
    }
}
