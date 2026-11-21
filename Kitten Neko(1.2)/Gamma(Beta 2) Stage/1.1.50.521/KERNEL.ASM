org 0x1000
bits 32

STACK_BASE  equ 0x90000

section .data
ctrl_pressed  db  0
text_color    db  0x0B    ; 默认亮青色字体
bg_color      db  0x00    ; 默认黑色背景
%include "MSG.INC"

section .text

start:
    mov esp, STACK_BASE
    mov ebp, esp

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
    mov esi, l11
    mov edi, 0xB8000 + 1600
    call print_str
    mov esi, l12
    mov edi, 0xB8000 + 1760
    call print_str
    mov esi, l13
    mov edi, 0xB8000 + 1920
    call print_str
    mov esi, l14
    mov edi, 0xB8000 + 2080
    call print_str
    mov esi, l15
    mov edi, 0xB8000 + 2240
    call print_str

; ========== 光标移动到10行文本下一行(第11行首列) ==========
    mov dx, 0x3D4
    mov al, 0x0F        ; 设置光标位置低字节寄存器
    out dx, al
    mov dx, 0x3D5
    mov al, 97          ; 偏移1121低8位
    out dx, al

    mov dx, 0x3D4
    mov al, 0x0E        ; 设置光标位置高字节寄存器
    out dx, al
    mov dx, 0x3D5
    mov al, 4           ; 偏移1121高8位
    out dx, al
; ============================================================

; 键盘主循环
key_loop:
    in al, 0x64
    test al, 1
    jz key_loop
    in al, 0x60

    ; Ctrl按键按下/松开
    cmp al, 0x1D
    je ctrl_down
    cmp al, 0x9D
    je ctrl_up
    ; Ctrl + C 关机
    cmp al, 0x2E
    je check_ctrl_and_c
    ; Ctrl + R 重启，扫描码0x13
    cmp al, 0x13
    je check_ctrl_and_r

    ; ========== F1-F12 功能键处理 ==========
    ; 先检查F1-F12扫描码范围，统一入口
    cmp al, 0x3B        ; F1
    jb not_f_key
    cmp al, 0x44        ; F10
    jbe is_f_key
    cmp al, 0x57        ; F11
    je is_f_key
    cmp al, 0x58        ; F12
    je is_f_key
    jmp not_f_key

is_f_key:
    ; 判断Ctrl是否按下
    cmp byte [ctrl_pressed], 1
    je set_bg_color     ; Ctrl按下 → 设置背景色
    jmp set_text_color  ; Ctrl未按下 → 设置字体色

not_f_key:
    jmp key_loop

; ========== 字体颜色设置 ==========
set_text_color:
    cmp al, 0x3B
    je text_F1
    cmp al, 0x3C
    je text_F2
    cmp al, 0x3D
    je text_F3
    cmp al, 0x3E
    je text_F4
    cmp al, 0x3F
    je text_F5
    cmp al, 0x40
    je text_F6
    cmp al, 0x41
    je text_F7
    cmp al, 0x42
    je text_F8
    cmp al, 0x43
    je text_F9
    cmp al, 0x44
    je text_F10
    cmp al, 0x57
    je text_F11
    cmp al, 0x58
    je text_F12
    jmp key_loop

text_F1:
    mov byte [text_color], 0x0C    ; 亮红
    jmp apply_color_to_main
text_F2:
    mov byte [text_color], 0x0A    ; 亮绿
    jmp apply_color_to_main
text_F3:
    mov byte [text_color], 0x0E    ; 亮黄
    jmp apply_color_to_main
text_F4:
    mov byte [text_color], 0x09    ; 亮蓝
    jmp apply_color_to_main
text_F5:
    mov byte [text_color], 0x0D    ; 亮品红
    jmp apply_color_to_main
text_F6:
    mov byte [text_color], 0x0B    ; 亮青
    jmp apply_color_to_main
text_F7:
    mov byte [text_color], 0x0F    ; 亮白
    jmp apply_color_to_main
text_F8:
    mov byte [text_color], 0x02    ; 绿
    jmp apply_color_to_main
text_F9:
    mov byte [text_color], 0x04    ; 红
    jmp apply_color_to_main
text_F10:
    mov byte [text_color], 0x01    ; 蓝
    jmp apply_color_to_main
text_F11:
    mov byte [text_color], 0x05    ; 品红
    jmp apply_color_to_main
text_F12:
    mov byte [text_color], 0x03    ; 青
    jmp apply_color_to_main

; ========== 背景颜色设置 ==========
set_bg_color:
    cmp al, 0x3B
    je bg_F1
    cmp al, 0x3C
    je bg_F2
    cmp al, 0x3D
    je bg_F3
    cmp al, 0x3E
    je bg_F4
    cmp al, 0x3F
    je bg_F5
    cmp al, 0x40
    je bg_F6
    cmp al, 0x41
    je bg_F7
    cmp al, 0x42
    je bg_F8
    cmp al, 0x43
    je bg_F9
    cmp al, 0x44
    je bg_F10
    cmp al, 0x57
    je bg_F11
    cmp al, 0x58
    je bg_F12
    jmp key_loop

bg_F1:
    mov byte [bg_color], 0x10      ; 蓝色背景
    jmp apply_color_to_main
bg_F2:
    mov byte [bg_color], 0x20      ; 绿色背景
    jmp apply_color_to_main
bg_F3:
    mov byte [bg_color], 0x30      ; 青色背景
    jmp apply_color_to_main
bg_F4:
    mov byte [bg_color], 0x40      ; 红色背景
    jmp apply_color_to_main
bg_F5:
    mov byte [bg_color], 0x50      ; 品红色背景
    jmp apply_color_to_main
bg_F6:
    mov byte [bg_color], 0x60      ; 棕色背景
    jmp apply_color_to_main
bg_F7:
    mov byte [bg_color], 0x70      ; 灰白色背景
    jmp apply_color_to_main
bg_F8:
    mov byte [bg_color], 0x80      ; 深灰色背景（闪烁位）
    jmp apply_color_to_main
bg_F9:
    mov byte [bg_color], 0x90      ; 亮蓝色背景
    jmp apply_color_to_main
bg_F10:
    mov byte [bg_color], 0xA0      ; 亮绿色背景
    jmp apply_color_to_main
bg_F11:
    mov byte [bg_color], 0xB0      ; 亮青色背景
    jmp apply_color_to_main
bg_F12:
    mov byte [bg_color], 0xC0      ; 亮红色背景
    jmp apply_color_to_main

; 组合字体颜色和背景颜色，应用到主页面12行文本
apply_color_to_main:
    mov edi, 0xB8000 + 1    ; 从第1个字符的属性字节开始
    mov ecx, 12 * 80        ; 12行，每行80列
    mov ah, [text_color]    ; 加载字体颜色（低4位）
    or  ah, [bg_color]      ; 组合背景颜色（高4位）
color_loop:
    mov [edi], ah           ; 只修改属性字节
    add edi, 2              ; 跳过字符字节，移动到下一个属性字节
    loop color_loop
    jmp key_loop

ctrl_down:
    mov byte [ctrl_pressed], 1
    jmp key_loop
ctrl_up:
    mov byte [ctrl_pressed], 0
    jmp key_loop

; 检测 Ctrl + C 关机
check_ctrl_and_c:
    cmp byte [ctrl_pressed], 1
    je shutdown_system
    jmp key_loop

; 检测 Ctrl + R 重启
check_ctrl_and_r:
    cmp byte [ctrl_pressed], 1
    je reboot_system
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

    ; 蓝屏界面隐藏硬件光标
    mov dx, 0x3D4
    mov al, 0x0A
    out dx, al
    mov dx, 0x3D5
    mov al, 0x20
    out dx, al

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