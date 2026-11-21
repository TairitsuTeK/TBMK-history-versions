bits 32

section .text
global _start

_start:
    push ebp
    mov ebp, esp

    ; === 你的代码写在这里 ===
    mov edi, 0xB8000 + 80 * 2 * 10
    mov word [edi], 0x1F00 | 'M'
    mov word [edi+2], 0x1F00 | 'Y'
    mov word [edi+4], 0x1F00 | ' '
    mov word [edi+6], 0x1F00 | 'A'
    mov word [edi+8], 0x1F00 | 'P'
    mov word [edi+10], 0x1F00 | 'P'
    ; === 代码结束 ===

    pop ebp
    ret