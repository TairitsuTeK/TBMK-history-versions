org 0x1000
bits 32
%include "MSG.INC"

start:
    ; 关闭闪烁光标
    mov dx, 0x3D4
    mov al, 0x0A
    out dx, al
    mov dx, 0x3D5
    mov al, 0x20
    out dx, al

    ; 常态清屏：黑色背景
    call cls

    ; 常态文字：黑底天蓝色
    mov esi, line1
    mov edi, 0xB8000
    mov ah, 0x0B
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

    mov edi, 0xB8000 + 1280
    mov esi, line9
    call print_str

; ------------------- 键盘检测主循环 -------------------
; 新增一个变量，记录Ctrl键状态
ctrl_pressed db 0

key_loop:
    in al, 0x64
    test al, 1
    jz key_loop
    in al, 0x60

    ; 处理Ctrl键按下/松开
    cmp al, 0x1D      ; Ctrl按下扫描码
    je ctrl_down
    cmp al, 0x9D      ; Ctrl松开扫描码
    je ctrl_up

    ; 处理C键按下（同时检查Ctrl是否按下）
    cmp al, 0x2E      ; C键按下扫描码
    je check_ctrl_and_c

    ; 保留你原来的按键功能
    cmp al, 0x13      ; R键（0x13是你的扫描码）
    je reboot_system

    jmp key_loop

ctrl_down:
    mov byte [ctrl_pressed], 1  ; 标记Ctrl已按下
    jmp key_loop

ctrl_up:
    mov byte [ctrl_pressed], 0  ; 标记Ctrl已松开
    jmp key_loop

check_ctrl_and_c:
    cmp byte [ctrl_pressed], 1  ; 检查Ctrl是否同时按下
    je shutdown_system          ; Ctrl+C同时按下，触发崩溃
    jmp key_loop                ; 只按C不按Ctrl，不处理

; ------------------- 系统重启函数 -------------------
reboot_system:
    mov al, 0xFE
    out 0x64, al
    jmp $

; ------------------- ESC关机界面：蓝背景红底白字 -------------------
; ------------------- ESC关机流程，蓝屏停留后崩溃 -------------------
shutdown_system:
    ; 填充蓝色背景
    mov edi, 0xB8000
    mov ecx, 2000
    mov ax, 0x1720
    rep stosw

    ; 打印居中文字
    mov esi, shut_txt
    mov edi, 0xB8000 + 1940
    mov ah, 0x4F
    call print_str

    ; 第一层：超大空循环拖住执行节奏
    mov ecx, 800
wait_loop1:
    mov edx, 0x700000
wait_loop2:
    dec edx
    nop
    nop
    jnz wait_loop2
    dec ecx
    jnz wait_loop1

    ; 停留完毕再崩溃
    cli
    ud2
    jmp $

; 常态清屏：纯黑背景
cls:
    mov edi, 0xB8000
    mov ecx, 2000
    mov ax, 0x0720
cls_loop:
    stosw
    loop cls_loop
    ret

; 长延时
delay_long:
    mov ebx, 0x400000
d1:
    dec ebx
    jnz d1
    ret

; 普通靠左打印
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