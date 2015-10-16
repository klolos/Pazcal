.text
.global _putchar
.global start_putchar

_putchar:
        pushl   %ebp
        movl    %esp, %ebp

start_putchar:
        leal    12(%ebp), %ecx
        movl    $1, %edx
        movl    $1, %ebx
        movl    $4, %eax
        int     $0x80

        movl    %ebp, %esp
        popl    %ebp
        ret
