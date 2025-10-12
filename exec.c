/* SimAK65 instruction execution
 * Copyright A.K. 2018, 2023
 */

#include "error.h"
#include "exec.h"
#include "alu.h"
#include "flags.h"
#include "simak65.h"

#define IRQ_VECTOR 0xfffe
#define RST_VECTOR 0xfffc
#define NMI_VECTOR 0xfffa

static void exec_push(struct simak65_cpu *cpu, u8 data)
{
	u16 addr;

	addr = 0x0100 | cpu->reg.sp;
	--cpu->reg.sp;

	if (cpu->reg.sp == 0xff)
		WARN("Stack pointer wrap-around");

	DEBUG("Pushing 0x%02x to stack: 0x%04x", data, addr);

	cpu->bus.write(addr, data);
}

static u8 exec_pop(struct simak65_cpu *cpu)
{
	u16 addr;
	u8 data;

	++cpu->reg.sp;
	addr = 0x0100 | cpu->reg.sp;

	if (cpu->reg.sp == 0)
		WARN("Stack pointer wrap-around");

	data = cpu->bus.read(addr);

	DEBUG("Popped 0x%02x from stack: 0x%04x", data, addr);

	return data;
}

static void exec_adc(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	cpu->reg.a = alu_add(cpu->reg.a, arg, &cpu->reg.flags);

	DEBUG("Adding 0x%02x to Acc, result 0x%02x", arg, cpu->reg.a);
}

static void exec_and(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	cpu->reg.a = alu_and(cpu->reg.a, arg, &cpu->reg.flags);

	DEBUG("Performing AND 0x%02x, Acc, result 0x%02x", arg, cpu->reg.a);
}

static void exec_asl(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;
	u8 result;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	result = alu_asl(arg, 0, &cpu->reg.flags);

	DEBUG("Performing ASL of 0x%02x, result 0x%02x", arg, result);

	if (argtype == arg_addr) {
		cpu->bus.write(addr, result);
		cpu->cycles += 1;
	}
	else {
		cpu->reg.a = result;
	}
}

static void exec_bcc(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;

	u16 addr;

	addr = (args[1] << 8) | args[0];

	if (!(cpu->reg.flags & FLAG_CARRY)) {
		DEBUG("BCC branch taken, new pc 0x%04x", addr);
#ifndef NDEBUG
		if (addr == cpu->reg.pc - 2)
			FATAL("Tight loop");
#endif
		cpu->reg.pc = addr;
		cpu->cycles += 1;
	}
	else {
		DEBUG("BCC branch not taken");
	}
}

static void exec_bcs(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;

	u16 addr;

	addr = (args[1] << 8) | args[0];

	if (cpu->reg.flags & FLAG_CARRY) {
		DEBUG("BCS branch taken, new pc 0x%04x", addr);
#ifndef NDEBUG
		if (addr == cpu->reg.pc - 2)
			FATAL("Tight loop");
#endif
		cpu->reg.pc = addr;
		cpu->cycles += 1;
	}
	else {
		DEBUG("BCS branch not taken");
	}
}

static void exec_beq(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;

	u16 addr;

	addr = (args[1] << 8) | args[0];

	if (cpu->reg.flags & FLAG_ZERO) {
		DEBUG("BEQ branch taken, new pc 0x%04x", addr);
#ifndef NDEBUG
		if (addr == cpu->reg.pc - 2)
			FATAL("Tight loop");
#endif
		cpu->reg.pc = addr;
		cpu->cycles += 1;
	}
	else {
		DEBUG("BEQ branch not taken");
	}
}

static void exec_bit(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	alu_bit(cpu->reg.a, arg, &cpu->reg.flags);

	DEBUG("Performing BIT A: 0x%02x and 0x%02x", cpu->reg.a, arg);
}

static void exec_bmi(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;

	u16 addr;

	addr = (args[1] << 8) | args[0];

	if (cpu->reg.flags & FLAG_SIGN) {
		DEBUG("BMI branch taken, new pc 0x%04x", addr);
#ifndef NDEBUG
		if (addr == cpu->reg.pc - 2)
			FATAL("Tight loop");
#endif
		cpu->reg.pc = addr;
		cpu->cycles += 1;
	}
	else {
		DEBUG("BMI branch not taken");
	}
}

static void exec_bne(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;

	u16 addr;

	addr = (args[1] << 8) | args[0];

	if (!(cpu->reg.flags & FLAG_ZERO)) {
		DEBUG("BNE branch taken, new pc 0x%04x", addr);
#ifndef NDEBUG
		if (addr == cpu->reg.pc - 2)
			FATAL("Tight loop");
#endif
		cpu->reg.pc = addr;
		cpu->cycles += 1;
	}
	else {
		DEBUG("BNE branch not taken");
	}
}

static void exec_bpl(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;

	u16 addr;

	addr = (args[1] << 8) | args[0];

	if (!(cpu->reg.flags & FLAG_SIGN)) {
		DEBUG("BPL branch taken, new pc 0x%04x", addr);
#ifndef NDEBUG
		if (addr == cpu->reg.pc - 2)
			FATAL("Tight loop");
#endif
		cpu->reg.pc = addr;
		cpu->cycles += 1;
	}
	else {
		DEBUG("BPL branch not taken");
	}
}

static void exec_brk(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	u8 flags;
	u16 addr;

	cpu->reg.pc += 1;
	exec_push(cpu, (cpu->reg.pc >> 8) & 0xff);
	exec_push(cpu, cpu->reg.pc & 0xff);

	flags = cpu->reg.flags;
	flags |= FLAG_ONE | FLAG_BRK;
	exec_push(cpu, flags);

	cpu->reg.flags |= FLAG_IRQD;

	addr = cpu->bus.read(IRQ_VECTOR);
	addr |= ((u16)cpu->bus.read(IRQ_VECTOR + 1) << 8);

	DEBUG("Performing BRK, old pc: 0x%04x, new pc: 0x%04x", cpu->reg.pc, addr);

	cpu->reg.pc = addr;

	cpu->cycles += 4;
}

static void exec_bvc(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;

	u16 addr;

	addr = (args[1] << 8) | args[0];

	if (!(cpu->reg.flags & FLAG_OVRF)) {
		DEBUG("BVC branch taken, new pc 0x%04x", addr);
#ifndef NDEBUG
		if (addr == cpu->reg.pc - 2)
			FATAL("Tight loop");
#endif
		cpu->reg.pc = addr;
		cpu->cycles += 1;
	}
	else {
		DEBUG("BVC branch not taken");
	}
}

static void exec_bvs(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;

	u16 addr;

	addr = (args[1] << 8) | args[0];

	if (cpu->reg.flags & FLAG_OVRF) {
		DEBUG("BVS branch taken, new pc 0x%04x", addr);
#ifndef NDEBUG
		if (addr == cpu->reg.pc - 2)
			FATAL("Tight loop");
#endif
		cpu->reg.pc = addr;
		cpu->cycles += 1;
	}
	else {
		DEBUG("BVS branch not taken");
	}
}

static void exec_clc(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;


	cpu->reg.flags &= ~FLAG_CARRY;

	DEBUG("Performing CLC");

	cpu->cycles += 1;
}

static void exec_cld(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.flags &= ~FLAG_BCD;

	DEBUG("Performing CLD");

	cpu->cycles += 1;
}

static void exec_cli(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.flags &= ~FLAG_IRQD;

	DEBUG("Performing CLI");

	cpu->cycles += 1;
}

static void exec_clv(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.flags &= ~FLAG_OVRF;

	DEBUG("Performing CLV");

	cpu->cycles += 1;
}

static void exec_cmp(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	alu_cmp(cpu->reg.a, arg, &cpu->reg.flags);

	DEBUG("Performing CMP A: 0x%02x and 0x%02x", cpu->reg.a, arg);
}

static void exec_cpx(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	alu_cmp(cpu->reg.x, arg, &cpu->reg.flags);

	DEBUG("Performing CPX X: 0x%02x and 0x%02x", cpu->reg.x, arg);
}

static void exec_cpy(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	alu_cmp(cpu->reg.y, arg, &cpu->reg.flags);

	DEBUG("Performing CPY Y: 0x%02x and 0x%02x", cpu->reg.y, arg);
}

static void exec_dec(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;
	u8 result;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	result = alu_dec(arg, 0, &cpu->reg.flags);

	DEBUG("Performing DEC of 0x%02x, result 0x%02x", arg, result);

	if (argtype == arg_addr) {
		cpu->bus.write(addr, result);
		cpu->cycles += 1;
	}
	else {
		cpu->reg.a = result;
	}
}

static void exec_dex(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.x = alu_dec(cpu->reg.x, 0, &cpu->reg.flags);

	DEBUG("Performing DEX, result 0x%02x", cpu->reg.x);

	cpu->cycles += 1;
}

static void exec_dey(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.y = alu_dec(cpu->reg.y, 0, &cpu->reg.flags);

	DEBUG("Performing DEY, result 0x%02x", cpu->reg.y);

	cpu->cycles += 1;
}

static void exec_eor(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	cpu->reg.a = alu_eor(cpu->reg.a, arg, &cpu->reg.flags);

	DEBUG("Performing EOR 0x%02x, Acc, result 0x%02x", arg, cpu->reg.a);
}

static void exec_inc(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;
	u8 result;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	result = alu_inc(arg, 0, &cpu->reg.flags);

	DEBUG("Performing INC of 0x%02x, result 0x%02x", arg, result);

	if (argtype == arg_addr) {
		cpu->bus.write(addr, result);
		cpu->cycles += 1;
	}
	else {
		cpu->reg.a = result;
	}
}

static void exec_inx(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.x = alu_inc(cpu->reg.x, 0, &cpu->reg.flags);

	DEBUG("Performing INX, result 0x%02x", cpu->reg.x);

	cpu->cycles += 1;
}

static void exec_iny(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.y = alu_inc(cpu->reg.y, 0, &cpu->reg.flags);

	DEBUG("Performing INY, result 0x%02x", cpu->reg.y);

	cpu->cycles += 1;
}

static void exec_jmp(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;

	u16 addr;

	addr = (args[1] << 8) | args[0];

	DEBUG("Performing JMP, old pc: 0x%04x, new pc: 0x%04x", cpu->reg.pc, addr);

	cpu->reg.pc = addr;

	cpu->cycles += 1;
}

static void exec_jsr(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;

	u16 addr;

	addr = cpu->reg.pc - 1;

	exec_push(cpu, (addr >> 8) & 0xff);
	exec_push(cpu, addr & 0xff);

	addr = (args[1] << 8) | args[0];

	DEBUG("Performing JSR, old pc: 0x%04x, new pc: 0x%04x", cpu->reg.pc, addr);

	cpu->reg.pc = addr;

	cpu->cycles += 2;
}

static void exec_lda(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	DEBUG("Performing LDA of 0x%02x", arg);

	cpu->reg.a = alu_load(arg, 0, &cpu->reg.flags);
}

static void exec_ldx(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	DEBUG("Performing LDX of 0x%02x", arg);

	cpu->reg.x = alu_load(arg, 0, &cpu->reg.flags);
}

static void exec_ldy(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	DEBUG("Performing LDY of 0x%02x", arg);

	cpu->reg.y = alu_load(arg, 0, &cpu->reg.flags);
}

static void exec_lsr(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;
	u8 result;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	result = alu_lsr(arg, 0, &cpu->reg.flags);

	DEBUG("Performing LSR of 0x%02x, result 0x%02x", arg, result);

	if (argtype == arg_addr) {
		cpu->bus.write(addr, result);
		cpu->cycles += 1;
	}
	else {
		cpu->reg.a = result;
	}
}

static void exec_nop(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)cpu;
	(void)argtype;
	(void)args;

	cpu->cycles += 1;
}

static void exec_ora(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	cpu->reg.a = alu_or(cpu->reg.a, arg, &cpu->reg.flags);

	DEBUG("Performing ORA 0x%02x, Acc, result 0x%02x", arg, cpu->reg.a);
}

static void exec_pha(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	exec_push(cpu, cpu->reg.a);

	DEBUG("Performing PHA");

	cpu->cycles += 2;
}

static void exec_php(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	u8 flags;

	flags = cpu->reg.flags;
	flags |= FLAG_ONE | FLAG_BRK;

	exec_push(cpu, flags);

	DEBUG("Performing PHP");

	cpu->cycles += 2;
}

static void exec_pla(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.a = alu_load(exec_pop(cpu), 0, &cpu->reg.flags);

	DEBUG("Performing PLA");

	cpu->cycles += 2;
}

static void exec_plp(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.flags = exec_pop(cpu);
	cpu->reg.flags &= FLAG_CARRY | FLAG_ZERO | FLAG_IRQD | FLAG_BCD | FLAG_OVRF | FLAG_SIGN;

	DEBUG("Performing PLP");

	cpu->cycles += 2;
}

static void exec_rol(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;
	u8 result;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	result = alu_rol(arg, 0, &cpu->reg.flags);

	DEBUG("Performing ROL of 0x%02x, result 0x%02x", arg, result);

	if (argtype == arg_addr) {
		cpu->bus.write(addr, result);
		cpu->cycles += 1;
	}
	else {
		cpu->reg.a = result;
	}
}

static void exec_ror(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;
	u8 result;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	result = alu_ror(arg, 0, &cpu->reg.flags);

	DEBUG("Performing ROR of 0x%02x, result 0x%02x", arg, result);

	if (argtype == arg_addr) {
		cpu->bus.write(addr, result);
		cpu->cycles += 1;
	}
	else {
		cpu->reg.a = result;
	}
}

static void exec_rti(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	u16 addr;

	cpu->reg.flags = exec_pop(cpu);
	cpu->reg.flags &= FLAG_CARRY | FLAG_ZERO | FLAG_IRQD | FLAG_BCD | FLAG_OVRF | FLAG_SIGN;

	addr = exec_pop(cpu);
	addr |= (u16)exec_pop(cpu) << 8;

	DEBUG("Performing RTI, old pc: 0x%04x, new pc: 0x%04x", cpu->reg.pc, addr);

	cpu->reg.pc = addr;

	cpu->cycles += 3;
}

static void exec_rts(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	u16 addr;

	addr = exec_pop(cpu);
	addr |= (u16)exec_pop(cpu) << 8;
	addr += 1;

	DEBUG("Performing RTS, old pc: 0x%04x, new pc: 0x%04x", cpu->reg.pc, addr);

	cpu->reg.pc = addr;

	cpu->cycles += 2;
}

static void exec_sbc(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;
	u8 arg;

	if (argtype == arg_addr) {
		addr = ((u16)args[1] << 8) | args[0];
		arg = cpu->bus.read(addr);
		cpu->cycles += 2;
	}
	else {
		arg = args[0];
		cpu->cycles += 1;
	}

	cpu->reg.a = alu_sub(cpu->reg.a, arg, &cpu->reg.flags);

	DEBUG("Subtracting 0x%02x from Acc, result 0x%02x", arg, cpu->reg.a);
}

static void exec_sec(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.flags |= FLAG_CARRY;

	DEBUG("Executing SEC");

	cpu->cycles += 1;
}

static void exec_sed(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.flags |= FLAG_BCD;

	DEBUG("Executing SED");

	cpu->cycles += 1;
}

static void exec_sei(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.flags |= FLAG_IRQD;

	DEBUG("Executing SEI");

	cpu->cycles += 1;
}

static void exec_sta(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;

	if (argtype != arg_addr)
		FATAL("STA: Invalid argument type != arg_addr");

	addr = ((u16)args[1] << 8) | args[0];

	cpu->bus.write(addr, cpu->reg.a);

	DEBUG("Stored A register at 0x%04x", addr);

	cpu->cycles += 2;
}

static void exec_stx(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;

	if (argtype != arg_addr)
		FATAL("STX: Invalid argument type != arg_addr");

	addr = ((u16)args[1] << 8) | args[0];

	cpu->bus.write(addr, cpu->reg.x);

	DEBUG("Stored X register at 0x%04x", addr);

	cpu->cycles += 2;
}

static void exec_sty(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	u16 addr;

	if (argtype != arg_addr)
		FATAL("STY: Invalid argument type != arg_addr");

	addr = ((u16)args[1] << 8) | args[0];

	cpu->bus.write(addr, cpu->reg.y);

	DEBUG("Stored Y register at 0x%04x", addr);

	cpu->cycles += 2;
}

static void exec_tax(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.x = alu_load(cpu->reg.a, 0, &cpu->reg.flags);

	DEBUG("Executing TAX");

	cpu->cycles += 1;
}

static void exec_tay(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.y = alu_load(cpu->reg.a, 0, &cpu->reg.flags);

	DEBUG("Executing TAY");

	cpu->cycles += 1;
}

static void exec_tsx(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.x = alu_load(cpu->reg.sp, 0, &cpu->reg.flags);

	DEBUG("Executing TSX");

	cpu->cycles += 1;
}

static void exec_txa(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.a = alu_load(cpu->reg.x, 0, &cpu->reg.flags);

	DEBUG("Executing TXA");

	cpu->cycles += 1;
}

static void exec_txs(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.sp = cpu->reg.x;

	DEBUG("Executing TXS");

	cpu->cycles += 1;
}

static void exec_tya(struct simak65_cpu *cpu, enum argtype argtype, u8 *args)
{
	(void)argtype;
	(void)args;

	cpu->reg.a = alu_load(cpu->reg.y, 0, &cpu->reg.flags);

	DEBUG("Executing TYA");

	cpu->cycles += 1;
}

typedef void (*exec_func_t)(struct simak65_cpu *, enum argtype, u8 *);
static const exec_func_t exec_instr[] = {
	exec_adc, exec_and, exec_asl, exec_bcc, exec_bcs, exec_beq, exec_bit, exec_bmi,
	exec_bne, exec_bpl, exec_brk, exec_bvc, exec_bvs, exec_clc, exec_cld, exec_cli,
	exec_clv, exec_cmp, exec_cpx, exec_cpy, exec_dec, exec_dex, exec_dey, exec_eor,
	exec_inc, exec_inx, exec_iny, exec_jmp, exec_jsr, exec_lda, exec_ldx, exec_ldy,
	exec_lsr, exec_nop, exec_ora, exec_pha, exec_php, exec_pla, exec_plp, exec_rol,
	exec_ror, exec_rti, exec_rts, exec_sbc, exec_sec, exec_sed, exec_sei, exec_sta,
	exec_stx, exec_sty, exec_tax, exec_tay, exec_tsx, exec_txa, exec_txs, exec_tya
};

void exec_execute(struct simak65_cpu *cpu, enum opcode instruction, enum argtype argtype, u8 *args)
{
	if (instruction > sizeof(exec_instr) / sizeof(exec_instr[0])) {
		FATAL("Tried to execute unknown instruction %u", instruction);
	}

	exec_instr[instruction](cpu, argtype, args);
}

void exec_irq(struct simak65_cpu *cpu)
{
	u8 flags;

	DEBUG("Received IRQ");

	exec_push(cpu, (cpu->reg.pc >> 8) & 0xff);
	exec_push(cpu, cpu->reg.pc & 0xff);

	flags = cpu->reg.flags;
	flags |= FLAG_ONE;
	flags &= ~FLAG_BRK;
	exec_push(cpu, flags);

	cpu->reg.pc = cpu->bus.read(IRQ_VECTOR);
	cpu->reg.pc |= (u16)cpu->bus.read(IRQ_VECTOR + 1) << 8;

	cpu->reg.flags |= FLAG_IRQD;

	cpu->cycles += 7;
}

void exec_nmi(struct simak65_cpu *cpu)
{
	u8 flags;

	DEBUG("Received NMI");

	exec_push(cpu, (cpu->reg.pc >> 8) & 0xff);
	exec_push(cpu, cpu->reg.pc & 0xff);

	flags = cpu->reg.flags;
	flags |= FLAG_ONE;
	flags &= ~FLAG_BRK;
	exec_push(cpu, flags);

	cpu->reg.pc = cpu->bus.read(NMI_VECTOR);
	cpu->reg.pc |= (u16)cpu->bus.read(NMI_VECTOR + 1) << 8;

	cpu->reg.flags |= FLAG_IRQD;

	cpu->cycles += 7;
}

void exec_rst(struct simak65_cpu *cpu)
{
	DEBUG("Received RST");

	cpu->reg.a = 0;
	cpu->reg.x = 0;
	cpu->reg.y = 0;
	cpu->reg.flags = FLAG_ONE;
	cpu->reg.sp = 0xff;

	cpu->reg.pc = cpu->bus.read(RST_VECTOR);
	cpu->reg.pc |= (u16)cpu->bus.read(RST_VECTOR + 1) << 8;

	cpu->cycles += 4;
}
