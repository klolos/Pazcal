.text
.global _READ_REAL
.global start_READ_REAL

_READ_REAL:
        pushl   %ebp
        movl    %esp, %ebp

start_READ_REAL:
        xorl    %ecx, %ecx      # digits after point read
        fldz                    # number kept at top of the stack
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
        fildl   ten
        fmulp
        movl    %eax, tmp
        fiaddl  tmp
        call    read_getc
        cmpb    $0, %al
        je      done
        cmpb    $'.', %al
        je      after_point
        cmpb    $'0', %al
        jl      done_unget
        cmpb    $'9', %al
        jg      done_unget
        jmp     next_dig

after_point:
        call    read_getc
        cmpb    $0, %al
        je      done
        cmpb    $'0', %al
        jl      done_unget
        cmpb    $'9', %al
        jg      done_unget
        subb    $'0', %al
        andl    $0x0FF, %eax
        fimull  ten
        movl    %eax, tmp
        fiaddl  tmp
        incl    %ecx
        jmp     after_point

done_unget:
        call    read_ungetc
done:
        cmpl    $0, %ecx
        je      done_divs

div:
        fidiv   ten
        loopl   div

done_divs:
        cmpb    $0, negative
        je      pos
        fchs
pos:
        movl    8(%ebp), %esi
        fstpt   (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret

.data
negative:
    .byte 0

ten:
    .long 10

tmp:
    .long 0
