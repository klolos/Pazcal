.text
.global _ln
.global start_ln

_ln:
        pushl   %ebp
        movl    %esp, %ebp

start_ln:
        fld1
        fldl2e
        fdivp   %st(0), %st(1)
        fldt    16(%ebp)
        fyl2x
        movl    12(%ebp), %esi
        fstpt   (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret
