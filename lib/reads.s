.text
.global _READ_STRING
.global start_READ_STRING

_READ_STRING:
    pushl   %ebp
    movl    %esp, %ebp

start_READ_STRING:
    movl    12(%ebp), %esi
next:
    subl    $1, 16(%ebp)
    jl      done
    call    read_getc
    cmpb    $'\n', %al
    je      done
    cmpb    $0, %al
    je      done
    movb    %al, (%esi)
    incl    %esi
    jmp next

done:
    movl    $0, (%esi)
    movl    %ebp, %esp
    popl    %ebp
    ret
