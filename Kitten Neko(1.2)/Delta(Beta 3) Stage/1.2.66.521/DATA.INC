section .data
ctrl_pressed    db  0
text_color      db  COLOR_BRIGHT_CYAN
bg_color        db  COLOR_BLACK
mem_detected    db  0
total_mem_kb    dd  0
kernel_size     dd  0
mem_visible     db  0
section .bss
screen_backup   resw TOTAL_CHARS    ; 保存主页面内容