.text
.global _READ_BOOL2
.global start_READ_BOOL2

_READ_BOOL2:
        pushl   %ebp
        movl    %esp, %ebp

start_READ_BOOL2:

retry:
        call    read_getc
        cmpb    $'\n', %al
        je      retry
        cmpb    $' ', %al
        je      retry
        cmpb    $0, %al
        je      false
        cmpb    $'t', %al
        je      try_true
        cmpb    $'f', %al
        je      try_false
        jmp     retry

try_true:
        call    read_getc
        cmpb    $'r', %al
        jne     retry
        call    read_getc
        cmpb    $'u', %al
        jne     retry
        call    read_getc
        cmpb    $'e', %al
        jne     retry
        jmp     true

try_false:
        call    read_getc
        cmpb    $'a', %al
        jne     retry
        call    read_getc
        cmpb    $'l', %al
        jne     retry
        call    read_getc
        cmpb    $'s', %al
        jne     retry
        call    read_getc
        cmpb    $'e', %al
        jne     retry
        jmp     false

false:
        movb     $0, %cl
        jmp     out

true:
        movb    $1, %cl
out:
        movl    12(%ebp), %esi
        movb    %cl, (%esi)    # integer at %ecx

        movl    %ebp, %esp
        popl    %ebp
        ret
