.text
.global _sin2
.global start_sin2

_sin2:
        pushl   %ebp
        movl    %esp, %ebp

start_sin2:
        fldt    16(%ebp)
        movl    12(%ebp), %esi
        fsin
        fstpt   (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret
