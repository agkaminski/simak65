/* SimAK65 addresing mode
 * Copyright A.K. 2018, 2023
 */

#include "error.h"
#include "bus.h"
#include "addrmode.h"
#include "decoder.h"
#include "simak65.h"


static enum argtype modeAcc(struct simak65_cpustate *cpu, u8 *args, unsigned int *cycles)
{
	(void)cycles;

	args[0] = cpu->a;

	DEBUG("Accumulator mode, args : 0x%02x", args[0]);

	return arg_byte;
}

static enum argtype modeAbsolute(struct simak65_cpustate *cpu, u8 *args, unsigned int *cycles)
{
	args[0] = addrmode_nextpc(cpu);
	args[1] = addrmode_nextpc(cpu);

	DEBUG("Absolute mode, args : 0x%02x%02x", args[1], args[0]);

	*cycles += 3;

	return arg_addr;
}

static enum argtype modeAbsoluteX(struct simak65_cpustate *cpu, u8 *args, unsigned int *cycles)
{
	u16 addr;

	addr = addrmode_nextpc(cpu);
	addr |= (u16)addrmode_nextpc(cpu) << 8;
	addr += cpu->x;

	args[0] = addr & 0xff;
	args[1] = (addr >> 8) & 0xff;

	DEBUG("Absolute, X mode, args : 0x%02x%02x", args[1], args[0]);

	*cycles += 3;

	return arg_addr;
}

static enum argtype modeAbsoluteY(struct simak65_cpustate *cpu, u8 *args, unsigned int *cycles)
{
	u16 addr;

	addr = addrmode_nextpc(cpu);
	addr |= (u16)addrmode_nextpc(cpu) << 8;
	addr += cpu->y;

	args[0] = addr & 0xff;
	args[1] = (addr >> 8) & 0xff;

	DEBUG("Absolute, Y mode, args : 0x%02x%02x", args[1], args[0]);

	*cycles += 3;

	return arg_addr;
}

static enum argtype modeImmediate(struct simak65_cpustate *cpu, u8 *args, unsigned int *cycles)
{
	args[0] = addrmode_nextpc(cpu);

	DEBUG("Immediate mode, args: 0x%02x", args[0]);

	*cycles += 1;

	return arg_byte;
}

static enum argtype modeImplicant(struct simak65_cpustate *cpu, u8 *args, unsigned int *cycles)
{
	(void)cpu;
	(void)args;
	(void)cycles;

	DEBUG("Implicant mode, no args");

	return arg_none;
}

static enum argtype modeIndirect(struct simak65_cpustate *cpu, u8 *args, unsigned int *cycles)
{
	u16 addr;

	addr = addrmode_nextpc(cpu);
	addr |= (u16)addrmode_nextpc(cpu) << 8;

	args[0] = bus.read(addr++);
	args[1] = bus.read(addr);

	DEBUG("Indirect mode, args: 0x%02x%02x from addr: 0x%04x", args[1], args[0], addr);

	*cycles += 7;

	return arg_addr;
}

static enum argtype modeIndirectX(struct simak65_cpustate *cpu, u8 *args, unsigned int *cycles)
{
	u16 addr;

	addr = addrmode_nextpc(cpu);
	addr += cpu->x;
	addr &= 0xff;

	args[0] = bus.read(addr++);
	args[1] = bus.read(addr);

	DEBUG("Indexed indirect mode, args: 0x%02x%02x from addr: 0x%04x", args[1], args[0], addr);

	*cycles += 5;

	return arg_addr;
}

static enum argtype modeIndirectY(struct simak65_cpustate *cpu, u8 *args, unsigned int *cycles)
{
	u16 zpAddr;
	u16 addr;

	zpAddr = addrmode_nextpc(cpu);

	addr = bus.read(zpAddr++);
	addr |= (u16)bus.read(zpAddr) << 8;

	addr += cpu->y;

	args[0] = addr & 0xff;
	args[1] = (addr >> 8) & 0xff;

	DEBUG("Indirect indexed mode, args: 0x%02x%02x from addr: 0x%04x", args[1], args[0], zpAddr);

	*cycles += 5;

	return arg_addr;
}

static enum argtype modeRelative(struct simak65_cpustate *cpu, u8 *args, unsigned int *cycles)
{
	s8 rel;
	u16 addr;

	rel = addrmode_nextpc(cpu);
	addr = cpu->pc;
	addr += rel;

	args[0] = addr & 0xff;
	args[1] = (addr >> 8) & 0xff;

	DEBUG("Relative mode, args: 0x%02x%02x = pc + rel: 0x%02x", args[1], args[0], rel);

	*cycles += 1;

	return arg_addr;
}

static enum argtype modeZeropage(struct simak65_cpustate *cpu, u8 *args, unsigned int *cycles)
{
	args[0] = addrmode_nextpc(cpu);
	args[1] = 0;

	DEBUG("Zero Page mode, args: 0x%02x%02x", args[1], args[0]);

	*cycles += 2;

	return arg_addr;
}

static enum argtype modeZeropageX(struct simak65_cpustate *cpu, u8 *args, unsigned int *cycles)
{
	args[0] = addrmode_nextpc(cpu) + cpu->x;
	args[1] = 0;

	DEBUG("Zero Page, X mode, args: 0x%02x%02x", args[1], args[0]);

	*cycles += 2;

	return arg_addr;
}

static enum argtype modeZeropageY(struct simak65_cpustate *cpu, u8 *args, unsigned int *cycles)
{
	args[0] = addrmode_nextpc(cpu) + cpu->y;
	args[1] = 0;

	DEBUG("Zero Page, Y mode, args: 0x%02x%02x", args[1], args[0]);

	*cycles += 2;

	return arg_addr;
}

u8 addrmode_nextpc(struct simak65_cpustate *cpu)
{
	u8 data;

	data = bus.read(cpu->pc);

	DEBUG("Read 0x%02x from pc: 0x%04x", data, cpu->pc);

	++cpu->pc;

	if (cpu->pc == 0)
		WARN("Program counter wrap-around");

	return data;
}

enum argtype addrmode_getArgs(struct simak65_cpustate *cpu, u8 *args, enum addrmode mode, unsigned int *cycles)
{
	enum argtype arg_type;

	switch(mode) {
		case mode_acc:
			arg_type = modeAcc(cpu, args, cycles);
			break;

		case mode_abs:
			arg_type = modeAbsolute(cpu, args, cycles);
			break;

		case mode_abx:
			arg_type = modeAbsoluteX(cpu, args, cycles);
			break;

		case mode_aby:
			arg_type = modeAbsoluteY(cpu, args, cycles);
			break;

		case mode_imm:
			arg_type = modeImmediate(cpu, args, cycles);
			break;

		case mode_imp:
			arg_type = modeImplicant(cpu, args, cycles);
			break;

		case mode_ind:
			arg_type = modeIndirect(cpu, args, cycles);
			break;

		case mode_inx:
			arg_type = modeIndirectX(cpu, args, cycles);
			break;

		case mode_iny:
			arg_type = modeIndirectY(cpu, args, cycles);
			break;

		case mode_rel:
			arg_type = modeRelative(cpu, args, cycles);
			break;

		case mode_zp:
			arg_type = modeZeropage(cpu, args, cycles);
			break;

		case mode_zpx:
			arg_type = modeZeropageX(cpu, args, cycles);
			break;

		case mode_zpy:
			arg_type = modeZeropageY(cpu, args, cycles);
			break;

		default:
			FATAL("Invalid addressing mode");
	}

	return arg_type;
}











