#ifndef SIMAK65_CORE_H_
#define SIMAK65_CORE_H_

#include "types.h"

void core_nmi(void);

void core_irq(void);

void core_rst(void);

void core_step(void);

void core_run(void);

void core_stop(void);

void core_getState(struct simak65_cpustate *cpu, unsigned int *cycles);

void core_setState(struct simak65_cpustate *cpu);

void core_setSpeed(unsigned int speed);

void core_init(void);

#endif /* SIMAK65_CORE_H_ */
