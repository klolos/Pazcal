.text
.global	_WRITE_BOOL
.global	start_WRITE_BOOL
        
_WRITE_BOOL:
        pushl   %ebp
        movl    %esp, %ebp

start_WRITE_BOOL:
        movl    $falsestring, %ecx
        movb    16(%ebp), %al
        andb    %al, %al
        jz      out
        movl    $truestring, %ecx
out:
        push    %ecx
        movl    12(%ebp), %ecx
        push    %ecx
        push    %ecx        # dummy result
        call    _WRITE_STRING

        movl    %ebp, %esp
        popl    %ebp
        ret

.data
truestring:
    .asciz "true"
falsestring:
    .asciz "false"
