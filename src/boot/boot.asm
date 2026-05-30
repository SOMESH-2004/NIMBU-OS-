/**
 * @file boot.asm
 * @brief x86 bootloader entry point
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Minimal bootloader that switches CPU to protected mode and jumps to kernel.
 * This assumes the bootloader has already been loaded by GRUB or similar.
 * 
 * The kernel expects:
 * - CPU in protected mode (GDT already loaded)
 * - Interrupts disabled
 * - Paging disabled
 * - Kernel loaded at physical address 0x100000
 */

[BITS 32]
[EXTERN kmain]
[GLOBAL _start]

; Multiboot header
ALIGN 4
MULTIBOOT_HEADER:
    MULTIBOOT_MAGIC     EQU 0x1BADB002
    MULTIBOOT_FLAGS     EQU 0x00000001
    MULTIBOOT_CHECKSUM  EQU -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)
    
    DD MULTIBOOT_MAGIC
    DD MULTIBOOT_FLAGS
    DD MULTIBOOT_CHECKSUM

; Kernel stack
ALIGN 4096
kernel_stack:
    RESB 8192   ; 8 KB kernel stack

; Entry point
_start:
    ; Set up stack pointer
    MOV ESP, kernel_stack + 8192
    
    ; Disable interrupts
    CLI
    
    ; Clear EFLAGS register
    XOR EAX, EAX
    PUSH EAX
    POPFD
    
    ; Call kernel main function
    CALL kmain
    
    ; If we return, halt
    CLI
    HLT
    JMP $

; Global descriptor table (temporary, will be replaced by kernel)
ALIGN 8
gdt_begin:
    ; Null descriptor
    DQ 0
    
    ; Code descriptor
    DW 0xFFFF           ; Limit
    DW 0x0000           ; Base low
    DB 0x00             ; Base mid
    DB 10011010B        ; Access byte (code, ring 0)
    DB 11001111B        ; Flags and limit high
    DB 0x00             ; Base high
    
    ; Data descriptor
    DW 0xFFFF           ; Limit
    DW 0x0000           ; Base low
    DB 0x00             ; Base mid
    DB 10010010B        ; Access byte (data, ring 0)
    DB 11001111B        ; Flags and limit high
    DB 0x00             ; Base high
    
gdt_end:

gdt_ptr:
    DW gdt_end - gdt_begin - 1
    DD gdt_begin
