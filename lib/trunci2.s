.text
.global _TRUNC2
.global start_TRUNC2

_TRUNC2:
        pushl   %ebp
        movl    %esp, %ebp

start_TRUNC2:
        fldt    16(%ebp)
        movl    12(%ebp), %esi
        fisttpl (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret

.data
tmp:
    .word 0
