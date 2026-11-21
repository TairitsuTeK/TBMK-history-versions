; 清屏函数
cls:
    push edi
    push ecx
    push ax
    
    mov edi, VGA_BASE
    mov ecx, TOTAL_CHARS
    mov ax, 0x0720
    rep stosw
    
    pop ax
    pop ecx
    pop edi
    ret

; 字符串打印函数
; 输入: esi = 字符串地址, edi = VGA内存地址, ah = 颜色属性
print_str:
    push ax
    push esi
    push edi
    
print_loop:
    lodsb
    test al, al
    jz ret_print
    mov [edi], ax
    add edi, 2
    jmp print_loop
    
ret_print:
    pop edi
    pop esi
    pop ax
    ret

; 打印所有文本行
print_all_lines:
    push ax
    push esi
    push edi
    
    mov ah, 0x0B
    
    mov esi, l1
    mov edi, VGA_BASE
    call print_str
    
    mov esi, l2
    mov edi, VGA_BASE + 160
    call print_str
    
    mov esi, l3
    mov edi, VGA_BASE + 320
    call print_str
    
    mov esi, l4
    mov edi, VGA_BASE + 480
    call print_str
    
    mov esi, l5
    mov edi, VGA_BASE + 640
    call print_str
    
    mov esi, l6
    mov edi, VGA_BASE + 800
    call print_str
    
    mov esi, l7
    mov edi, VGA_BASE + 960
    call print_str
    
    mov esi, l8
    mov edi, VGA_BASE + 1120
    call print_str
    
    mov esi, l9
    mov edi, VGA_BASE + 1280
    call print_str
    
    mov esi, l10
    mov edi, VGA_BASE + 1440
    call print_str
    
    mov esi, l11
    mov edi, VGA_BASE + 1600
    call print_str
    
    mov esi, l12
    mov edi, VGA_BASE + 1760
    call print_str
    
    mov esi, l13
    mov edi, VGA_BASE + 1920
    call print_str
    
    mov esi, l14
    mov edi, VGA_BASE + 2080
    call print_str
    
    mov esi, l15
    mov edi, VGA_BASE + 2240
    call print_str

    mov esi, l16
    mov edi, VGA_BASE + 2400
    call print_str
    
    pop edi
    pop esi
    pop ax
    ret

; 设置初始光标位置（第16行首列）
set_initial_cursor:
    push dx
    push ax
    
    mov dx, CRTC_ADDR
    mov al, 0x0F
    out dx, al
    mov dx, CRTC_DATA
    mov al, 97
    out dx, al
    
    mov dx, CRTC_ADDR
    mov al, 0x0E
    out dx, al
    mov dx, CRTC_DATA
    mov al, 4
    out dx, al
    
    pop ax
    pop dx
    ret