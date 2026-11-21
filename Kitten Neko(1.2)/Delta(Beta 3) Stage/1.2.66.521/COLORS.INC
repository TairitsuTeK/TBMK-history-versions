; 应用文本颜色（仅15行）
apply_text_color:
    push edi
    push ecx
    push ax
    
    mov edi, VGA_BASE + 1
    mov ecx, TEXT_ROWS * SCREEN_COLS
    mov ah, [text_color]
    
text_color_loop:
    mov al, [edi]
    and al, 0xF0
    or  al, ah
    mov [edi], al
    add edi, 2
    loop text_color_loop
    
    pop ax
    pop ecx
    pop edi
    jmp key_loop

; 应用背景颜色（全屏）
apply_bg_color:
    push edi
    push ecx
    push ax
    
    mov edi, VGA_BASE + 1
    mov ecx, TOTAL_CHARS
    mov ah, [bg_color]
    
bg_color_loop:
    mov al, [edi]
    and al, 0x0F
    or  al, ah
    mov [edi], al
    add edi, 2
    loop bg_color_loop
    
    pop ax
    pop ecx
    pop edi
    jmp key_loop

; F1-F12 文本颜色表
set_text_color:
    cmp al, SC_F1
    je .f1
    cmp al, SC_F2
    je .f2
    cmp al, SC_F3
    je .f3
    cmp al, SC_F4
    je .f4
    cmp al, SC_F5
    je .f5
    cmp al, SC_F6
    je .f6
    cmp al, SC_F7
    je .f7
    cmp al, SC_F8
    je .f8
    cmp al, SC_F9
    je .f9
    cmp al, SC_F10
    je .f10
    cmp al, SC_F11
    je .f11
    cmp al, SC_F12
    je .f12
    jmp key_loop

.f1:  mov byte [text_color], COLOR_BRIGHT_RED
      jmp apply_text_color
.f2:  mov byte [text_color], COLOR_BRIGHT_GREEN
      jmp apply_text_color
.f3:  mov byte [text_color], COLOR_YELLOW
      jmp apply_text_color
.f4:  mov byte [text_color], COLOR_BRIGHT_BLUE
      jmp apply_text_color
.f5:  mov byte [text_color], COLOR_BRIGHT_MAGENTA
      jmp apply_text_color
.f6:  mov byte [text_color], COLOR_BRIGHT_CYAN
      jmp apply_text_color
.f7:  mov byte [text_color], COLOR_WHITE
      jmp apply_text_color
.f8:  mov byte [text_color], COLOR_GREEN
      jmp apply_text_color
.f9:  mov byte [text_color], COLOR_RED
      jmp apply_text_color
.f10: mov byte [text_color], COLOR_BLUE
      jmp apply_text_color
.f11: mov byte [text_color], COLOR_MAGENTA
      jmp apply_text_color
.f12: mov byte [text_color], COLOR_CYAN
      jmp apply_text_color

; F1-F12 背景颜色表
set_bg_color:
    cmp al, SC_F1
    je .f1
    cmp al, SC_F2
    je .f2
    cmp al, SC_F3
    je .f3
    cmp al, SC_F4
    je .f4
    cmp al, SC_F5
    je .f5
    cmp al, SC_F6
    je .f6
    cmp al, SC_F7
    je .f7
    cmp al, SC_F8
    je .f8
    cmp al, SC_F9
    je .f9
    cmp al, SC_F10
    je .f10
    cmp al, SC_F11
    je .f11
    cmp al, SC_F12
    je .f12
    jmp key_loop

.f1:  mov byte [bg_color], BG_BLUE
      jmp apply_bg_color
.f2:  mov byte [bg_color], BG_GREEN
      jmp apply_bg_color
.f3:  mov byte [bg_color], BG_CYAN
      jmp apply_bg_color
.f4:  mov byte [bg_color], BG_RED
      jmp apply_bg_color
.f5:  mov byte [bg_color], BG_MAGENTA
      jmp apply_bg_color
.f6:  mov byte [bg_color], BG_BROWN
      jmp apply_bg_color
.f7:  mov byte [bg_color], BG_LIGHT_GRAY
      jmp apply_bg_color
.f8:  mov byte [bg_color], BG_BLACK
      jmp apply_bg_color
.f9:  mov byte [bg_color], BG_BLUE
      jmp apply_bg_color
.f10: mov byte [bg_color], BG_GREEN
      jmp apply_bg_color
.f11: mov byte [bg_color], BG_CYAN
      jmp apply_bg_color
.f12: mov byte [bg_color], BG_RED
      jmp apply_bg_color