org 0x7C00
bits 16

start:
    jmp short real_start
    nop

; ============ FAT16 BPB 占位符 ============
bpb_oem:            db "T.B.M.K. "
bpb_bytes_per_sec:  dw 512
bpb_sec_per_cluster: db 8
bpb_reserved:       dw 1
bpb_num_fats:       db 2
bpb_root_entries:   dw 512
bpb_total_sec16:    dw 0
bpb_media:          db 0xF8
bpb_sec_per_fat:    dw 16
bpb_sec_per_track:  dw 63
bpb_num_heads:      dw 16
bpb_hidden:         dd 0
bpb_total_sec32:    dd 0
bpb_drive_num:      db 0x80
bpb_reserved2:      db 0
bpb_ext_sig:        db 0x29
bpb_vol_id:         dd 0x12345678
bpb_vol_label:      db "TBMK_DISK  "
bpb_fs_type:        db "FAT16   "

real_start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7B00

    mov [boot_drive], dl
    mov byte [0x7F60], dl

    in al, 0x92
    or al, 2
    out 0x92, al

    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13

    ; CPU检测 (不变)
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

    mov dword [0x7F70], 0xB8000

    ; 根据启动驱动器选择 CHS 参数
    cmp byte [boot_drive], 0x80
    je hdd_boot

    ; ===== 软盘启动：LBA 1 = CHS (0/0/2) =====
    mov ax, 0x0201        ; AH=02(读), AL=01(先读1个扇区获取几何参数)
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    mov bx, 0x1000
    int 0x13
    jc disk_error

    ; 继续读剩余 49 个扇区
    mov ah, 0x02
    mov al, 49
    mov ch, 0
    mov cl, 3           ; 从扇区 3 继续
    mov dh, 0
    mov dl, [boot_drive]
    mov bx, 0x1200      ; 0x1000 + 512
    int 0x13
    jc disk_error
    jmp load_done

hdd_boot:
    ; ===== 硬盘启动：LBA 65 = 尝试用 CHS 读 =====
    ; QEMU 硬盘几何: 63 扇区/道, 16 磁头
    ; LBA 65 = 65/63 = 1 磁道余 2 → 柱面 0, 磁头 1, 扇区 3
    mov ax, 0x0201        ; 先读 1 个扇区测试
    mov ch, 0              ; 柱面 0
    mov cl, 3              ; 扇区 3
    mov dh, 1              ; 磁头 1
    mov dl, 0x80
    mov bx, 0x1000
    int 0x13
    jc try_hdd_chs2

    ; 继续读剩余 49 个扇区
    mov ah, 0x02
    mov al, 49
    mov ch, 0
    mov cl, 4
    mov dh, 1
    mov dl, 0x80
    mov bx, 0x1200
    int 0x13
    jc disk_error
    jmp load_done

try_hdd_chs2:
    ; 尝试柱面 1, 磁头 0, 扇区 2 (另一种 CHS 映射)
    mov ax, 0x0201
    mov ch, 1
    mov cl, 2
    mov dh, 0
    mov dl, 0x80
    mov bx, 0x1000
    int 0x13
    jc disk_error

    mov ah, 0x02
    mov al, 49
    mov ch, 1
    mov cl, 3
    mov dh, 0
    mov dl, 0x80
    mov bx, 0x1200
    int 0x13
    jc disk_error

load_done:
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