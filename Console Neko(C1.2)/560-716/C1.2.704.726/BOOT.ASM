org 0x7C00
bits 16

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [boot_drive], dl
    mov byte [0x7F60], dl

    in al, 0x92
    or al, 2
    out 0x92, al

    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13

    ; ============ CPU检测 ============
    mov eax, 0
    cpuid
    mov [0x7F00], ebx
    mov [0x7F04], edx
    mov [0x7F08], ecx
    mov byte [0x7F0C], 0

    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000004
    jb no_cpu_name

    mov eax, 0x80000002
    cpuid
    mov [0x7F10], eax
    mov [0x7F14], ebx
    mov [0x7F18], ecx
    mov [0x7F1C], edx

    mov eax, 0x80000003
    cpuid
    mov [0x7F20], eax
    mov [0x7F24], ebx
    mov [0x7F28], ecx
    mov [0x7F2C], edx

    mov eax, 0x80000004
    cpuid
    mov [0x7F30], eax
    mov [0x7F34], ebx
    mov [0x7F38], ecx
    mov [0x7F3C], edx

    mov byte [0x7F40], 0
    jmp cpu_done

no_cpu_name:
    mov byte [0x7F10], 0
cpu_done:

    ; 默认文本模式
    mov dword [0x7F70], 0xB8000

    ; 读取内核50扇区
    mov ah, 0x02
    mov al, 50
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    mov bx, 0x1000
    int 0x13
    jc disk_error

    cli
    lgdt [gdt_ptr]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:0x1000

disk_error:
    mov si, err_msg
    mov ah, 0x0E
print_loop:
    lodsb
    test al, al
    jz hang
    int 0x10
    jmp print_loop
hang:
    cli
halt_loop:
    hlt
    jmp halt_loop

boot_drive db 0
err_msg db 'Disk Error!', 0

align 4
gdt_start:
    dd 0, 0
    dd 0x0000FFFF, 0x00CF9A00
    dd 0x0000FFFF, 0x00CF9200
gdt_end:
gdt_ptr:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510 - ($ - $$) db 0
dw 0xAA55