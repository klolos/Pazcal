.text
.global _ln2
.global start_ln2

_ln2:
        pushl   %ebp
        movl    %esp, %ebp

start_ln2:
        fldln2
        fldt    16(%ebp)
        fyl2x

        movl    12(%ebp), %esi
        fstpt   (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret
