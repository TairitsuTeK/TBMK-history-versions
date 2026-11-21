; memory.inc - 内存检测和显示（全屏页面）

section .data
; 制表符
char_tl     db 0xC9    ; 左上角 ╔
char_tr     db 0xBB    ; 右上角 ╗
char_bl     db 0xC8    ; 左下角 ╚
char_br     db 0xBC    ; 右下角 ╝
char_hz     db 0xCD    ; 水平线 ═
char_vt     db 0xBA    ; 垂直线 ║

; 标题和标签
mem_page_title   db 'Memory Information', 0
mem_label_total  db 'Total RAM:', 0
mem_label_used   db 'Used:', 0
mem_label_free   db 'Free:', 0
mem_label_kernel db 'Kernel:', 0
mem_label_stack  db 'Stack:', 0
mem_label_pct    db 'Usage:', 0
mem_unit_kb      db 'KB', 0
mem_unit_bytes   db 'bytes', 0
mem_unit_pct     db '%', 0
mem_hint_esc     db 'Press ESC to return', 0

section .text

; 保存主页面内容（15行文本 + 背景色）
save_main_screen:
    push esi
    push edi
    push ecx
    
    mov esi, VGA_BASE
    mov edi, screen_backup
    mov ecx, TOTAL_CHARS
    rep movsw
    
    pop ecx
    pop edi
    pop esi
    ret

; 恢复主页面内容
restore_main_screen:
    push esi
    push edi
    push ecx
    
    mov esi, screen_backup
    mov edi, VGA_BASE
    mov ecx, TOTAL_CHARS
    rep movsw
    
    pop ecx
    pop edi
    pop esi
    ret

; 显示内存信息全屏页面
show_memory_page:
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    
    ; 保存当前屏幕
    call save_main_screen
    
    ; 检测内存
    cmp byte [mem_detected], 1
    je .skip_detect
    call detect_memory
    mov byte [mem_detected], 1
    
.skip_detect:
    ; 清屏（深蓝色背景，白色文字）
    mov edi, VGA_BASE
    mov ecx, TOTAL_CHARS
    mov ax, 0x1F20      ; 深蓝底白字空格
    rep stosw
    
    ; ========== 绘制边框 ==========
    
    ; 第一行边框
    mov edi, VGA_BASE
    mov ah, 0x1F
    
    mov al, [char_tl]
    stosw
    
    mov ecx, 78
    mov al, [char_hz]
.hz_top:
    stosw
    loop .hz_top
    
    mov al, [char_tr]
    stosw
    
    ; 中间行边框（第2-24行）
    mov ebx, 2
.mid_rows:
    mov edi, VGA_BASE
    mov eax, 160
    mul ebx
    add edi, eax
    
    mov ah, 0x1F
    mov al, [char_vt]
    stosw
    
    add edi, 156       ; 跳过78个字符*2
    
    mov al, [char_vt]
    stosw
    
    inc ebx
    cmp ebx, 25
    jb .mid_rows
    
    ; 最后一行边框
    mov edi, VGA_BASE + 24*160
    mov ah, 0x1F
    
    mov al, [char_bl]
    stosw
    
    mov ecx, 78
    mov al, [char_hz]
.hz_bottom:
    stosw
    loop .hz_bottom
    
    mov al, [char_br]
    stosw
    
    ; ========== 标题（第3行居中） ==========
    mov edi, VGA_BASE + 2*160
    
    ; 计算居中位置
    ; 标题长度约20个字符，居中位置 = (80-20)/2 = 30
    add edi, 30*2
    
    mov ah, 0x1F
    mov esi, mem_page_title
    call mem_print_str
    
    ; ========== 分隔线（第5行） ==========
    mov edi, VGA_BASE + 4*160 + 2*2
    mov ah, 0x1F
    mov al, [char_hz]
    mov ecx, 76
.sep_line:
    stosw
    loop .sep_line
    
    ; ========== 内存信息（第8-18行） ==========
    
    ; 总内存 (第8行)
    mov edi, VGA_BASE + 7*160 + 10*2
    mov ah, 0x1F
    mov esi, mem_label_total
    call mem_print_str
    
    mov edi, VGA_BASE + 7*160 + 30*2
    mov eax, [total_mem_kb]
    test eax, eax
    jnz .show_total
    mov eax, 65536
.show_total:
    call mem_print_num
    
    mov esi, mem_unit_kb
    call mem_print_str
    
    ; 内核大小 (第10行)
    mov edi, VGA_BASE + 9*160 + 10*2
    mov esi, mem_label_kernel
    call mem_print_str
    
    mov edi, VGA_BASE + 9*160 + 30*2
    mov eax, [kernel_size]
    test eax, eax
    jnz .show_kernel
    mov eax, 8192
.show_kernel:
    call mem_print_num
    
    mov esi, mem_unit_bytes
    call mem_print_str
    
    ; 栈大小 (第11行)
    mov edi, VGA_BASE + 10*160 + 10*2
    mov esi, mem_label_stack
    call mem_print_str
    
    mov edi, VGA_BASE + 10*160 + 30*2
    mov eax, STACK_SIZE
    call mem_print_num
    
    mov esi, mem_unit_bytes
    call mem_print_str
    
    ; 已用内存 (第13行)
    mov edi, VGA_BASE + 12*160 + 10*2
    mov esi, mem_label_used
    call mem_print_str
    
    mov edi, VGA_BASE + 12*160 + 30*2
    mov eax, [kernel_size]
    test eax, eax
    jnz .calc_used
    mov eax, 8192
.calc_used:
    add eax, STACK_SIZE
    add eax, 4096
    call mem_print_num
    
    mov esi, mem_unit_bytes
    call mem_print_str
    
    ; 空闲内存 (第14行)
    mov edi, VGA_BASE + 13*160 + 10*2
    mov esi, mem_label_free
    call mem_print_str
    
    mov edi, VGA_BASE + 13*160 + 30*2
    
    mov eax, [total_mem_kb]
    test eax, eax
    jnz .calc_free
    mov eax, 65536
.calc_free:
    mov ebx, 1024
    mul ebx
    mov ebx, eax
    
    mov eax, [kernel_size]
    test eax, eax
    jnz .sub_used
    mov eax, 8192
.sub_used:
    add eax, STACK_SIZE
    add eax, 4096
    sub ebx, eax
    mov eax, ebx
    call mem_print_num
    
    mov esi, mem_unit_bytes
    call mem_print_str
    
    ; ========== 分隔线（第16行） ==========
    mov edi, VGA_BASE + 15*160 + 2*2
    mov ah, 0x1F
    mov al, [char_hz]
    mov ecx, 76
.sep_line2:
    stosw
    loop .sep_line2
    
    ; ========== 使用率（第18行） ==========
    mov edi, VGA_BASE + 17*160 + 10*2
    mov esi, mem_label_pct
    call mem_print_str
    
    ; 计算百分比
    mov eax, [kernel_size]
    test eax, eax
    jnz .calc_pct
    mov eax, 8192
.calc_pct:
    add eax, STACK_SIZE
    add eax, 4096
    mov ebx, 100
    mul ebx
    mov ecx, eax
    
    mov eax, [total_mem_kb]
    test eax, eax
    jnz .div_pct
    mov eax, 65536
.div_pct:
    mov ebx, 1024
    mul ebx
    mov ebx, eax
    mov eax, ecx
    xor edx, edx
    div ebx
    
    ; 显示百分比数字
    mov edi, VGA_BASE + 17*160 + 30*2
    call mem_print_num
    
    mov esi, mem_unit_pct
    call mem_print_str
    
    ; 进度条（40个字符宽）
    mov edi, VGA_BASE + 17*160 + 35*2
    mov al, '['
    stosw
    
    push eax        ; 保存百分比
    
    ; 计算填充数
    mov eax, [kernel_size]
    test eax, eax
    jnz .calc_bar
    mov eax, 8192
.calc_bar:
    add eax, STACK_SIZE
    add eax, 4096
    mov ebx, 100
    mul ebx
    mov ecx, eax
    
    mov eax, [total_mem_kb]
    test eax, eax
    jnz .div_bar
    mov eax, 65536
.div_bar:
    mov ebx, 1024
    mul ebx
    mov ebx, eax
    mov eax, ecx
    xor edx, edx
    div ebx
    
    ; 40个字符的进度条，每2.5%一个字符
    ; 简化：百分比 * 40 / 100 = 百分比 * 2 / 5
    mov ebx, 2
    mul ebx
    mov ebx, 5
    xor edx, edx
    div ebx
    mov ecx, eax
    
    cmp ecx, 40
    jbe .fill_ok
    mov ecx, 40
.fill_ok:
    
    mov ah, 0x1F
    mov al, '#'
    
.bar_fill:
    cmp ecx, 0
    je .bar_empty_start
    stosw
    dec ecx
    jmp .bar_fill
    
.bar_empty_start:
    mov ebx, 40
    sub ebx, ecx
    mov ecx, ebx
    mov al, '.'
    
.bar_empty:
    cmp ecx, 0
    je .bar_done
    stosw
    dec ecx
    jmp .bar_empty
    
.bar_done:
    mov al, ']'
    stosw
    
    pop eax        ; 恢复百分比
    
    ; ========== 底部提示（第22行居中） ==========
    mov edi, VGA_BASE + 21*160
    
    ; 居中位置 = (80 - 22) / 2 = 29
    add edi, 29*2
    
    mov ah, 0x1F
    mov esi, mem_hint_esc
    call mem_print_str
    
    ; ========== 底部分隔线（第23行） ==========
    mov edi, VGA_BASE + 22*160 + 2*2
    mov ah, 0x1F
    mov al, [char_hz]
    mov ecx, 76
.sep_line3:
    stosw
    loop .sep_line3
    
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret

; 简单内存检测
detect_memory:
    push eax
    push ebx
    push esi
    
    mov dword [total_mem_kb], 0
    mov esi, 0x100000
    
.detect_loop:
    mov eax, [esi]
    mov ebx, eax
    
    mov dword [esi], 0x55AA55AA
    cmp dword [esi], 0x55AA55AA
    jne .detect_done
    
    mov dword [esi], 0xAA55AA55
    cmp dword [esi], 0xAA55AA55
    jne .detect_done
    
    mov [esi], ebx
    
    add dword [total_mem_kb], 1024
    add esi, 0x100000
    
    cmp esi, 0x4000000
    jb .detect_loop
    
.detect_done:
    cmp dword [total_mem_kb], 0
    jne .done
    mov dword [total_mem_kb], 65536
    
.done:
    mov dword [kernel_size], detect_end - 0x1000
    
    pop esi
    pop ebx
    pop eax
    ret

; 打印字符串（保留当前属性）
mem_print_str:
    push ax
    push esi
    push edi
    
.print_loop:
    lodsb
    test al, al
    jz .done
    stosw
    jmp .print_loop
    
.done:
    pop edi
    pop esi
    pop ax
    ret

; 打印数字（保留当前属性）
mem_print_num:
    push eax
    push ebx
    push ecx
    push edx
    push edi
    
    test eax, eax
    jnz .not_zero
    mov al, '0'
    stosw
    jmp .num_done
    
.not_zero:
    mov ebx, 10
    mov ecx, 0
    
.div_loop:
    xor edx, edx
    div ebx
    push dx
    inc ecx
    test eax, eax
    jnz .div_loop
    
.print_loop:
    pop ax
    add al, '0'
    stosw
    loop .print_loop
    
.num_done:
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret

detect_end: