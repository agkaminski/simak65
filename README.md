# simak65 - 6502 CPU simulator library

This simulator is based on the simualator of [AK6502 CPU](https://github.com/agkaminski/AK6502).
NMOS version of the CPU is simulated, cycle accurate. There's no undocumented intructions support,
invalid opcodes are decoded as `nop`.

## Build

There're no dependencies, only `make` and `gcc` are needed. Simply type `make` to build the library.
It can be installed to `/usr/local/lib` via `sudo make install`, along with the api header (to `/usr/local/include`).

## API

Library interface is available in `simak65.h` header.

### struct simak65_bus

This structure holds CPU bus read/write callback, that are need to provided by the library user.

### struct simak65_cpustate

This structure holds internal CPU state. It is stored by the library user and needs to be passed
to the library functions.

### void simak65_step(struct simak65_cpustate *cpu, unsigned int *cycles)

Execute the next instruction.
- cpu - cpu state,
- cycles - optional (can be NULL), number of cycles passed for the execution of the instruction
is added to this variable.

### void simak65_rst(struct simak65_cpustate *cpu, unsigned int *cycles)

Perform the CPU reset. This operation must be performed pefored before executing first instruction.
- cpu - cpu state,
- cycles - optional (can be NULL), number of cycles passed for the execution of the instruction
is added to this variable.

### void simak65_nmi(struct simak65_cpustate *cpu, unsigned int *cycles)

Execute the non-maskable interrupt.
- cpu - cpu state,
- cycles - optional (can be NULL), number of cycles passed for the execution of the instruction
is added to this variable.

### void simak65_irq(struct simak65_cpustate *cpu, unsigned int *cycles)

Execute the interrupt.
- cpu - cpu state,
- cycles - optional (can be NULL), number of cycles passed for the execution of the instruction
is added to this variable.

### void simak65_init(struct simak65_cpustate *cpu, const struct simak65_bus *ops)

Initialize the library and the cpu state.
- cpu - cpu state,
- ops - CPU memory bus callbacks.

## License

See LICENSE for details.
