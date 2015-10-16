.text
.global _getchar2
.global start_getchar2

_getchar2:
        pushl   %ebp
        movl    %esp, %ebp

start_getchar2:
        call    read_getc
        andl    $0x0FF, %eax
        movl    12(%ebp), %esi
        movl    %eax, (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret
