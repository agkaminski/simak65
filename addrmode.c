/* SimAK65 addresing mode
 * Copyright A.K. 2018, 2023
 */

#include "error.h"
#include "addrmode.h"
#include "decoder.h"
#include "simak65.h"


static enum argtype modeAcc(struct simak65_cpu *cpu, u8 *args)
{
	args[0] = cpu->reg.a;

	DEBUG("Accumulator mode, args : 0x%02x", args[0]);

	return arg_byte;
}

static enum argtype modeAbsolute(struct simak65_cpu *cpu, u8 *args)
{
	args[0] = addrmode_nextpc(cpu);
	args[1] = addrmode_nextpc(cpu);

	DEBUG("Absolute mode, args : 0x%02x%02x", args[1], args[0]);

	cpu->cycles += 3;

	return arg_addr;
}

static enum argtype modeAbsoluteX(struct simak65_cpu *cpu, u8 *args)
{
	u16 addr;

	addr = addrmode_nextpc(cpu);
	addr |= (u16)addrmode_nextpc(cpu) << 8;
	addr += cpu->reg.x;

	args[0] = addr & 0xff;
	args[1] = (addr >> 8) & 0xff;

	DEBUG("Absolute, X mode, args : 0x%02x%02x", args[1], args[0]);

	cpu->cycles += 3;

	return arg_addr;
}

static enum argtype modeAbsoluteY(struct simak65_cpu *cpu, u8 *args)
{
	u16 addr;

	addr = addrmode_nextpc(cpu);
	addr |= (u16)addrmode_nextpc(cpu) << 8;
	addr += cpu->reg.y;

	args[0] = addr & 0xff;
	args[1] = (addr >> 8) & 0xff;

	DEBUG("Absolute, Y mode, args : 0x%02x%02x", args[1], args[0]);

	cpu->cycles += 3;

	return arg_addr;
}

static enum argtype modeImmediate(struct simak65_cpu *cpu, u8 *args)
{
	args[0] = addrmode_nextpc(cpu);

	DEBUG("Immediate mode, args: 0x%02x", args[0]);

	cpu->cycles += 1;

	return arg_byte;
}

static enum argtype modeImplicant(struct simak65_cpu *cpu, u8 *args)
{
	(void)cpu;
	(void)args;

	DEBUG("Implicant mode, no args");

	return arg_none;
}

static enum argtype modeIndirect(struct simak65_cpu *cpu, u8 *args)
{
	u16 addr;

	addr = addrmode_nextpc(cpu);
	addr |= (u16)addrmode_nextpc(cpu) << 8;

	args[0] = cpu->bus.read(addr++);
	args[1] = cpu->bus.read(addr);

	DEBUG("Indirect mode, args: 0x%02x%02x from addr: 0x%04x", args[1], args[0], addr);

	cpu->cycles += 7;

	return arg_addr;
}

static enum argtype modeIndirectX(struct simak65_cpu *cpu, u8 *args)
{
	u16 addr;

	addr = addrmode_nextpc(cpu);
	addr += cpu->reg.x;
	addr &= 0xff;

	args[0] = cpu->bus.read(addr++);
	args[1] = cpu->bus.read(addr);

	DEBUG("Indexed indirect mode, args: 0x%02x%02x from addr: 0x%04x", args[1], args[0], addr);

	cpu->cycles += 5;

	return arg_addr;
}

static enum argtype modeIndirectY(struct simak65_cpu *cpu, u8 *args)
{
	u16 zpAddr;
	u16 addr;

	zpAddr = addrmode_nextpc(cpu);

	addr = cpu->bus.read(zpAddr++);
	addr |= (u16)cpu->bus.read(zpAddr) << 8;

	addr += cpu->reg.y;

	args[0] = addr & 0xff;
	args[1] = (addr >> 8) & 0xff;

	DEBUG("Indirect indexed mode, args: 0x%02x%02x from addr: 0x%04x", args[1], args[0], zpAddr);

	cpu->cycles += 5;

	return arg_addr;
}

static enum argtype modeRelative(struct simak65_cpu *cpu, u8 *args)
{
	s8 rel;
	u16 addr;

	rel = addrmode_nextpc(cpu);
	addr = cpu->reg.pc;
	addr += rel;

	args[0] = addr & 0xff;
	args[1] = (addr >> 8) & 0xff;

	DEBUG("Relative mode, args: 0x%02x%02x = pc + rel: 0x%02x", args[1], args[0], rel);

	cpu->cycles += 1;

	return arg_addr;
}

static enum argtype modeZeropage(struct simak65_cpu *cpu, u8 *args)
{
	args[0] = addrmode_nextpc(cpu);
	args[1] = 0;

	DEBUG("Zero Page mode, args: 0x%02x%02x", args[1], args[0]);

	cpu->cycles += 2;

	return arg_addr;
}

static enum argtype modeZeropageX(struct simak65_cpu *cpu, u8 *args)
{
	args[0] = addrmode_nextpc(cpu) + cpu->reg.x;
	args[1] = 0;

	DEBUG("Zero Page, X mode, args: 0x%02x%02x", args[1], args[0]);

	cpu->cycles += 2;

	return arg_addr;
}

static enum argtype modeZeropageY(struct simak65_cpu *cpu, u8 *args)
{
	args[0] = addrmode_nextpc(cpu) + cpu->reg.y;
	args[1] = 0;

	DEBUG("Zero Page, Y mode, args: 0x%02x%02x", args[1], args[0]);

	cpu->cycles += 2;

	return arg_addr;
}

u8 addrmode_nextpc(struct simak65_cpu *cpu)
{
	u8 data;

	data = cpu->bus.read(cpu->reg.pc);

	DEBUG("Read 0x%02x from pc: 0x%04x", data, cpu->reg.pc);

	++cpu->reg.pc;

	if (cpu->reg.pc == 0)
		WARN("Program counter wrap-around");

	return data;
}

enum argtype addrmode_getArgs(struct simak65_cpu *cpu, u8 *args, enum addrmode mode)
{
	enum argtype arg_type;

	switch(mode) {
		case mode_acc:
			arg_type = modeAcc(cpu, args);
			break;

		case mode_abs:
			arg_type = modeAbsolute(cpu, args);
			break;

		case mode_abx:
			arg_type = modeAbsoluteX(cpu, args);
			break;

		case mode_aby:
			arg_type = modeAbsoluteY(cpu, args);
			break;

		case mode_imm:
			arg_type = modeImmediate(cpu, args);
			break;

		case mode_imp:
			arg_type = modeImplicant(cpu, args);
			break;

		case mode_ind:
			arg_type = modeIndirect(cpu, args);
			break;

		case mode_inx:
			arg_type = modeIndirectX(cpu, args);
			break;

		case mode_iny:
			arg_type = modeIndirectY(cpu, args);
			break;

		case mode_rel:
			arg_type = modeRelative(cpu, args);
			break;

		case mode_zp:
			arg_type = modeZeropage(cpu, args);
			break;

		case mode_zpx:
			arg_type = modeZeropageX(cpu, args);
			break;

		case mode_zpy:
			arg_type = modeZeropageY(cpu, args);
			break;

		default:
			FATAL("Invalid addressing mode");
	}

	return arg_type;
}

