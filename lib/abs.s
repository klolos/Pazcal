.text
.global _abs
.global start_abs

_abs:
        pushl   %ebp
        movl    %esp, %ebp

start_abs:
        movl    16(%ebp), %eax
        cmpl    $0, %eax
        jge     ok
        negl    %eax
ok:
        movl    12(%ebp), %esi
        movl    %eax, (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret
