ENTRY(_start)

SECTIONS
{
    /* Start at 1MB (0x100000) */
    . = 0x100000;

    /* Limine / Multiboot header at the very start */
    .multiboot ALIGN(0x1000) : 
    {
        KEEP(*(.multiboot))
    }

    /* Kernel text (code) */
    .text ALIGN(0x1000) :
    {
        *(.text*)
    }

    /* Read-only data */
    .rodata ALIGN(0x1000) :
    {
        *(.rodata*)
    }

    /* Initialized data */
    .data ALIGN(0x1000) :
    {
        *(.data*)
    }

    /* Uninitialized data / BSS */
    .bss ALIGN(0x1000) :
    {
        *(COMMON)
        *(.bss*)
    }

    /* Stack (optional) */
    .stack ALIGN(0x1000) :
    {
        stack_start = .;
        . += 0x4000; /* 16 KB stack */
        stack_end = .;
    }
}
