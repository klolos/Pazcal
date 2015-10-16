.text
.global _strcmp
.global start_strcmp

_strcmp:
    pushl   %ebp
    movl    %esp, %ebp

start_strcmp:
    movl    16(%ebp), %esi
    movl    12(%ebp), %edi

loop0:
    movb    (%edi), %al
    cmpb    %al, (%esi)
    jl      less
    jg      greater
    andb    %al, %al
    jz      eq
    incl    %esi
    incl    %edi
    jmp loop0

less:
    movl $-1, %eax
    jmp out
eq  :
    movl $0, %eax
    jmp out
greater:
    movl $1, %eax
    jmp out

out:
    movl    8(%ebp), %esi
    movl    %eax, (%esi)

    movl    %ebp, %esp
    popl    %ebp
    ret
