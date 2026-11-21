org 0x7C00
bits 16

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; 保存BIOS传递的驱动器号
    mov [boot_drive], dl

    ; 启用A20地址线
    in al, 0x92
    or al, 2
    out 0x92, al

    ; 重置磁盘系统
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13

    ; ============ CPU检测 ============
    ; 获取厂商字符串
    mov eax, 0
    cpuid
    mov [0x7F00], ebx
    mov [0x7F04], edx
    mov [0x7F08], ecx
    mov byte [0x7F0C], 0

    ; 获取CPU完整型号（CPUID 0x80000002 ~ 0x80000004）
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000004
    jb .no_cpu_name

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
    jmp .cpu_done

.no_cpu_name:
    mov byte [0x7F10], 0

.cpu_done:
    ; ============ CPU检测结束 ============

    ; ============ 内存检测 ============
    mov di, 0x5000
    xor ebx, ebx
    xor bp, bp
    mov dword [total_mem], 0

.mem_next:
    mov eax, 0xE820
    mov edx, 0x534D4150
    mov ecx, 24
    int 0x15
    jc .mem_done
    cmp eax, 0x534D4150
    jne .mem_done

    cmp dword [di + 16], 1
    jne .skip_add

    mov eax, [di + 8]
    add [total_mem], eax

.skip_add:
    add di, 24
    cmp ebx, 0
    jne .mem_next

.mem_done:
    mov eax, [total_mem]
    mov ecx, 1048576
    xor edx, edx
    div ecx
    mov [0x7E00], eax
    ; ============ 内存检测结束 ============

    ; 读取内核到 0x1000（40个扇区）
    mov ah, 0x02
    mov al, 40
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
.loop:
    lodsb
    test al, al
    jz hang
    int 0x10
    jmp .loop

hang:
    cli
.h:
    hlt
    jmp .h

boot_drive db 0
err_msg db 'Disk Error!', 0

total_mem dd 0

; GDT定义
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