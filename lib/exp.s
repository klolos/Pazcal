.text
.global _exp
.global start_exp

_exp:
        pushl   %ebp
        movl    %esp, %ebp

start_exp:
        fldt    16(%ebp)
        fldl2e
        fmulp   %st(1), %st(0)
        fld1
        fscale
        fstp    %st(1)
        movl    12(%ebp), %esi
        fstpt   (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret
