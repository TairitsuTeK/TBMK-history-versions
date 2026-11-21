bits 32
global _start
extern _kernel_main

section .text
_start:
    ; 设置数据段寄存器
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; 调用C内核
    call _kernel_main
    
    ; 如果C返回，停机
    cli
    hlt
    jmp $