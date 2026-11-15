
NPROC     := $(shell nproc)
BR_DIR    := buildroot
DEFCONFIG := $(CURDIR)/barelyos_defconfig
TARGET_CC := $(CURDIR)/$(BR_DIR)/output/host/bin/x86_64-buildroot-linux-gnu-gcc

INIT_SRC  := src/init/main.c
INIT_BIN  := rootfs-overlay/sbin/barely-init

SHELL_DIR := src/shell
SHELL_BIN := rootfs-overlay/bin/barelysh

.PHONY: all build init shell defconfig saveconfig menuconfig linux-menuconfig run clean

all: build

build: init shell
	$(MAKE) -C $(BR_DIR) -j$(NPROC)

init: $(INIT_BIN)

$(INIT_BIN): $(INIT_SRC)
	$(TARGET_CC) -Wall -Wextra -O2 $< -o $@

shell:
	$(MAKE) -C $(SHELL_DIR) CC=$(TARGET_CC)

defconfig:
	$(MAKE) -C $(BR_DIR) BR2_DEFCONFIG=$(DEFCONFIG) defconfig

saveconfig:
	$(MAKE) -C $(BR_DIR) BR2_DEFCONFIG=$(DEFCONFIG) savedefconfig

menuconfig:
	$(MAKE) -C $(BR_DIR) menuconfig

linux-menuconfig:
	$(MAKE) -C $(BR_DIR) linux-menuconfig

run:
	qemu-system-x86_64 -M pc \
		-kernel $(BR_DIR)/output/images/bzImage \
		-drive file=$(BR_DIR)/output/images/rootfs.ext2,if=virtio,format=raw \
		-append "rootwait root=/dev/vda console=tty1 console=ttyS0 init=/sbin/barely-init" \
		-net nic,model=virtio -net user \
		-nographic

clean:
	$(MAKE) -C $(SHELL_DIR) clean
	$(MAKE) -C $(BR_DIR) clean
