org 0x1000
bits 32

%include "CONSTANTS.INC"
%include "DATA.INC"
%include "MSG.INC"

section .text

start:
    mov esp, STACK_BASE
    mov ebp, esp

    call cls
    call print_all_lines
    call set_initial_cursor

    jmp key_loop

%include "DISPLAY.INC"
%include "COLORS.INC"
%include "KEYBOARD.INC"
%include "SHUTDOWN.INC"