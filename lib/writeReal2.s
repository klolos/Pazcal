.section .text
.globl   _WRITE_REAL2

__L0: # unit, write_length, -, -
write_length:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$14, %esp
__L1: # :=, $10, -, length
	movl	$0, %eax
	movl	%eax, -4(%ebp)
__L2: # :=, x, -, num
	movl	16(%ebp), %eax
	movl	%eax, -8(%ebp)
__L3: # <, x, $11, $12
	movl	$0, %ecx
	movl	$1, %edx
	movl	$0, %ebx
	movl	16(%ebp), %eax
	cmpl	%ebx, %eax
	cmovl	%edx, %ecx
	movb	%cl, -9(%ebp)
__L4: # ==, $12, $1, 7
	movl	$0, %ecx
	movl	$1, %edx
	movl	$0, %ebx
	movl	$0, %eax
	movsbl	-9(%ebp), %eax
	cmpl	%ebx, %eax
	je	__L7
__L5: # +, length, $3, length
	movl	$1, %ebx
	movl	-4(%ebp), %eax
	addl	%ebx, %eax
	movl	%eax, -4(%ebp)
__L6: # jump, -, -, 7
	jmp __L7
__L7: # +, length, $3, length
	movl	$1, %ebx
	movl	-4(%ebp), %eax
	addl	%ebx, %eax
	movl	%eax, -4(%ebp)
__L8: # /, num, $13, $14
	movl	$0, %edx
	movl	-8(%ebp), %eax
	movl	$10, %ebx
	idiv	%ebx
	movl	%eax, -13(%ebp)
__L9: # :=, $14, -, num
	movl	-13(%ebp), %eax
	movl	%eax, -8(%ebp)
__L10: # >, num, $15, $16
	movl	$0, %ecx
	movl	$1, %edx
	movl	$0, %ebx
	movl	-8(%ebp), %eax
	cmpl	%ebx, %eax
	cmovg	%edx, %ecx
	movb	%cl, -14(%ebp)
__L11: # ==, $16, $2, 7
	movl	$0, %ecx
	movl	$1, %edx
	movl	$1, %ebx
	movl	$0, %eax
	movsbl	-14(%ebp), %eax
	cmpl	%ebx, %eax
	je	__L7
__L12: # retv, length, -, -
	movl	-4(%ebp), %eax
	movl	12(%ebp), %esi
	movl	%eax, 0(%esi)
	movl	%ebp, %esp
	popl	%ebp
	ret
__L13: # endu, write_length, -, -
	movl	%ebp, %esp
	popl	%ebp
	ret

__L14: # unit, print_rounded, -, -
print_rounded:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$58, %esp
__L15: # :=, $17, -, m
	movl	$1, %eax
	movl	%eax, -4(%ebp)
__L16: # :=, $18, -, $19
	movl	$1, %eax
	movl	%eax, -16(%ebp)
__L17: # :=, $19, -, j
	movl	-16(%ebp), %eax
	movl	%eax, -8(%ebp)
__L18: # <=, $19, num_decimals, 20
	movl	$0, %ecx
	movl	$1, %edx
	movl	12(%ebp), %ebx
	movl	-16(%ebp), %eax
	cmpl	%ebx, %eax
	jle	__L20
__L19: # jump, -, -, 24
	jmp __L24
__L20: # *, m, $20, $21
	movl	-4(%ebp), %eax
	movl	$10, %ebx
	mul	%ebx
	movl	%eax, -20(%ebp)
__L21: # :=, $21, -, m
	movl	-20(%ebp), %eax
	movl	%eax, -4(%ebp)
__L22: # +, $19, $3, $19
	movl	$1, %ebx
	movl	-16(%ebp), %eax
	addl	%ebx, %eax
	movl	%eax, -16(%ebp)
__L23: # jump, -, -, 17
	jmp __L17
__L24: # *, remainder, m, $22
	fildl	-4(%ebp)
	fldt	16(%ebp)
	fmulp	%st, %st(1)
	fstpt	-30(%ebp)
__L25: # par, $22, V, -
	fldt	-30(%ebp)
	subl	$10, %esp
	fstpt	0(%esp)
__L26: # par, $23, RET, -
	movl	%ebp, %eax
	addl	$-34, %eax
	pushl	%eax
__L27: # call, -, -, _ROUND2
	pushl	%ebp
	call	_ROUND2
	addl	$18, %esp
__L28: # :=, $23, -, i
	movl	-34(%ebp), %eax
	movl	%eax, -38(%ebp)
__L29: # :=, $24, -, $25
	movl	$1, %eax
	movl	%eax, -42(%ebp)
__L30: # :=, $25, -, j
	movl	-42(%ebp), %eax
	movl	%eax, -8(%ebp)
__L31: # <=, $25, num_decimals, 33
	movl	$0, %ecx
	movl	$1, %edx
	movl	12(%ebp), %ebx
	movl	-42(%ebp), %eax
	cmpl	%ebx, %eax
	jle	__L33
__L32: # jump, -, -, 45
	jmp __L45
__L33: # /, m, $26, $27
	movl	$0, %edx
	movl	-4(%ebp), %eax
	movl	$10, %ebx
	idiv	%ebx
	movl	%eax, -46(%ebp)
__L34: # :=, $27, -, m
	movl	-46(%ebp), %eax
	movl	%eax, -4(%ebp)
__L35: # /, i, m, $28
	movl	$0, %edx
	movl	-38(%ebp), %eax
	movl	-4(%ebp), %ebx
	idiv	%ebx
	movl	%eax, -50(%ebp)
__L36: # :=, $28, -, x
	movl	-50(%ebp), %eax
	movl	%eax, -12(%ebp)
__L37: # par, x, V, -
	movl	-12(%ebp), %eax
	pushl	%eax
__L38: # par, $7, V, -
	movl	$1, %eax
	pushl	%eax
__L39: # call, -, -, _WRITE_INT
	pushl	%ebp
	call	_WRITE_INT
	addl	$12, %esp
__L40: # *, x, m, $29
	movl	-12(%ebp), %eax
	movl	-4(%ebp), %ebx
	mul	%ebx
	movl	%eax, -54(%ebp)
__L41: # -, i, $29, $30
	movl	-38(%ebp), %eax
	movl	-54(%ebp), %ebx
	subl	%ebx, %eax
	movl	%eax, -58(%ebp)
__L42: # :=, $30, -, i
	movl	-58(%ebp), %eax
	movl	%eax, -38(%ebp)
__L43: # +, $25, $3, $25
	movl	$1, %ebx
	movl	-42(%ebp), %eax
	addl	%ebx, %eax
	movl	%eax, -42(%ebp)
__L44: # jump, -, -, 30
	jmp __L30
__L45: # endu, print_rounded, -, -
	movl	%ebp, %esp
	popl	%ebp
	ret

__L46: # unit, print_raw, -, -
print_raw:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$51, %esp
__L47: # -, num_decimals, $31, $32
	movl	12(%ebp), %eax
	movl	$1, %ebx
	subl	%ebx, %eax
	movl	%eax, -4(%ebp)
__L48: # :=, $32, -, decs
	movl	-4(%ebp), %eax
	movl	%eax, -8(%ebp)
__L49: # >, decs, $33, $34
	movl	$0, %ecx
	movl	$1, %edx
	movl	$0, %ebx
	movl	-8(%ebp), %eax
	cmpl	%ebx, %eax
	cmovg	%edx, %ecx
	movb	%cl, -13(%ebp)
__L50: # ==, $34, $1, 64
	movl	$0, %ecx
	movl	$1, %edx
	movl	$0, %ebx
	movl	$0, %eax
	movsbl	-13(%ebp), %eax
	cmpl	%ebx, %eax
	je	__L64
__L51: # *, remainder, $35, $36
	movl	$10, -4(%esp)
	fildl	-4(%esp)
	fldt	16(%ebp)
	fmulp	%st, %st(1)
	fstpt	-23(%ebp)
__L52: # :=, $36, -, remainder
	fldt	-23(%ebp)
	fstpt	16(%ebp)
__L53: # par, remainder, V, -
	fldt	16(%ebp)
	subl	$10, %esp
	fstpt	0(%esp)
__L54: # par, $37, RET, -
	movl	%ebp, %eax
	addl	$-27, %eax
	pushl	%eax
__L55: # call, -, -, _TRUNC2
	pushl	%ebp
	call	_TRUNC2
	addl	$18, %esp
__L56: # :=, $37, -, i
	movl	-27(%ebp), %eax
	movl	%eax, -12(%ebp)
__L57: # -, remainder, i, $38
	fildl	-12(%ebp)
	fldt	16(%ebp)
	fsubp	%st(1)
	fstpt	-37(%ebp)
__L58: # :=, $38, -, remainder
	fldt	-37(%ebp)
	fstpt	16(%ebp)
__L59: # par, i, V, -
	movl	-12(%ebp), %eax
	pushl	%eax
__L60: # par, $7, V, -
	movl	$1, %eax
	pushl	%eax
__L61: # call, -, -, _WRITE_INT
	pushl	%ebp
	call	_WRITE_INT
	addl	$12, %esp
__L62: # -, decs, $3, decs
	movl	-8(%ebp), %eax
	movl	$1, %ebx
	subl	%ebx, %eax
	movl	%eax, -8(%ebp)
__L63: # jump, -, -, 49
	jmp __L49
__L64: # *, remainder, $39, $40
	movl	$10, -4(%esp)
	fildl	-4(%esp)
	fldt	16(%ebp)
	fmulp	%st, %st(1)
	fstpt	-47(%ebp)
__L65: # par, $40, V, -
	fldt	-47(%ebp)
	subl	$10, %esp
	fstpt	0(%esp)
__L66: # par, $41, RET, -
	movl	%ebp, %eax
	addl	$-51, %eax
	pushl	%eax
__L67: # call, -, -, _ROUND2
	pushl	%ebp
	call	_ROUND2
	addl	$18, %esp
__L68: # :=, $41, -, i
	movl	-51(%ebp), %eax
	movl	%eax, -12(%ebp)
__L69: # par, i, V, -
	movl	-12(%ebp), %eax
	pushl	%eax
__L70: # par, $7, V, -
	movl	$1, %eax
	pushl	%eax
__L71: # call, -, -, _WRITE_INT
	pushl	%ebp
	call	_WRITE_INT
	addl	$12, %esp
__L72: # endu, print_raw, -, -
	movl	%ebp, %esp
	popl	%ebp
	ret

__L73: # unit, print_scientific, -, -
print_scientific:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$73, %esp
__L74: # :=, $42, -, exponent
	movl	$0, %eax
	movl	%eax, -4(%ebp)
__L75: # :=, num, -, x
	fldt	20(%ebp)
	fstpt	-14(%ebp)
__L76: # >=, x, $43, $44
	movl	$0, %ecx
	movl	$1, %edx
	movl	$10, -4(%esp)
	fildl	-4(%esp)
	fldt	-14(%ebp)
	fucomip
	fstp	%st(0)
	cmovnb	%edx, %ecx
	movb	%cl, -15(%ebp)
__L77: # ==, $44, $1, 82
	movl	$0, %ecx
	movl	$1, %edx
	movl	$0, %ebx
	movl	$0, %eax
	movsbl	-15(%ebp), %eax
	cmpl	%ebx, %eax
	je	__L82
__L78: # /, x, $45, $46
	movl	$10, -4(%esp)
	fildl	-4(%esp)
	fldt	-14(%ebp)
	fdivp	%st(1)
	fstpt	-25(%ebp)
__L79: # :=, $46, -, x
	fldt	-25(%ebp)
	fstpt	-14(%ebp)
__L80: # +, exponent, $3, exponent
	movl	$1, %ebx
	movl	-4(%ebp), %eax
	addl	%ebx, %eax
	movl	%eax, -4(%ebp)
__L81: # jump, -, -, 76
	jmp __L76
__L82: # par, exponent, V, -
	movl	-4(%ebp), %eax
	pushl	%eax
__L83: # par, $47, RET, -
	movl	%ebp, %eax
	addl	$-29, %eax
	pushl	%eax
__L84: # call, -, -, write_length
	pushl	%ebp
	call	write_length
	addl	$12, %esp
__L85: # :=, $47, -, exp_length
	movl	-29(%ebp), %eax
	movl	%eax, -33(%ebp)
__L86: # -, space, $48, $49
	movl	16(%ebp), %eax
	movl	$2, %ebx
	subl	%ebx, %eax
	movl	%eax, -37(%ebp)
__L87: # -, $49, num_decimals, $50
	movl	-37(%ebp), %eax
	movl	12(%ebp), %ebx
	subl	%ebx, %eax
	movl	%eax, -41(%ebp)
__L88: # -, $50, $51, $52
	movl	-41(%ebp), %eax
	movl	$1, %ebx
	subl	%ebx, %eax
	movl	%eax, -45(%ebp)
__L89: # -, $52, exp_length, $53
	movl	-45(%ebp), %eax
	movl	-33(%ebp), %ebx
	subl	%ebx, %eax
	movl	%eax, -49(%ebp)
__L90: # :=, $53, -, num_spaces
	movl	-49(%ebp), %eax
	movl	%eax, -53(%ebp)
__L91: # >, num_spaces, $54, $55
	movl	$0, %ecx
	movl	$1, %edx
	movl	$0, %ebx
	movl	-53(%ebp), %eax
	cmpl	%ebx, %eax
	cmovg	%edx, %ecx
	movb	%cl, -54(%ebp)
__L92: # ==, $55, $1, 98
	movl	$0, %ecx
	movl	$1, %edx
	movl	$0, %ebx
	movl	$0, %eax
	movsbl	-54(%ebp), %eax
	cmpl	%ebx, %eax
	je	__L98
__L93: # par, $56, V, -
	movl	$32, %eax
	pushl	%eax
__L94: # par, $7, V, -
	movl	$1, %eax
	pushl	%eax
__L95: # call, -, -, _WRITE_CHAR
	pushl	%ebp
	call	_WRITE_CHAR
	addl	$9, %esp
__L96: # -, num_spaces, $3, num_spaces
	movl	-53(%ebp), %eax
	movl	$1, %ebx
	subl	%ebx, %eax
	movl	%eax, -53(%ebp)
__L97: # jump, -, -, 91
	jmp __L91
__L98: # par, x, V, -
	fldt	-14(%ebp)
	subl	$10, %esp
	fstpt	0(%esp)
__L99: # par, $57, RET, -
	movl	%ebp, %eax
	addl	$-58, %eax
	pushl	%eax
__L100: # call, -, -, _TRUNC2
	pushl	%ebp
	call	_TRUNC2
	addl	$18, %esp
__L101: # :=, $57, -, i
	movl	-58(%ebp), %eax
	movl	%eax, -62(%ebp)
__L102: # par, i, V, -
	movl	-62(%ebp), %eax
	pushl	%eax
__L103: # par, $7, V, -
	movl	$1, %eax
	pushl	%eax
__L104: # call, -, -, _WRITE_INT
	pushl	%ebp
	call	_WRITE_INT
	addl	$12, %esp
__L105: # par, $58, V, -
	movl	$46, %eax
	pushl	%eax
__L106: # par, $7, V, -
	movl	$1, %eax
	pushl	%eax
__L107: # call, -, -, _WRITE_CHAR
	pushl	%ebp
	call	_WRITE_CHAR
	addl	$9, %esp
__L108: # -, x, i, $59
	fildl	-62(%ebp)
	fldt	-14(%ebp)
	fsubp	%st(1)
	fstpt	-72(%ebp)
__L109: # :=, $59, -, x
	fldt	-72(%ebp)
	fstpt	-14(%ebp)
__L110: # <, num_decimals, $60, $61
	movl	$0, %ecx
	movl	$1, %edx
	movl	$10, %ebx
	movl	12(%ebp), %eax
	cmpl	%ebx, %eax
	cmovl	%edx, %ecx
	movb	%cl, -73(%ebp)
__L111: # ==, $61, $1, 116
	movl	$0, %ecx
	movl	$1, %edx
	movl	$0, %ebx
	movl	$0, %eax
	movsbl	-73(%ebp), %eax
	cmpl	%ebx, %eax
	je	__L116
__L112: # par, x, V, -
	fldt	-14(%ebp)
	subl	$10, %esp
	fstpt	0(%esp)
__L113: # par, num_decimals, V, -
	movl	12(%ebp), %eax
	pushl	%eax
__L114: # call, -, -, print_rounded
	pushl	%ebp
	call	print_rounded
	addl	$18, %esp
__L115: # jump, -, -, 119
	jmp __L119
__L116: # par, x, V, -
	fldt	-14(%ebp)
	subl	$10, %esp
	fstpt	0(%esp)
__L117: # par, num_decimals, V, -
	movl	12(%ebp), %eax
	pushl	%eax
__L118: # call, -, -, print_raw
	pushl	%ebp
	call	print_raw
	addl	$18, %esp
__L119: # par, $62, V, -
	movl	$101, %eax
	pushl	%eax
__L120: # par, $7, V, -
	movl	$1, %eax
	pushl	%eax
__L121: # call, -, -, _WRITE_CHAR
	pushl	%ebp
	call	_WRITE_CHAR
	addl	$9, %esp
__L122: # par, exponent, V, -
	movl	-4(%ebp), %eax
	pushl	%eax
__L123: # par, $7, V, -
	movl	$1, %eax
	pushl	%eax
__L124: # call, -, -, _WRITE_INT
	pushl	%ebp
	call	_WRITE_INT
	addl	$12, %esp
__L125: # endu, print_scientific, -, -
	movl	%ebp, %esp
	popl	%ebp
	ret

__L126: # unit, _WRITE_REAL2, -, -
_WRITE_REAL2:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$78, %esp
__L127: # :=, $63, -, minus
	movl	$0, %eax
	movl	%eax, -4(%ebp)
__L128: # <, x, $64, $65
	movl	$0, %ecx
	movl	$1, %edx
	movl	$0, -4(%esp)
	fildl	-4(%esp)
	fldt	20(%ebp)
	fucomip
	fstp	%st(0)
	cmovb	%edx, %ecx
	movb	%cl, -5(%ebp)
__L129: # ==, $65, $1, 137
	movl	$0, %ecx
	movl	$1, %edx
	movl	$0, %ebx
	movl	$0, %eax
	movsbl	-5(%ebp), %eax
	cmpl	%ebx, %eax
	je	__L137
__L130: # -, $4, x, $66
	fldt	20(%ebp)
	movl	$0, -4(%esp)
	fildl	-4(%esp)
	fsubp	%st(1)
	fstpt	-15(%ebp)
__L131: # :=, $66, -, x
	fldt	-15(%ebp)
	fstpt	20(%ebp)
__L132: # :=, $67, -, minus
	movl	$1, %eax
	movl	%eax, -4(%ebp)
__L133: # par, $68, V, -
	movl	$45, %eax
	pushl	%eax
__L134: # par, $7, V, -
	movl	$1, %eax
	pushl	%eax
__L135: # call, -, -, _WRITE_CHAR
	pushl	%ebp
	call	_WRITE_CHAR
	addl	$9, %esp
__L136: # jump, -, -, 137
	jmp __L137
__L137: # >, x, $69, $70
	movl	$0, %ecx
	movl	$1, %edx
	movl	$2147438647, -4(%esp)
	fildl	-4(%esp)
	fldt	20(%ebp)
	fucomip
	fstp	%st(0)
	cmovnbe	%edx, %ecx
	movb	%cl, -16(%ebp)
__L138: # ==, $70, $1, 146
	movl	$0, %ecx
	movl	$1, %edx
	movl	$0, %ebx
	movl	$0, %eax
	movsbl	-16(%ebp), %eax
	cmpl	%ebx, %eax
	je	__L146
__L139: # -, space, minus, $71
	movl	16(%ebp), %eax
	movl	-4(%ebp), %ebx
	subl	%ebx, %eax
	movl	%eax, -20(%ebp)
__L140: # par, x, V, -
	fldt	20(%ebp)
	subl	$10, %esp
	fstpt	0(%esp)
__L141: # par, $71, V, -
	movl	-20(%ebp), %eax
	pushl	%eax
__L142: # par, num_decimals, V, -
	movl	12(%ebp), %eax
	pushl	%eax
__L143: # call, -, -, print_scientific
	pushl	%ebp
	call	print_scientific
	addl	$22, %esp
__L144: # ret, -, -, -
	movl	%ebp, %esp
	popl	%ebp
	ret
__L145: # jump, -, -, 146
	jmp __L146
__L146: # par, x, V, -
	fldt	20(%ebp)
	subl	$10, %esp
	fstpt	0(%esp)
__L147: # par, $72, RET, -
	movl	%ebp, %eax
	addl	$-24, %eax
	pushl	%eax
__L148: # call, -, -, _TRUNC2
	pushl	%ebp
	call	_TRUNC2
	addl	$18, %esp
__L149: # :=, $72, -, i
	movl	-24(%ebp), %eax
	movl	%eax, -28(%ebp)
__L150: # par, i, V, -
	movl	-28(%ebp), %eax
	pushl	%eax
__L151: # par, $73, RET, -
	movl	%ebp, %eax
	addl	$-32, %eax
	pushl	%eax
__L152: # call, -, -, write_length
	pushl	%ebp
	call	write_length
	addl	$12, %esp
__L153: # :=, $73, -, length
	movl	-32(%ebp), %eax
	movl	%eax, -36(%ebp)
__L154: # -, x, i, $74
	fildl	-28(%ebp)
	fldt	20(%ebp)
	fsubp	%st(1)
	fstpt	-46(%ebp)
__L155: # :=, $74, -, remainder
	fldt	-46(%ebp)
	fstpt	-56(%ebp)
__L156: # -, space, minus, $75
	movl	16(%ebp), %eax
	movl	-4(%ebp), %ebx
	subl	%ebx, %eax
	movl	%eax, -60(%ebp)
__L157: # -, $75, length, $76
	movl	-60(%ebp), %eax
	movl	-36(%ebp), %ebx
	subl	%ebx, %eax
	movl	%eax, -64(%ebp)
__L158: # -, $76, $77, $78
	movl	-64(%ebp), %eax
	movl	$1, %ebx
	subl	%ebx, %eax
	movl	%eax, -68(%ebp)
__L159: # -, $78, num_decimals, $79
	movl	-68(%ebp), %eax
	movl	12(%ebp), %ebx
	subl	%ebx, %eax
	movl	%eax, -72(%ebp)
__L160: # :=, $79, -, num_spaces
	movl	-72(%ebp), %eax
	movl	%eax, -76(%ebp)
__L161: # >, num_spaces, $80, $81
	movl	$0, %ecx
	movl	$1, %edx
	movl	$0, %ebx
	movl	-76(%ebp), %eax
	cmpl	%ebx, %eax
	cmovg	%edx, %ecx
	movb	%cl, -77(%ebp)
__L162: # ==, $81, $1, 168
	movl	$0, %ecx
	movl	$1, %edx
	movl	$0, %ebx
	movl	$0, %eax
	movsbl	-77(%ebp), %eax
	cmpl	%ebx, %eax
	je	__L168
__L163: # par, $82, V, -
	movl	$32, %eax
	pushl	%eax
__L164: # par, $7, V, -
	movl	$1, %eax
	pushl	%eax
__L165: # call, -, -, _WRITE_CHAR
	pushl	%ebp
	call	_WRITE_CHAR
	addl	$9, %esp
__L166: # -, num_spaces, $3, num_spaces
	movl	-76(%ebp), %eax
	movl	$1, %ebx
	subl	%ebx, %eax
	movl	%eax, -76(%ebp)
__L167: # jump, -, -, 161
	jmp __L161
__L168: # par, i, V, -
	movl	-28(%ebp), %eax
	pushl	%eax
__L169: # par, $7, V, -
	movl	$1, %eax
	pushl	%eax
__L170: # call, -, -, _WRITE_INT
	pushl	%ebp
	call	_WRITE_INT
	addl	$12, %esp
__L171: # par, $83, V, -
	movl	$46, %eax
	pushl	%eax
__L172: # par, $7, V, -
	movl	$1, %eax
	pushl	%eax
__L173: # call, -, -, _WRITE_CHAR
	pushl	%ebp
	call	_WRITE_CHAR
	addl	$9, %esp
__L174: # <, num_decimals, $84, $85
	movl	$0, %ecx
	movl	$1, %edx
	movl	$10, %ebx
	movl	12(%ebp), %eax
	cmpl	%ebx, %eax
	cmovl	%edx, %ecx
	movb	%cl, -78(%ebp)
__L175: # ==, $85, $1, 180
	movl	$0, %ecx
	movl	$1, %edx
	movl	$0, %ebx
	movl	$0, %eax
	movsbl	-78(%ebp), %eax
	cmpl	%ebx, %eax
	je	__L180
__L176: # par, remainder, V, -
	fldt	-56(%ebp)
	subl	$10, %esp
	fstpt	0(%esp)
__L177: # par, num_decimals, V, -
	movl	12(%ebp), %eax
	pushl	%eax
__L178: # call, -, -, print_rounded
	pushl	%ebp
	call	print_rounded
	addl	$18, %esp
__L179: # jump, -, -, 183
	jmp __L183
__L180: # par, remainder, V, -
	fldt	-56(%ebp)
	subl	$10, %esp
	fstpt	0(%esp)
__L181: # par, num_decimals, V, -
	movl	12(%ebp), %eax
	pushl	%eax
__L182: # call, -, -, print_raw
	pushl	%ebp
	call	print_raw
	addl	$18, %esp
__L183: # endu, _WRITE_REAL2, -, -
	movl	%ebp, %esp
	popl	%ebp
	ret

