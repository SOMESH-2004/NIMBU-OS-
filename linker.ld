/* NIMBU OS Linker Script
 * Kernel loaded at physical 0x100000 (1 MB)
 * Higher-half kernel: kernel at 0xC0000000 (3 GB virtual)
 */

ENTRY(_start)

SECTIONS
{
    /* Physical address of first loadable segment */
    . = 0x100000;
    
    .text ALIGN(4096) : AT(ADDR(.text) - 0xC0000000)
    {
        *(.text)
        *(.text.*)
    }
    
    .rodata ALIGN(4096) :
    {
        *(.rodata)
        *(.rodata.*)
    }
    
    .data ALIGN(4096) :
    {
        *(.data)
        *(.data.*)
    }
    
    .bss ALIGN(4096) :
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
    
    /* Discard unused sections */
    /DISCARD/ :
    {
        *(.note.GNU-stack)
        *(.gnu_debuglink)
        *(.gnu_debugdata)
    }
}
