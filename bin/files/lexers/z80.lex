.sid z80
.name Assembler Z80
.type asm

.ext z80|zx|a80|azx

.register
af bc de hl a f b c d e h l
af' bc' de' hl' a' f' b' c' d' e' h' l'
sp pc i ix iy ixh ixl iyh iyl r

.cpu_instruction
adc add and bit call ccf cp cpd cpdr cpi cpir cpl
daa dec di djnz ei ex exx halt 
im inc ind indr ini inir jp jr ld ldd lddr ldi ldir
neg nop or otdr otir out outd outi pop push 
res ret reti retn rl rla rlc rlca rld rr rra rrc rrca rrd rst
sbc scf set sla sra srl sub xor

.fpu_instruction

.ext_instruction

.directive
org equ db dw ds defb defw defs include if else endif 

