.text
.global _WRITE_STRING
.global start_WRITE_STRING

_WRITE_STRING:
        pushl   %ebp
        movl    %esp, %ebp

start_WRITE_STRING:
        movl    16(%ebp), %esi          # 1st parameter, the string
        xorl    %edx, %edx              # counter = 0
next:
        movb    (%esi), %al             # Load next character
        andb    %al, %al
        jz      ok                      # if 0, then ok
        incl    %edx                    # counter = counter + 1
        incl    %esi
        jmp     next
ok:
        movl    12(%ebp), %ecx
        subl    %edx, %ecx
        jle     out
        pushl   %edx
more_spaces:
        pushl   %ecx
        movl    $space, %ecx
        movl    $1, %edx
        movl    $1, %ebx                # fd for stdout in %ebx
        movl    $4, %eax                # system call number (sys_write)
        int     $0x80
        popl    %ecx
        loopl   more_spaces
        popl    %edx

out:
        movl    16(%ebp), %ecx          # %ecx/%edx, the string and #chars
        movl    $1, %ebx                # fd for stdout in %ebx
        movl    $4, %eax                # system call number (sys_write)
        int     $0x80                   # call kernel

        movl    %ebp, %esp
        popl    %ebp
        ret

.data
space:
    .byte ' '
