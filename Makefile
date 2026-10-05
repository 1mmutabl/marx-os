ASM     := nasm
CC      := gcc
LD      := ld
OBJCOPY := objcopy

QEMU      := qemu-system-i386
RAM       ?= 8G
SMP       ?= 4
CPU       ?= host
QEMUFLAGS := -enable-kvm \
	     -cpu $(CPU) \
	     -m $(RAM) \
	     -smp $(SMP)

MKFS   := mkfs.fat
MTOOLS := mcopy
MMD    := mmd

BUILD_DIR := build
BIN_DIR   := bin

BOOT_SRC := src/boot/boot.asm
BOOT_BIN := $(BIN_DIR)/boot.bin

STAGE2_SRC := src/boot/stage2.asm
STAGE2_BIN := $(BIN_DIR)/stage2.bin

KERNEL_SRC_DIR := src/kernel

KERNEL_ASM := $(KERNEL_SRC_DIR)/entry.asm
KERNEL_ASM_OBJ := $(BUILD_DIR)/entry.o

KERNEL_C_SRCS := $(shell find $(KERNEL_SRC_DIR) -type f -name '*.c')
KERNEL_C_OBJS := $(patsubst $(KERNEL_SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(KERNEL_C_SRCS))

KERNEL_OBJS := \
	$(KERNEL_ASM_OBJ) \
	$(KERNEL_C_OBJS)

KERNEL_ELF := $(BUILD_DIR)/kernel.elf
KERNEL_BIN := $(BIN_DIR)/kernel.bin

LINKER_SCRIPT := linker.ld

DISK_IMG := $(BUILD_DIR)/disk.img
SYSTEM_DIR := system

SECTOR_SIZE := 512

DISK_SECTORS := 131072

FAT32_START_SECTOR := 2048

STAGE2_MAX_SECTORS := 8

FAT32_OFFSET := $(shell echo $$(( $(FAT32_START_SECTOR) * $(SECTOR_SIZE) )))

MTOOLS_IMAGE := $(DISK_IMG)@@$(FAT32_OFFSET)

CFLAGS := \
	-m32 \
	-O2 \
	-march=i686 \
	-msse2 \
	-mfpmath=sse \
	-ffreestanding \
	-fno-pic \
	-fno-stack-protector \
	-fno-asynchronous-unwind-tables \
	-nostdlib \
	-nostartfiles \
	-nodefaultlibs \
	-Isrc/kernel

LDFLAGS := \
	-m elf_i386 \
	-T $(LINKER_SCRIPT)

.PHONY: all
all: $(DISK_IMG)

$(KERNEL_ASM_OBJ): $(KERNEL_ASM)
	@mkdir -p $(dir $@)
	@$(ASM) $< -f elf32 -o $@

$(BUILD_DIR)/%.o: $(KERNEL_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(KERNEL_OBJS) $(LINKER_SCRIPT)
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

$(KERNEL_BIN): $(KERNEL_ELF)
	@mkdir -p $(dir $@)
	@$(OBJCOPY) -O binary $< $@

$(STAGE2_BIN): $(STAGE2_SRC) $(KERNEL_BIN)
	@mkdir -p $(dir $@)

	@KERNEL_SIZE=$$(stat -c%s "$(KERNEL_BIN)"); \
	KERNEL_SECTORS=$$(( (KERNEL_SIZE + $(SECTOR_SIZE) - 1) / $(SECTOR_SIZE) )); \
	echo "Kernel: $$KERNEL_SIZE bytes ($$KERNEL_SECTORS sectors)"; \
	\
	$(ASM) "$(STAGE2_SRC)" -f bin \
		-dKERNEL_SECTORS=$$KERNEL_SECTORS \
		-dKERNEL_START_SECTOR=2 \
		-o "$@.sizing"; \
	STAGE2_SIZE=$$(stat -c%s "$@.sizing"); \
	STAGE2_SECTORS=$$(( (STAGE2_SIZE + $(SECTOR_SIZE) - 1) / $(SECTOR_SIZE) )); \
	rm -f "$@.sizing"; \
	\
	if [ "$$STAGE2_SECTORS" -gt $(STAGE2_MAX_SECTORS) ]; then \
		echo "ERROR: Stage 2 is too large to load safely"; \
		echo "       Size: $$STAGE2_SIZE bytes ($$STAGE2_SECTORS sectors, max $(STAGE2_MAX_SECTORS))"; \
		echo "       boot.asm reads it with a single-track CHS read (cylinder 0,"; \
		echo "       head 0) - keep this under your BIOS's sectors-per-track, or"; \
		echo "       switch that read to LBA/extensions like the kernel load already is."; \
		exit 1; \
	fi; \
	\
	KERNEL_START_SECTOR=$$(( 1 + STAGE2_SECTORS )); \
	echo "Stage 2: $$STAGE2_SIZE bytes ($$STAGE2_SECTORS sectors); kernel starts at LBA $$KERNEL_START_SECTOR"; \
	\
	$(ASM) "$(STAGE2_SRC)" -f bin \
		-dKERNEL_SECTORS=$$KERNEL_SECTORS \
		-dKERNEL_START_SECTOR=$$KERNEL_START_SECTOR \
		-o "$@"

$(BOOT_BIN): $(BOOT_SRC) $(STAGE2_BIN)
	@mkdir -p $(dir $@)

	@STAGE2_SIZE=$$(stat -c%s "$(STAGE2_BIN)"); \
	STAGE2_SECTORS=$$(( (STAGE2_SIZE + $(SECTOR_SIZE) - 1) / $(SECTOR_SIZE) )); \
	echo "Stage 2 sectors: $$STAGE2_SECTORS"; \
	$(ASM) "$(BOOT_SRC)" -f bin \
		-dSTAGE2_SECTORS=$$STAGE2_SECTORS \
		-o "$@"

	@BOOT_SIZE=$$(stat -c%s "$@"); \
	if [ "$$BOOT_SIZE" -gt $(SECTOR_SIZE) ]; then \
		echo "ERROR: Boot sector is larger than 512 bytes"; \
		echo "       Size: $$BOOT_SIZE bytes"; \
		exit 1; \
	fi

$(DISK_IMG): $(BOOT_BIN) $(STAGE2_BIN) $(KERNEL_BIN)
	@mkdir -p $(BUILD_DIR)

	@echo "Creating disk image..."

	@dd \
		if=/dev/zero \
		of="$@" \
		bs=$(SECTOR_SIZE) \
		count=$(DISK_SECTORS) \
		status=none

	@echo "Formatting FAT32 filesystem at sector $(FAT32_START_SECTOR)..."

	@$(MKFS) \
		-F 32 \
		-S $(SECTOR_SIZE) \
		-s 1 \
		-R 32 \
		--offset=$(FAT32_START_SECTOR) \
		"$@" >/dev/null

	@echo "Installing bootloader..."

	@dd \
		if="$(BOOT_BIN)" \
		of="$@" \
		bs=$(SECTOR_SIZE) \
		seek=0 \
		count=1 \
		conv=notrunc \
		status=none

	@echo "Installing stage 2..."

	@dd \
		if="$(STAGE2_BIN)" \
		of="$@" \
		bs=$(SECTOR_SIZE) \
		seek=1 \
		conv=notrunc \
		status=none

	@echo "Installing kernel..."

	@STAGE2_SIZE=$$(stat -c%s "$(STAGE2_BIN)"); \
	STAGE2_SECTORS=$$(( (STAGE2_SIZE + $(SECTOR_SIZE) - 1) / $(SECTOR_SIZE) )); \
	KERNEL_START_SECTOR=$$(( 1 + STAGE2_SECTORS )); \
	dd \
		if="$(KERNEL_BIN)" \
		of="$@" \
		bs=$(SECTOR_SIZE) \
		seek=$$KERNEL_START_SECTOR \
		conv=notrunc \
		status=none

	@echo "Installing /system files..."

	@$(MMD) -i "$(MTOOLS_IMAGE)" "::/system"

	@if [ -d "$(SYSTEM_DIR)" ]; then \
		find "$(SYSTEM_DIR)" -type d -mindepth 1 | while read -r dir; do \
			relative=$${dir#$(SYSTEM_DIR)/}; \
			$(MMD) -i "$(MTOOLS_IMAGE)" "::/system/$$relative"; \
		done; \
		\
		find "$(SYSTEM_DIR)" -type f | while read -r file; do \
			relative=$${file#$(SYSTEM_DIR)/}; \
			echo "  $$relative"; \
			$(MTOOLS) -i "$(MTOOLS_IMAGE)" -s "$$file" "::/system/$$relative"; \
		done; \
	fi

	@echo "Disk image ready: $@"

.PHONY: run
run: $(DISK_IMG)
	@$(QEMU) \
		$(QEMUFLAGS) \
		-display gtk \
		-drive format=raw,file=$(DISK_IMG)

.PHONY: clean
clean:
	@rm -rf "$(BUILD_DIR)" "$(BIN_DIR)"

.PHONY: rebuild
rebuild: clean all
