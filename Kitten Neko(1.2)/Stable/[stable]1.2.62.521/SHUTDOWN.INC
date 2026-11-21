; 重启系统
reboot_system:
    cli
    mov al, 0xFE
    out KEYB_STATUS, al
    
restart_halt:
    hlt
    jmp restart_halt

; 蓝屏关机
shutdown_system:
    cli
    
    ; 填充蓝色背景
    mov edi, VGA_BASE
    mov ecx, TOTAL_CHARS
    mov ax, 0x1720
    rep stosw

    ; 输出提示文字
    mov esi, shut_txt
    mov edi, VGA_BASE + 1938
    mov ah, 0x4F
    call print_str

    ; 隐藏硬件光标
    mov dx, CRTC_ADDR
    mov al, 0x0A
    out dx, al
    mov dx, CRTC_DATA
    mov al, 0x20
    out dx, al

    ; 延时约3秒
    mov ecx, 3000
    
wait_loop1:
    mov edx, 0xA00000
    
wait_loop2:
    dec edx
    nop
    nop
    nop
    jnz wait_loop2
    dec ecx
    jnz wait_loop1

    ; 停机
    ud2
    
inf_halt:
    hlt
    jmp inf_halt