.text
.global _getchar
.global start_getchar

_getchar:
        pushl   %ebp
        movl    %esp, %ebp

start_getchar:
        call    read_getc
        andl    $0x0FF, %eax
        movl    8(%ebp), %esi
        movl    %eax, (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret
