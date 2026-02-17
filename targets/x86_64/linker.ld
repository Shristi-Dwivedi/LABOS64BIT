ENTRY(start)

SECTIONS
{
    . = 1M;

    .boot :
    {
        /* Force the header to the start */
        KEEP(*(.multiboot2))
    }

    .text :
    {
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss)
    }
}