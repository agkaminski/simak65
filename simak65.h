/* SimAK65 interface
 * Copyright A.K. 2018, 2023
 */

#ifndef SIMAK65_H_
#define SIMAK65_H_

#include <stdint.h>

struct simak65_cpu {
	struct {
		uint16_t pc;
		uint8_t a;
		uint8_t x;
		uint8_t y;
		uint8_t sp;
		uint8_t flags;
	} reg;

	/* This struct has to be populated by the user */
	struct {
		uint8_t (*read)(uint16_t address);
		void (*write)(uint16_t address, uint8_t byte);
	} bus;
	unsigned long cycles;
};

/* Execute next instruction */
void simak65_step(struct simak65_cpu *cpu);

/* Execute reset */
void simak65_rst(struct simak65_cpu *cpu);

/* Execute non-maskable interrupt */
void simak65_nmi(struct simak65_cpu *cpu);

/* Execute interrupt */
void simak65_irq(struct simak65_cpu *cpu);

/* Perform core initialization (excluding CPU reset) */
void simak65_init(struct simak65_cpu *cpu);

#endif /* SIMAK65_H_ */
