org 0x1000
bits 32

STACK_BASE  equ 0x90000

section .data
ctrl_pressed  db  0
%include "MSG.INC"

section .text

start:
    mov esp, STACK_BASE
    mov ebp, esp

    ; 关闭闪烁光标
    mov dx, 0x3D4
    mov al, 0x0A
    out dx, al
    mov dx, 0x3D5
    mov al, 0x20
    out dx, al

    call cls

    ; 批量打印文本
    mov ah, 0x0B
    mov esi, l1
    mov edi, 0xB8000
    call print_str

    mov esi, l2
    mov edi, 0xB8000 + 160
    call print_str
    mov esi, l3
    mov edi, 0xB8000 + 320
    call print_str
    mov esi, l4
    mov edi, 0xB8000 + 480
    call print_str
    mov esi, l5
    mov edi, 0xB8000 + 640
    call print_str
    mov esi, l6
    mov edi, 0xB8000 + 800
    call print_str
    mov esi, l7
    mov edi, 0xB8000 + 960
    call print_str
    mov esi, l8
    mov edi, 0xB8000 + 1120
    call print_str
    mov esi, l9
    mov edi, 0xB8000 + 1280
    call print_str
    mov esi, l10
    mov edi, 0xB8000 + 1440
    call print_str

; 键盘主循环
key_loop:
    in al, 0x64
    test al, 1
    jz key_loop
    in al, 0x60

    cmp al, 0x1D
    je ctrl_down
    cmp al, 0x9D
    je ctrl_up
    cmp al, 0x2E
    je check_ctrl_and_c
    cmp al, 0x13
    je reboot_system

    jmp key_loop

ctrl_down:
    mov byte [ctrl_pressed], 1
    jmp key_loop
ctrl_up:
    mov byte [ctrl_pressed], 0
    jmp key_loop

check_ctrl_and_c:
    cmp byte [ctrl_pressed], 1
    je shutdown_system
    jmp key_loop

; 重启逻辑
reboot_system:
    cli
    mov al, 0xFE
    out 0x64, al
restart_halt:
    hlt
    jmp restart_halt

; 蓝屏界面修复核心
shutdown_system:
    cli
    ; 填充蓝色背景
    mov edi, 0xB8000
    mov ecx, 2000
    mov ax, 0x1720
    rep stosw

    ; 输出提示文字
    mov esi, shut_txt
    mov edi, 0xB8000 + 1938
    mov ah, 0x4F
    call print_str

    ; 加长延时，保证画面稳定显示3秒左右
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

    ; 停留完毕再停机
    ud2
inf_halt:
    hlt
    jmp inf_halt

; 清屏函数
cls:
    mov edi, 0xB8000
    mov ecx, 2000
    mov ax, 0x0720
    rep stosw
    ret

; 字符串打印
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