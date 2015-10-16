.text
.global _strcat
.global start_strcat

_strcat:
        pushl   %ebp
        movl    %esp, %ebp

start_strcat:
        movl    16(%ebp), %esi
next:
        movb    (%esi), %al             # Load next character
        andb    %al, %al
        jz      ok                      # if 0, then ok
        incl    %esi
        jmp     next
ok:
        movl    12(%ebp), %edi

loop0:
    movb    (%edi), %al
    movb    %al, (%esi)
    andb    %al, %al
    jz      ok2
    incl    %edi
    incl    %esi
    jmp     loop0

ok2:
    movl    %ebp, %esp
    popl    %ebp
    ret
