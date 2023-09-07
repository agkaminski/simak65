#ifndef SIMAK65_H_
#define SIMAK65_H_

#include <stdint.h>

struct simak65_bus {
	uint8_t (*read)(uint16_t address);
	void (*write)(uint16_t address, uint8_t byte);
}

struct simak65_cpustate {
	u16 pc;
	u8 a;
	u8 x;
	u8 y;
	u8 sp;
	u8 flags;
}

/* Execute next instruction */
void simak65_step(struct simak65_cpustate *cpu, unsigned int *cycles);

/* Execute reset */
void simak65_rst(struct simak65_cpustate *cpu, unsigned int *cycles);

/* Execute non-maskable interrupt */
void simak65_nmi(struct simak65_cpustate *cpu, unsigned int *cycles);

/* Execute interrupt */
void simak65_irq(struct simak65_cpustate *cpu, unsigned int *cycles);

/* Perform core initialization (excluding CPU reset) */
void simak65_init(struct simak65_cpustate *cpu, const struct simak65_bus *bus);

#endif /* SIMAK65_H_ */
