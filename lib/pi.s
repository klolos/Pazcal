.text
.global _pi
.global start_pi

_pi:
        pushl   %ebp
        movl    %esp, %ebp

start_pi:
        fldpi
        movl    12(%ebp), %esi
        fstpt   (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret
