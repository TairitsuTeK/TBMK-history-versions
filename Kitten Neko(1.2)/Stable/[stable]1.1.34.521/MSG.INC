; ------------------- 文本定义 -------------------
l1 db '[!]Mirco Kernel Started Successfully.',0
l2 db '[!]Running at x86 Bare-metal Protected Mode.....(OK)',0

l3 db '[!]T.B.M.K. (C) Tairitsu_tty 2026 [Version 1.1.34.521]',0
l4 db '',0
l5 db '[!]Press R to Reboot, Ctrl+C to Crash.',0
l6 db '',0
l7 db '[!]Welcome text:',0
l8 db '   ------------------------',0
l9 db '  [ print("Hello, World!") ]',0
l10 db '   ------------------------',0

; -------------------- 蓝屏文本 -------------------
shut_txt db '[KERNEL PANIC]Kernel has been crashed! (Division By Zero)',0