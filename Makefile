CC      = gcc
CFLAGS  = -std=c99 -m32 -ffreestanding -O2 -Wall -Wextra -Werror -nostdlib -fno-pie -fno-stack-protector -mno-sse -mno-sse2 -mno-mmx -mno-80387
LD      = gcc
LDFLAGS = -m32 -ffreestanding -O2 -nostdlib -nostartfiles -static -fno-pie -fno-stack-protector -lgcc -Wl,-e,_start
AS      = gcc
BUILD = build
SRC = src
KERNEL = neodymium.bin
ISO = neodymium.iso
OBJS = $(BUILD)/boot/boot.o $(BUILD)/drivers/vga.o $(BUILD)/kernel/kernel.o
.PHONY: all clean iso run
all: $(BUILD)/$(KERNEL)
$(BUILD)/$(KERNEL): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -T linker.ld -o $@ $(OBJS)

$(BUILD)/boot/%.o: $(SRC)/boot/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/drivers/%.o: $(SRC)/drivers/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/kernel/%.o: $(SRC)/kernel/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

iso: $(BUILD)/$(KERNEL)
	mkdir -p $(BUILD)/iso/boot/grub
	cp $(BUILD)/$(KERNEL) $(BUILD)/iso/boot/
	echo 'set timeout=0'                      > $(BUILD)/iso/boot/grub/grub.cfg
	echo 'set default=0'                     >> $(BUILD)/iso/boot/grub/grub.cfg
	echo 'menuentry "neodymium" {'           >> $(BUILD)/iso/boot/grub/grub.cfg
	echo '  multiboot /boot/neodymium.bin'   >> $(BUILD)/iso/boot/grub/grub.cfg
	echo '}'                                 >> $(BUILD)/iso/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD)/$(ISO) $(BUILD)/iso

run: $(BUILD)/$(KERNEL)
	qemu-system-x86_64 -kernel $(BUILD)/$(KERNEL)

run-iso: iso
	qemu-system-x86_64 -cdrom $(BUILD)/$(ISO)

clean:
	rm -rf $(BUILD)
