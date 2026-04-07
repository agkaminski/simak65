# simak65 - 6502 CPU simulator library

This simulator is based on the simualator of [AK6502 CPU](https://github.com/agkaminski/AK6502).
NMOS version of the CPU is simulated, cycle accurate. There's no undocumented intructions support,
invalid opcodes are decoded as `nop`.

## Build

There are no dependencies, only `make` and `gcc` are needed. Simply type `make` to build the library.
It can be installed to `/usr/local/lib` via `sudo make install`, along with the api header (to `/usr/local/include`).

## API

Library interface is available in `simak65.h` header.

### struct simak65_cpu

This structure holds the whole CPU state and callbacks to the bus operations. Fields `bus.read()` and
`bus.write()` have to be populated by the user with bus access callbacks.

### void simak65_step(struct simak65_cpu *cpu)

Execute the next instruction.

### void simak65_rst(struct simak65_cpu)

Perform the CPU reset.

### void simak65_nmi(struct simak65_cpu)

Execute the non-maskable interrupt.

### void simak65_irq(struct simak65_cpu)

Execute the interrupt.

### void simak65_init(struct simak65_cpu *cpu)

Initialize the library and the cpu state. The `bus` substructure of the CPU state has to be populated
by the user.

## License

See LICENSE for details.
