#ifndef SIMAK65_ADDRMODE_H_
#define SIMAK65_ADDRMODE_H_

#include "types.h"
#include "decoder.h"

enum argtype { arg_none, arg_byte, arg_addr };

u8 addrmode_nextpc(struct simak65_cpustate *cpu);

enum argtype addrmode_getArgs(struct simak65_cpustate *cpu, u8 *args, enum addrmode mode, unsigned int *cycles);

#endif /* SIMAK65_ADDRMODE_H_ */
