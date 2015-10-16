.text
.global _trunc2
.global start_trunc2

_trunc2:
        pushl   %ebp
        movl    %esp, %ebp

start_trunc2:
        fldt    16(%ebp)
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
