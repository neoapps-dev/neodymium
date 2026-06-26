CC      = gcc
CFLAGS  = -std=c99 -m32 -ffreestanding -O2 -Wall -Wextra -Werror -nostdlib -fno-pie -fno-stack-protector -mno-sse -mno-sse2 -mno-mmx -mno-80387 -DMAKE_COMMIT_HASH=\"$(shell git describe --always --dirty)\"
LD      = gcc
LDFLAGS = -m32 -ffreestanding -O2 -nostdlib -nostartfiles -static -fno-pie -fno-stack-protector -lgcc -Wl,-e,_start
AS      = gcc
BUILD = build
SRC = src
KERNEL = neodymium.bin
ISO = neodymium.iso
SRCS_C := $(shell find src -path src/programs -prune -o -name '*.c' -print)
SRCS_S := $(shell find src -path src/programs -prune -o -name '*.S' -print)
OBJS   := $(patsubst src/%.c,$(BUILD)/%.o,$(SRCS_C)) $(patsubst src/%.S,$(BUILD)/%.o,$(SRCS_S))
OBJS += $(BUILD)/kernel/hello_elf.o
all: $(BUILD)/$(KERNEL)
.PHONY: all clean iso run
$(BUILD)/$(KERNEL): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -T linker.ld -o $@ $(OBJS)

LD2 = ld
$(BUILD)/programs/%.o: src/programs/%.S
	@mkdir -p $(@D)
	$(AS) -m32 -c -o $@ $<
$(BUILD)/programs/%: $(BUILD)/programs/%.o
	$(LD2) -m elf_i386 -nostdlib -static -Ttext 0x08048000 -o $@ $<
$(BUILD)/kernel/hello.elf: $(BUILD)/programs/hello
	@mkdir -p $(@D)
	cp $< $@
$(BUILD)/kernel/hello_elf.o: $(BUILD)/kernel/hello.elf
	$(LD2) -r -b binary -m elf_i386 -o $@ $<

$(BUILD)/boot/%.o: $(SRC)/boot/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/drivers/%.o: $(SRC)/drivers/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/kernel/%.o: $(SRC)/kernel/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/kernel/%.o: $(SRC)/kernel/%.S
	@mkdir -p $(@D)
	$(CC) -m32 -c -o $@ $<

iso: $(BUILD)/$(KERNEL)
	mkdir -p $(BUILD)/iso/boot/grub
	cp $(BUILD)/$(KERNEL) $(BUILD)/iso/boot/
	echo 'set timeout=10'                      > $(BUILD)/iso/boot/grub/grub.cfg
	echo 'set default=0'                     >> $(BUILD)/iso/boot/grub/grub.cfg
	echo 'insmod vbe'                        >> $(BUILD)/iso/boot/grub/grub.cfg
	echo 'insmod vga'                        >> $(BUILD)/iso/boot/grub/grub.cfg
	echo 'menuentry "neodymium" {'           >> $(BUILD)/iso/boot/grub/grub.cfg
	echo '  multiboot /boot/neodymium.bin'   >> $(BUILD)/iso/boot/grub/grub.cfg
	echo '}'                                 >> $(BUILD)/iso/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD)/$(ISO) $(BUILD)/iso

run: $(BUILD)/$(KERNEL)
	qemu-system-x86_64 -vga std -kernel $(BUILD)/$(KERNEL) -serial stdio

run-iso: iso
	qemu-system-x86_64 -vga std -cdrom $(BUILD)/$(ISO) -serial stdio

clean:
	rm -rf $(BUILD)
