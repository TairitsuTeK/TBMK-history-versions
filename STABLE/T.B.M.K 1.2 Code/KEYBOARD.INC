; 键盘主循环
key_loop:
    in al, KEYB_STATUS
    test al, 1
    jz key_loop
    in al, KEYB_DATA

    ; Ctrl按键按下/松开
    cmp al, SC_CTRL_DOWN
    je ctrl_down
    cmp al, SC_CTRL_UP
    je ctrl_up
    
    ; Ctrl + C 关机
    cmp al, SC_C
    je check_ctrl_and_c
    
    ; Ctrl + R 重启
    cmp al, SC_R
    je check_ctrl_and_r

    ; ========== F1-F12 功能键处理 ==========
    cmp al, SC_F1
    jb not_f_key
    cmp al, SC_F10
    jbe is_f_key
    cmp al, SC_F11
    je is_f_key
    cmp al, SC_F12
    je is_f_key
    jmp not_f_key

is_f_key:
    cmp byte [ctrl_pressed], 1
    je set_bg_color
    jmp set_text_color

not_f_key:
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

check_ctrl_and_r:
    cmp byte [ctrl_pressed], 1
    je reboot_system
    jmp key_loop