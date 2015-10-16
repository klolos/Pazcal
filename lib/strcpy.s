.text
.global _strcpy
.global start_strcpy

_strcpy:
    pushl   %ebp
    movl    %esp, %ebp

start_strcpy:
    movl    16(%ebp), %esi
    movl    12(%ebp), %edi

loop0:
    movb    (%edi), %al
    movb    %al, (%esi)
    andb    %al, %al
    jz      ok
    incl    %edi
    incl    %esi
    jmp     loop0

ok:
    movl    %ebp, %esp
    popl    %ebp
    ret
