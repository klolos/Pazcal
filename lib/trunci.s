.text
.global _TRUNC
.global start_TRUNC

_TRUNC:
        pushl   %ebp
        movl    %esp, %ebp

start_TRUNC:
        fldt    12(%ebp)
        movl    8(%ebp), %esi
        fisttpl (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret

.data
tmp:
    .word 0
