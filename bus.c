#include "simak65.h"
#include "bus.h"

struct simak65_bus bus;

void bus_init(struct simak65_bus *ops)
{
	bus = *ops;
}
