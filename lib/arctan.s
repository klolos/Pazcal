.text
.global _arctan
.global start_arctan

_arctan:
        pushl   %ebp
        movl    %esp, %ebp

start_arctan:
        fldt    16(%ebp)
        movl    12(%ebp), %esi
        fld1
        fpatan
        fstpt   (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret
