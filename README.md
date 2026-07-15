# neodymium

A weird hobby x86 (for now) operating system kernel written in C99.

neodymium is a from-scratch monolithic kernel targeting 32-bit x86 (only, for now!). It boots via Multiboot2 (BIOS with GRUB or UEFI) and includes a preemptive (round-robin) scheduler, VMM, ELF loading, a basic syscall interface, and graphical output through a framebuffer console (`fbcon`).

## Building

### Prerequisites

- GCC cross-compiler targeting i686-elf (or a 32-bit GCC with `-ffreestanding`)
- GNU binutils (`ld` for i386)
- GRUB (`grub-mkrescue`) and `xorriso` for ISO creation
- QEMU for testing

### Commands

```sh
make            # build the kernel binary
make iso        # build a bootable ISO image
make run        # run with QEMU (kernel only, serial output)
make run-iso    # boot from ISO in QEMU
make run-uefi   # boot from ISO in QEMU with UEFI (OVMF)
make clean      # remove build artifacts
```

The kernel binary is output to `build/neodymium.bin` and the ISO to `build/neodymium.iso`.

## Running

`make run` launches QEMU with the kernel directly (`-kernel`), outputting serial to stdout. `make run-iso` boots the GRUB-based ISO. `make run-uefi` does the same with UEFI firmware.

On boot, the kernel initializes all subsystems and loads the embedded `hello` ELF program as a userspace task. A framebuffer console is drawn if a framebuffer was negotiated at boot. The PS/2 keyboard driver is active; you can type and see output echoed to both serial and the framebuffer console.

Keyboard shortcuts:
- **Ctrl+Shift+Alt+F12**: trigger a kernel panic (for testing)
- **Ctrl+Shift+F12**: toggle framebuffer console visibility

## Syscall Convention

System calls are issued via `int $0x80`. Registers follow this convention:

| Register | Purpose         |
|----------|-----------------|
| `eax`    | Syscall number  |
| `ebx`    | Argument 1      |
| `ecx`    | Argument 2      |
| `edx`    | Argument 3      |
| `esi`    | Argument 4      |
| `edi`    | Argument 5      |

| Number | Name      | Description                      |
|--------|-----------|----------------------------------|
| 0      | `exit`    | Exit with code                   |
| 1      | `write`   | Write a single character         |
| 2      | `writestr`| Write a null-terminated string   |
| 3      | `gettick` | Get current timer tick           |
| 4      | `read`    | Blocking keyboard read           |
| 5      | `fork`    | Fork current process             |
| 6      | `getpid`  | Get current PID                  |
| 7      | `wait`    | Wait for a child to exit         |
| 8      | `sleep`   | Sleep for N ticks                |
| 9      | `exec`    | Execute an ELF image in-place    |

## License

neodymium is licensed under the [GPL-2.0](LICENSE) license.
