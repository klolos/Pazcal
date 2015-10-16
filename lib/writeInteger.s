.text
.global	_WRITE_INT
.global	start_WRITE_INT
        
_WRITE_INT:
        pushl   %ebp
        movl    %esp, %ebp
        subl	$1, %esp                # dummy space for the result

start_WRITE_INT:
        movl    $buffer, %eax
        pushl   %eax
        pushl   16(%ebp)
        subl    $3, %esp
        movl    %esp, %esi
        movb    $0, 2(%esi)             # width = 0
        movb    $0, 1(%esi)             # flags = 0
        movb    $10, 0(%esi)            # base = 10
        lea     -1(%ebp), %eax
        pushl   %eax                    # dummy: result
        pushl   %ebp                    # dummy: access link
        call    _formatInteger
        addl    $19, %esp

        movl    $buffer, %eax           # what to print
        pushl   %eax
        pushl   12(%ebp)
        subl    $4, %esp                # dummy: result
        call    _WRITE_STRING
        addl    $12, %esp

        movl    %ebp, %esp
        popl    %ebp
        ret

.data
buffer:
.rept   11
.byte   0
.endr
