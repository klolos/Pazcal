.text
.global _cos
.global start_cos

_cos:
        pushl   %ebp
        movl    %esp, %ebp

start_cos:
        fldt    16(%ebp)
        movl    12(%ebp), %esi
        fcos
        fstpt   (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret
