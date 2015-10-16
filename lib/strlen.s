.text
.global _strlen
.global start_strlen

_strlen:
        pushl   %ebp
        movl    %esp, %ebp

start_strlen:
        movl    16(%ebp), %esi          # 1st parameter, the string
        xorl    %edx, %edx              # counter = 0
next:
        movb    (%esi), %al             # Load next character
        andb    %al, %al
        jz      ok                      # if 0, then ok
        incl    %edx                    # counter = counter + 1
        incl    %esi
        jmp     next
ok:
        movl    12(%ebp), %esi
        movl    %edx, (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret
