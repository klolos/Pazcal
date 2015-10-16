.text
.global _READ_INT
.global start_READ_INT

_READ_INT:
        pushl   %ebp
        movl    %esp, %ebp

start_READ_INT:
        xorl    %ecx, %ecx
        movb    $0, negative

retry:
        call    read_getc
        cmpb    $'\n', %al
        je      retry
        cmpb    $' ', %al
        je      retry
        cmpb    $'+', %al
        jne     skip1
        movb    $0, negative
        jmp     retry
skip1:
        cmpb    $'-', %al
        jne     skip2
        movb    $1, negative
        jmp     retry
skip2:
        cmpb    $0, %al
        je      done
        cmpb    $'0', %al
        jl      retry
        cmpb    $'9', %al
        jg      retry

next_dig:
        subb    $'0', %al
        andl    $0x0FF, %eax
        imull   $10, %ecx
        addl    %eax, %ecx
        call    read_getc
        cmpb    $0, %al
        je      done
        cmpb    $'0', %al
        jl      done_unget
        cmpb    $'9', %al
        jg      done_unget
        jmp     next_dig

done_unget:
        call    read_ungetc
done:
        movl    8(%ebp), %esi
        cmpb    $0, negative
        je      pos
        negl    %ecx
pos:
        movl    %ecx, (%esi)    # integer at %ecx

        movl    %ebp, %esp
        popl    %ebp
        ret

.data
negative:
    .byte 0
