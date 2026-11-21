org 0x7C00
bits 16

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov ah, 0x02
    mov al, 20
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov bx, 0x1000
    int 0x13

    lgdt [gdt_ptr]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:protected_mode

gdt:
gdt_null: dd 0,0
gdt_code: dw 0xFFFF,0
          db 0,0b10011010,0b11001111,0
gdt_data: dw 0xFFFF,0
          db 0,0b10010010,0b11001111,0
gdt_end:

gdt_ptr:
    dw gdt_end - gdt - 1
    dd gdt

bits 32
protected_mode:
    cli        ; 必加！锁住所有中断永不触发
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x80000
    jmp 0x1000

times 510-($-$$) db 0
dw 0xAA55