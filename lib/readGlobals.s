.global read_getc
.global read_ungetc
.text
read_getc:
    movl    read_rd, %eax
    cmpl    read_wr, %eax
    jne     notEmpty
    call    read_more
    andl    %eax, %eax
    jnz     notEmpty
    movb    $0, %al
    ret
notEmpty:
    pushl   %edi
    movl    read_rd, %edi
    addl    $read_buffer, %edi
    movb    (%edi), %al
    movl    read_rd, %edi
    incl    %edi
    movl    %edi, read_rd
    popl    %edi
    ret

read_ungetc:
    pushl   %edi
    movl    read_rd, %edi
    decl    %edi
    movl    %edi, read_rd
    popl    %edi
    ret

read_more:
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    movl    $4096, %edx
    subl    read_wr, %edx
    jnz     notFull
    movl    $0, %eax
    movl    %eax, read_rd
    movl    %eax, read_wr
    movl    $4096, %edx
notFull:
    movl    $3, %eax
    movl    $0, %ebx
    movl    $read_buffer, %ecx
    addl    read_wr, %ecx
    int     $0x80
    addl    %eax, read_wr
    popl    %edx
    popl    %ecx
    popl    %ebx
    ret

.data
read_buffer:
    .rept 4096
    .byte 0
    .endr

read_rd:
    .long 0

read_wr:
    .long 0
