.text
.global _sin
.global start_sin

_sin:
        pushl   %ebp
        movl    %esp, %ebp

start_sin:
        fldt    12(%ebp)
        movl    8(%ebp), %esi
        fsin
        fstpt   (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret
