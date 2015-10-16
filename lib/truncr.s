.text
.global _trunc
.global start_trunc

_trunc:
        pushl   %ebp
        movl    %esp, %ebp

start_trunc:
        fldt    12(%ebp)
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
