org 0x7C00
bits 16

start:
    jmp short real_start
    nop

; ============ FAT16 BPB（BIOS需要）============
bpb_oem:            db "T.B.M.K. "
bpb_bytes_per_sec:  dw 512
bpb_sec_per_cluster: db 1
bpb_reserved:       dw 1
bpb_num_fats:       db 2
bpb_root_entries:   dw 224
bpb_total_sec16:    dw 2880
bpb_media:          db 0xF0
bpb_sec_per_fat:    dw 9
bpb_sec_per_track:  dw 18
bpb_num_heads:      dw 2
bpb_hidden:         dd 0
bpb_total_sec32:    dd 0
bpb_drive_num:      db 0x00
bpb_reserved2:      db 0
bpb_ext_sig:        db 0x29
bpb_vol_id:         dd 0x12345678
bpb_vol_label:      db "TBMK_GUI   "
bpb_fs_type:        db "FAT12   "

real_start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7B00
    mov [boot_drive], dl

    in al, 0x92
    or al, 2
    out 0x92, al

    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13

    ; 读取内核
    mov ax, 0x0210
    mov cx, 0x0002
    mov dh, 0
    mov dl, [boot_drive]
    mov bx, 0x1000
    int 0x13
    jc disk_error

load_done:
    mov ax, 0x0013
    int 0x10

enter_protected:
    cli
    lgdt [gdt_ptr]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:0x1000

disk_error:
    mov si, err
    mov ah, 0x0E
.l2:
    lodsb
    test al, al
    jz $
    int 0x10
    jmp .l2

boot_drive db 0
err db 'E', 0

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