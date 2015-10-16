.text
.globl _WRITE_REAL
.globl start_WRITE_REAL

_WRITE_REAL:
	push	%ebp
	movl	%esp, %ebp
	addl	$-34, %esp
start_WRITE_REAL:
_q2:
	movl	$1, %eax
	movl	%eax, -4(%ebp)
_q3:
	movl	12(%ebp), %eax
	movl	%eax, -8(%ebp)
_q4:
	movl	$1, %eax
	movl	%eax, -12(%ebp)
_q6:
	movl	-4(%ebp), %eax
	movl	-8(%ebp), %ecx
	cmpl	%ecx, %eax
	jg	_q11
_q7:
	movl	-4(%ebp), %eax
	movl	%eax, tmp
_q8:
	movl	decs, %eax
	movl	$10, %ecx
	imull	%ecx, %eax
	movl	%eax, decs
_q9:
	movl	-4(%ebp), %eax
	movl	-12(%ebp), %ecx
	addl	%ecx, %eax
	movl	%eax, -4(%ebp)
_q10:
	jmp	_q6
_q11:
	movl	decs, %eax
	pushl	%eax
	fildl	(%esp)
	popl	%eax
	fldt	20(%ebp)
	fmulp %st, %st(1)
	fstpt	-22(%ebp)
_q12:
	fldt	-22(%ebp)
	subl	$10, %esp
	fstpt	(%esp)
_q13:
	leal	-26(%ebp), %esi
	push	%esi
_q14:
	call	_ROUND
	addl	$14, %esp
_q15:
	movl	-26(%ebp), %eax
	movl	%eax, tmp
_q16:
	movl	tmp, %eax
	movl	decs, %ecx
	cltd
	idivl	%ecx
	movl	%eax, -30(%ebp)
_q17:
	movl	-30(%ebp), %eax
	push	%eax
_q18:
	movl	16(%ebp), %eax
    subl    12(%ebp), %eax
    decl    %eax
	push	%eax
_q19:
	subl	$4, %esp
	call	_WRITE_INT
	addl	$12, %esp
_q20:
	xorl	%eax, %eax
	movb	$'.', %al
	subl	$1, %esp
	movb	%al, (%esp)
_q21:
	movl	$0, %eax
	push	%eax
_q22:
	subl	$4, %esp
	call	_WRITE_CHAR
	addl	$9, %esp
_q23:
	movl	tmp, %eax
	movl	decs, %ecx
	cltd
	idivl	%ecx
	movl	%edx, -34(%ebp)
_q24:
	movl	-34(%ebp), %eax
    cmpl    $0, %eax
    jg      pos
    negl    %eax
pos:
	push	%eax
_q25:
	movl	$0, %eax
	push	%eax
_q26:
	subl	$4, %esp
	call	_WRITE_INT

end_WRITE_REAL:
	movl	%ebp, %esp
	pop	%ebp
	ret

	.data

decs:
	.long	1
tmp:
	.long	0
