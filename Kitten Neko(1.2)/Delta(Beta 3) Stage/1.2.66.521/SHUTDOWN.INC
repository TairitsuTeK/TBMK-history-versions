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
    
    mov edi, VGA_BASE
    mov ecx, TOTAL_CHARS
    mov ax, 0x1720
    rep stosw

    mov esi, shut_txt
    mov edi, VGA_BASE + 1938
    mov ah, 0x4F
    call print_str

    mov dx, CRTC_ADDR
    mov al, 0x0A
    out dx, al
    mov dx, CRTC_DATA
    mov al, 0x20
    out dx, al

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

    ud2
    
inf_halt:
    hlt
    jmp inf_halt