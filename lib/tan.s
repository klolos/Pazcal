.text
.global _tan
.global start_tan

_tan:
        pushl   %ebp
        movl    %esp, %ebp

start_tan:
        fldt    16(%ebp)
        movl    12(%ebp), %esi
        fptan
        ffree   %st(0)
        fincstp
        fstpt   (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret
