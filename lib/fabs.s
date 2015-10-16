.text
.global _fabs
.global start_fabs

_fabs:
        pushl   %ebp
        movl    %esp, %ebp

start_fabs:
        fldt    16(%ebp)
        movl    12(%ebp), %esi
        fabs
        fstpt   (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret
