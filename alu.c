#include "error.h"
#include "alu.h"
#include "flags.h"

void alu_flags(u8 result, u8 *flags, u8 mask)
{
	if (mask & FLAG_ZERO) {
		if ((result & 0xff) == 0)
			*flags |= FLAG_ZERO;
		else
			*flags &= ~FLAG_ZERO;
	}

	if (mask & FLAG_SIGN) {
		if (result & 0x80)
			*flags |= FLAG_SIGN;
		else
			*flags &= ~FLAG_SIGN;
	}
}

u8 alu_add(u8 a, u8 b, u8 *flags)
{
	u16 result, ai, bi;
	u8 carry_in = !!(*flags & FLAG_CARRY);

	ai = (u16)a;
	bi = (u16)b;

	result = ai + bi + carry_in;

	if (*flags & FLAG_BCD) {
		if ((ai & 0xf) + (bi & 0xf) + carry_in > 9)
			result += 0x06;

		if ((result >> 4) > 9)
			result += 0x60;
	}

	if (result < ai || result < bi || (result & 0xff00) != 0)
		*flags |= FLAG_CARRY;
	else
		*flags &= ~FLAG_CARRY;

	alu_flags(result & 0xff, flags, FLAG_SIGN | FLAG_ZERO);

	if ((ai ^ result) & (bi ^ result) & 0x80)
		*flags |= FLAG_OVRF;
	else
		*flags &= ~FLAG_OVRF;

	DEBUG("0x%02x + 0x%02x + %d = 0x%02x, flags 0x%02x", ai & 0xff, bi & 0xff, carry_in, result & 0xff, *flags);

	return result & 0xff;
}

u8 alu_sub(u8 a, u8 b, u8 *flags)
{
	u16 result, ai, bi;
	u8 carry_in = !!(*flags & FLAG_CARRY);

	ai = (u16)a;

	if (*flags & FLAG_BCD)
		bi = (u16)((0x99 - b) & 0xff);
	else
		bi = (u16)(~b & 0xff);

	result = ai + bi + carry_in;

	if (*flags & FLAG_BCD) {
		if ((ai & 0xf) + (bi & 0xf) + carry_in > 9)
			result += 0x06;

		if ((result >> 4) > 9)
			result += 0x60;
	}

	if (result < ai || result < bi || (result & 0xff00) != 0)
		*flags |= FLAG_CARRY;
	else
		*flags &= ~FLAG_CARRY;

	alu_flags(result & 0xff, flags, FLAG_SIGN | FLAG_ZERO);

	if ((ai ^ result) & (bi ^ result) & 0x80)
		*flags |= FLAG_OVRF;
	else
		*flags &= ~FLAG_OVRF;

	DEBUG("0x%02x - 0x%02x + %d = 0x%02x, flags 0x%02x", ai & 0xff, bi & 0xff, carry_in, result & 0xff, *flags);

	return result & 0xff;
}

u8 alu_inc(u8 a, u8 b, u8 *flags)
{
	u8 result;

	result = a + 1;

	alu_flags(result, flags, FLAG_SIGN | FLAG_ZERO);

	DEBUG("0x%02x + 1 = 0x%02x, flags 0x%02x", a & 0xff, result & 0xff, *flags);

	return result;
}

u8 alu_dec(u8 a, u8 b, u8 *flags)
{
	u8 result;

	result = a - 1;

	alu_flags(result, flags, FLAG_SIGN | FLAG_ZERO);

	DEBUG("0x%02x - 1 = 0x%02x, flags 0x%02x", a & 0xff, result & 0xff, *flags);

	return result;
}

u8 alu_and(u8 a, u8 b, u8 *flags)
{
	u8 result;

	result = a & b;

	alu_flags(result, flags, FLAG_SIGN | FLAG_ZERO);

	DEBUG("0x%02x & 0x%02x = 0x%02x, flags 0x%02x", a & 0xff, b & 0xff, result & 0xff, *flags);

	return result;
}

u8 alu_or(u8 a, u8 b, u8 *flags)
{
	u8 result;

	result = a | b;

	alu_flags(result, flags, FLAG_SIGN | FLAG_ZERO);

	DEBUG("0x%02x | 0x%02x = 0x%02x, flags 0x%02x", a & 0xff, b & 0xff, result & 0xff, *flags);

	return result;
}

u8 alu_eor(u8 a, u8 b, u8 *flags)
{
	u8 result;

	result = a ^ b;

	alu_flags(result, flags, FLAG_SIGN | FLAG_ZERO);

	DEBUG("0x%02x ^ 0x%02x = 0x%02x, flags 0x%02x", a & 0xff, b & 0xff, result & 0xff, *flags);

	return result;
}

u8 alu_rol(u8 a, u8 b, u8 *flags)
{
	u8 result;

	result = a << 1;

	if (*flags & FLAG_CARRY)
		result |= 1;

	alu_flags(result, flags, FLAG_SIGN | FLAG_ZERO);

	if (a & 0x80)
		*flags |= FLAG_CARRY;
	else
		*flags &= ~FLAG_CARRY;

	DEBUG("c << 0x%02x << c = 0x%02x, flags 0x%02x", a & 0xff, result & 0xff, *flags);

	return result;
}

u8 alu_ror(u8 a, u8 b, u8 *flags)
{
	u8 result;

	result = a >> 1;

	if (*flags & FLAG_CARRY)
		result |= 0x80;

	alu_flags(result, flags, FLAG_SIGN | FLAG_ZERO);

	if (a & 0x01)
		*flags |= FLAG_CARRY;
	else
		*flags &= ~FLAG_CARRY;

	DEBUG("c >> 0x%02x >> c = 0x%02x, flags 0x%02x", a & 0xff, result & 0xff, *flags);

	return result;
}

u8 alu_asl(u8 a, u8 b, u8 *flags)
{
	u8 result;

	result = a << 1;

	alu_flags(result, flags, FLAG_SIGN | FLAG_ZERO);

	if (a & 0x80)
		*flags |= FLAG_CARRY;
	else
		*flags &= ~FLAG_CARRY;

	DEBUG("c << 0x%02x << 0 = 0x%02x, flags 0x%02x", a & 0xff, result & 0xff, *flags);

	return result;
}

u8 alu_lsr(u8 a, u8 b, u8 *flags)
{
	u8 result;

	result = a >> 1;

	alu_flags(result, flags, FLAG_SIGN | FLAG_ZERO);

	if (a & 0x01)
		*flags |= FLAG_CARRY;
	else
		*flags &= ~FLAG_CARRY;

	DEBUG("0 >> 0x%02x >> c = 0x%02x, flags 0x%02x", a & 0xff, result & 0xff, *flags);

	return result;
}

u8 alu_bit(u8 a, u8 b, u8 *flags)
{
	u8 result;

	result = a & b;

	alu_flags(result, flags, FLAG_ZERO);

	if (b & 0x40)
		*flags |= FLAG_OVRF;
	else
		*flags &= ~FLAG_OVRF;

	if (b & 0x80)
		*flags |= FLAG_SIGN;
	else
		*flags &= ~FLAG_SIGN;

	DEBUG("0x%02x & 0x%02x = 0x%02x, flags 0x%02x", a & 0xff, b & 0xff, result & 0xff, *flags);

	return result;
}

u8 alu_cmp(u8 a, u8 b, u8 *flags)
{
	u16 result, ai, bi;

	ai = (u16)a;
	bi = (u16)(~b & 0xff);

	result = ai + bi + 1;

	if (result < ai || result < bi || (result & 0xff00) != 0)
		*flags |= FLAG_CARRY;
	else
		*flags &= ~FLAG_CARRY;

	alu_flags(result & 0xff, flags, FLAG_SIGN | FLAG_ZERO);

	DEBUG("0x%02x - 0x%02x = 0x%02x, flags 0x%02x", ai & 0xff, bi & 0xff, result & 0xff, *flags);
	DEBUG("0x%04x - 0x%04x = 0x%04x, flags 0x%02x", ai, bi, result, *flags);

	return result;
}

u8 alu_load(u8 a, u8 b, u8 *flags)
{
	alu_flags(a, flags, FLAG_ZERO | FLAG_SIGN);

	DEBUG("0x%02x, flags 0x%02x", a & 0xff, *flags);

	return a;
}
