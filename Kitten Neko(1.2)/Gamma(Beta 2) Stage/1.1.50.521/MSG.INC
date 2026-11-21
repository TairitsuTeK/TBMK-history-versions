; ------------------- 文本定义 -------------------
l1 db '[!]Mirco Kernel Started Successfully.',0
l2 db '[!]Running at x86 Bare-metal Protected Mode.....(OK)',0
l3 db '[!]T.B.M.K. (C) Tairitsu_tty 2026 [Version 1.1.50.521 Gamma (Beta 2)]',0
l4 db '',0
l5 db '[!]Press F1-F12 to change text color.',0
l6 db '[!]Press Ctrl+F1-F12 to change background color.',0
l7 db '[!]press Ctrl+R to Reboot.',0
l8 db '[!]Press Ctrl+C to Crash.',0
l9 db '',0
l10 db '[!]Welcome text:',0
l11 db '   +------------------------+',0
l12 db '   | print("Hello, World!") |',0
l13 db '   +------------------------+',0
l14 db '',0
l15 db '>',0

; -------------------- 蓝屏文本 -------------------
shut_txt db '[KERNEL PANIC]Kernel has been crashed! (Undefined Instruction.)',0