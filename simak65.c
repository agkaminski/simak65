/* SimAK65 interface
 * Copyright A.K. 2018, 2023
 */

#include <stddef.h>
#include "simak65.h"
#include "decoder.h"
#include "addrmode.h"
#include "types.h"
#include "exec.h"

void simak65_step(struct simak65_cpu *cpu)
{
	u8 args[2];

	struct opinfo instruction = decode(addrmode_nextpc(cpu));
	enum argtype argtype = addrmode_getArgs(cpu, args, instruction.mode);
	exec_execute(cpu, instruction.opcode, argtype, args);
}

void simak65_rst(struct simak65_cpu *cpu)
{
	exec_rst(cpu);
}

void simak65_nmi(struct simak65_cpu *cpu)
{
	exec_nmi(cpu);
}

void simak65_irq(struct simak65_cpu *cpu)
{
	exec_irq(cpu);
}

void simak65_init(struct simak65_cpu *cpu)
{
	cpu->reg.pc = 0;
	cpu->reg.a = 0;
	cpu->reg.x = 0;
	cpu->reg.y = 0;
	cpu->reg.sp = 0;
	cpu->reg.flags = 0;
	cpu->cycles = 0;
}
