.text
.global _sqrt
.global start_sqrt

_sqrt:
        pushl   %ebp
        movl    %esp, %ebp

start_sqrt:
        fldt    16(%ebp)
        movl    12(%ebp), %esi
        fsqrt
        fstpt   (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret
