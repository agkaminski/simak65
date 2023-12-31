/* SimAK65 arithmetic-logic unit
 * Copyright A.K. 2018, 2023
 */

#ifndef SIMAK65_ALU_H_
#define SIMAK65_ALU_H_

#include "types.h"

void alu_flags(u8 result, u8 *flags, u8 mask);

u8 alu_add(u8 a, u8 b, u8 *flags);

u8 alu_sub(u8 a, u8 b, u8 *flags);

u8 alu_inc(u8 a, u8 b, u8 *flags);

u8 alu_dec(u8 a, u8 b, u8 *flags);

u8 alu_and(u8 a, u8 b, u8 *flags);

u8 alu_or(u8 a, u8 b, u8 *flags);

u8 alu_eor(u8 a, u8 b, u8 *flags);

u8 alu_rol(u8 a, u8 b, u8 *flags);

u8 alu_ror(u8 a, u8 b, u8 *flags);

u8 alu_asl(u8 a, u8 b, u8 *flags);

u8 alu_lsr(u8 a, u8 b, u8 *flags);

u8 alu_bit(u8 a, u8 b, u8 *flags);

u8 alu_cmp(u8 a, u8 b, u8 *flags);

u8 alu_load(u8 a, u8 b, u8 *flags);

#endif /* SIMAK65_ALU_H_ */
