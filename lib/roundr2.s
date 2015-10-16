.text
.global _round2
.global start_round2

_round2:
        pushl   %ebp
        movl    %esp, %ebp

start_round2:
        fldt    16(%ebp)
        fldz
        fcomip
        fstp %st(0)
        ja      negative
        fldt    16(%ebp)
        fldt    half
        faddp   %st, %st(1)
        jmp     ok

negative:
        fldt    16(%ebp)
        fldt    half
        fsubrp   %st, %st(1)

ok:
        movl    12(%ebp), %esi
        fisttpl tmp
        fildl   tmp
        fstpt   (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret

.data
tmp:
    .long 0
half:
    .tfloat 0.5
