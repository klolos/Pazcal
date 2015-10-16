.text
.global _ROUND2
.global start_ROUND2

_ROUND2:
        pushl   %ebp
        movl    %esp, %ebp

start_ROUND2:
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
        fisttpl (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret

.data
tmp:
    .word 0
half:
    .tfloat 0.5
