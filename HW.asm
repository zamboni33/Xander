     .ORIG x3000
     AND R5, R5, #0
     AND R3, R3, #0
     ADD R3, R3, #8
     LEA R0, B
     LDW R1, R0, #1
     LDW R1, R1, #0
     ADD R2, R1, #0
AGAIN     ADD R2, R2, R2
     ADD R3, R3, #-1
     BRp AGAIN
     LDW R4, R0, #0
     AND R1, R1, R4
     NOT R1, R1
     ADD R1, R1, #1
     ADD R2, R2, R1
     BRnp NO
     ADD R5, R5, #1
NO     HALT
B     .FILL XFF00
A     .FILL X4000
    .END
