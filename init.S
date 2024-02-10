.text

.globl _start
.globl printf

.globl ir_generic
.globl get_mstatus
.globl get_sp
.globl get_ticks
.globl delay_cyc
.globl delay_us
.globl delay_ms

.macro pusht
addi	sp,sp,-64
sw	ra,4(sp)
sw	t0,8(sp)
sw	t1,12(sp)
sw	a0,16(sp)
sw	a1,20(sp)
sw	a2,24(sp)
sw	a3,28(sp)
sw	a4,32(sp)
sw	a5,36(sp)
sw	a6,40(sp)
sw	a7,44(sp)
sw	t3,48(sp)
sw	t4,52(sp)
sw	t5,56(sp)
sw	t6,60(sp)
.endm

.macro popt
lw	ra,4(sp)
lw	t0,8(sp)
lw	t1,12(sp)
lw	a0,16(sp)
lw	a1,20(sp)
lw	a2,24(sp)
lw	a3,28(sp)
lw	a4,32(sp)
lw	a5,36(sp)
lw	a6,40(sp)
lw	a7,44(sp)
lw	t3,48(sp)
lw	t4,52(sp)
lw	t5,56(sp)
lw	t6,60(sp)
addi	sp,sp,64
.endm

ivt:
j _hlt
nop
j _hlt
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop
j _intr
nop

_intr:
pusht
csrr a0, mcause
andi a0, a0, 0x1f
jal ir_generic
popt
mret

get_mstatus:
csrr a0, mstatus
ret

get_sp:
add a0, zero, sp
ret

get_ticks:
la a2, SYSTIMER_UNIT0_OP_REG
lui a0, 0x40000
sw a0, 0(a2)
srli a0, a0, 1
1: lw a1, 0(a2)
and a1, a1, a0
beq zero, a1, 1b
lw a1, 0x3c(a2)
lw a0, 0x40(a2)
ret

delay_cyc:
addi a0, a0, -4
slli a0, a0, 1
1: addi a0, a0, -2
bne zero, a0, 1b
ret
delay_us:
slli a0, a0, 6
addi a0, a0, -8
j 1b

delay_ms:
li a1, 1000
mul a0, a0, a1
j delay_us

_start:
csrci mstatus, 8

# Set clock to 160 MHz
la t0, SYSTEM_CPU_PER_CONF_REG
lw a0, 0(t0)
andi a0, a0, -4
ori a0, a0, 5
sw a0, 0(t0)

la t0, SYSTEM_SYSCLK_CONF_REG
lui a0, 168
ori a0, a0, 1024
sw a0, 0(t0)

li a0, 160
la a1, ets_update_cpu_frequency
jalr a1

# Disable WDTs
la t0, TIMG0_REGCLK_REG
li a0, 0xdfffffff
sw a0, 0(t0)
la t0, TIMG1_REGCLK_REG
sw a0, 0(t0)
la t0, TIMG0_WDTCONFIG0_REG
sw zero, 0(t0)
la t0, TIMG1_WDTCONFIG0_REG
sw zero, 0(t0)

la t0, RTC_CNTL_SWD_WPROTECT_REG
la t1, RTC_CNTL_SWD_CONF_REG
li a0, 0x8F1D312A
sw a0, 0(t0)
lw a0, 0(t1)
lui a1, 0x80000
or a0, a0, a1
sw a0, 0(t1)
la t1, RTC_CNTL_SW_CPU_STALL_REG
sw zero, 0(t1)
sw zero, 0(t0)

# Podesavenje addrese IVT
la t0, ivt
ori t0, t0, 1
csrw mtvec, t0
fence
csrsi mstatus, 8
mv a0, sp
jal start

_hlt: # Ovo je fatal exception, iz ovoga se ne izlazi
la a0, fatal_str
csrr a1, mcause
la a2, printf
jalr a2
1: j 1b

.data

fatal_str:
.asciz "\nKobna greska %d\n"
