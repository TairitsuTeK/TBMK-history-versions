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

    ; 读取内核到 0x1000（20个扇区）
    mov ah, 0x02
    mov al, 40
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    mov bx, 0x1000
    int 0x13
    jc disk_error

    ; 关闭中断
    cli

    ; 加载GDT
    lgdt [gdt_ptr]

    ; 设置CR0的PE位
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 远跳转进入保护模式，跳转到内核入口
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

; GDT定义
align 4
gdt_start:
    dd 0, 0          ; 空描述符
    dd 0x0000FFFF, 0x00CF9A00    ; 代码段：基址0，界限4GB，DPL=0，32位
    dd 0x0000FFFF, 0x00CF9200    ; 数据段：基址0，界限4GB，DPL=0，32位
gdt_end:

gdt_ptr:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510 - ($ - $$) db 0
dw 0xAA55