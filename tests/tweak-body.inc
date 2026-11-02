    ldr w11,[x10,#8]
    cbnz w11, Ldone
    ldp w8,w9,[x12]
    cmp w8,#1
    b.lt Ldone
    cmp w9,#1
    b.lt Ldone
    sub w11,w0,#1
    cmp w11,#2
    b.lo Lportrait
    sub w11,w0,#3
    cmp w11,#2
    b.hs Ldone
    cmp w8,w9
    b.ge Ldone
    b Lswap
Lportrait:
    cmp w9,w8
    b.ge Ldone
Lswap:
    stp w9,w8,[x12]
Ldone:
    ret
