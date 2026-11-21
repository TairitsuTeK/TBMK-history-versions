org 0x1000
bits 32

%include "DATA.INC"

TEXT_COLOR EQU 0x0B

start:
    ; 首次关闭闪烁光标
    call hide_cursor
    
    call cls

    ; 打印文字
    mov esi, line1
    mov edi, 0xB8000
    mov ah, TEXT_COLOR
    call print_str

    mov edi, 0xB8000 + 160
    mov esi, line2
    call print_str

    mov edi, 0xB8000 + 320
    mov esi, line3
    call print_str

    mov edi, 0xB8000 + 480
    mov esi, line4
    call print_str

    mov edi, 0xB8000 + 640
    mov esi, line5
    call print_str

    mov edi, 0xB8000 + 800
    mov esi, line6
    call print_str

    mov edi, 0xB8000 + 960
    mov esi, line7
    call print_str

    mov edi, 0xB8000 + 1120
    mov esi, line8
    call print_str

; ------------------- 键盘检测主循环 -------------------
key_loop:
    in al, 0x64
    test al, 1
    jz key_loop
    in al, 0x60

    cmp al, 0x13
    je reboot_system
    cmp al, 0x01
    je shutdown_system
    jmp key_loop

; ------------------- 系统重启函数 -------------------
reboot_system:
    mov al, 0xFE
    out 0x64, al
    jmp $

; ------------------- ESC关机流程（精准居中版） -------------------
shutdown_system:
    call cls
    call hide_cursor
    mov esi, shut_txt
    mov edi, 0xB8000 + 1954
    mov ah, 0x4F
    call print_str
    call delay_long
    cli
    hlt
    jmp $

; 清屏（全屏刷蓝底白字空格）
cls:
    mov edi, 0xB8000
    mov ecx, 2000
    mov ax, 0x0720
cls_loop:
    stosw
    loop cls_loop
    ret

; 隐藏光标独立子程序
hide_cursor:
    mov dx, 0x3D4
    mov al, 0x0A
    out dx, al
    mov dx, 0x3D5
    mov al, 0x20
    out dx, al
    ret

; 长延时
delay_long:
    mov ebx, 0x400000
d1:
    dec ebx
    jnz d1
    ret

; 普通靠左打印（esi=字符串, edi=位置, ah=颜色）
print_str:
print_loop:
    lodsb
    test al, al
    jz ret_print
    mov [edi], ax
    add edi, 2
    jmp print_loop
ret_print:
    ret