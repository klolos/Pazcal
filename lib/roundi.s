.text
.global _ROUND
.global start_ROUND

_ROUND:
        pushl   %ebp
        movl    %esp, %ebp

start_ROUND:
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
        fisttpl (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret

.data
tmp:
    .word 0
half:
    .tfloat 0.5
