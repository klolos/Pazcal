.data
__e: .tfloat 2.7182818284
tmp: .long 0

.text
.global _exp2
.global start_exp2

_exp2:
        pushl   %ebp
        movl    %esp, %ebp

start_exp2:                     # st(0), st(1), ...
        fldt    16(%ebp)        # x
        fldt    __e             # x, e
        fyl2x                   # x * log2(e)
        fld     %st             # x * log2(e), x * log2(e)
        fisttpl tmp
        fildl   tmp             # int(x * log2(e)), x * log2(e)
        fxch    %st(1)          # x * log2(e), int(x * log2(e))
        fsub    %st(1), %st(0)  # frac(x * log2(e)), int(x * log2(e))
        f2xm1                   # 2 ^ frac(x * log2(e)) - 1, int(x * log2(e))
        
        fld1                    # 1, 2 ^ frac(x * log2(e)) - 1, int(x * log2(e))
        faddp   %st(1)          # 2 ^ frac(x * log2(e)), int(x * log2(e))
        fscale                  # (2 ^ frac(x * log2(e))) * 2 ^ int(x * log2(e)) = e^x
        fstp    %st(1)
        
        movl    12(%ebp), %esi
        fstpt   (%esi)

        movl    %ebp, %esp
        popl    %ebp
        ret
