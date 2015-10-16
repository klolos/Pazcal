.text
.global	_WRITE_CHAR
.global	start_WRITE_CHAR
        
_WRITE_CHAR:
        pushl   %ebp
        movl    %esp, %ebp

start_WRITE_CHAR:
        movb    16(%ebp), %al
        movb    %al, buffer

        movl    $buffer, %ecx
        push    %ecx
        movl    12(%ebp), %ecx
        push    %ecx
        push    %ecx        # dummy result
        call    _WRITE_STRING

        movl    %ebp, %esp
        popl    %ebp
        ret

.data
buffer:
    .byte 0
    .byte 0
