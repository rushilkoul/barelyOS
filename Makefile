NPROC     := $(shell nproc)
BR_DIR    := buildroot
PATCH     := patch.sh
DEFCONFIG := $(CURDIR)/barelyos_defconfig

.PHONY: all build defconfig saveconfig patch menuconfig linux-menuconfig run clean


all: build

build:
	$(MAKE) -C $(BR_DIR) -j$(NPROC)

defconfig:
	$(MAKE) -C $(BR_DIR) BR2_DEFCONFIG=$(DEFCONFIG) defconfig

saveconfig:
	$(MAKE) -C $(BR_DIR) BR2_DEFCONFIG=$(DEFCONFIG) savedefconfig

patch:
	chmod +x $(PATCH)
	./$(PATCH)

menuconfig:
	$(MAKE) -C $(BR_DIR) menuconfig

run:
	$(BR_DIR)/output/images/start-qemu.sh

clean:
	$(MAKE) -C $(BR_DIR) clean
