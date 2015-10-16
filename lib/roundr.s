.text
.global _round
.global start_round

_round:
        pushl   %ebp
        movl    %esp, %ebp

start_round:
        fldt    12(%ebp)
        fldz
        fcomip
        ja      negative
        fldt    12(%ebp)
        fldt    half
        faddp   %st, %st(1)
        jmp     ok

negative:
        fldt    12(%ebp)
        fldt    half
        fsubrp   %st, %st(1)

ok:
        movl    8(%ebp), %esi
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
