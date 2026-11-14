### barelyOS
barely a linux distribution.

---

### what's in the box

- **arch:** x86_64
- **kernel:** Linux 6.18.7
- **userspace:** BusyBox
- **libc:** glibc (buildroot toolchain)
- **rootfs:** ext2, 60MB
- **emulator:** QEMU (pc machine, virtio disk + net)

---

### setup

**arch**
```bash
sudo pacman -S base-devel git wget cpio unzip rsync bc python ncurses qemu-system-x86 qemu-img grub
```

**fedora**
```bash
sudo dnf install @development-tools git wget cpio unzip rsync bc python3 ncurses-devel qemu-system-x86 qemu-img grub2-tools
```

buildroot is a git submodule, pinned to `2026.08.1`. clone with:
```bash
git clone --recurse-submodules https://github.com/rushilkoul/barelyOS.git

# if already cloned:
git submodule update --init --recursive
```

---

### building

first build is a two-step process. buildroot extracts sources mid-build, and one of them (zstd 1.5.7) uses a renamed LZ4 type that breaks:

```bash
# 1. let buildroot extract sources. will fail on zstd!
make -C buildroot -j$(nproc)

# 2. apply the patch
./patch.sh

# 3. resume
make -C buildroot -j$(nproc)
```
###### yes, this is hacky. but i specialize in hacky

after the first successful build, incremental builds are just:
```bash
make -C buildroot -j$(nproc)
```


---

### running

```bash
./buildroot/output/images/start-qemu.sh
```

or manually:
```bash
qemu-system-x86_64 -M pc \
  -kernel buildroot/output/images/bzImage \
  -drive file=buildroot/output/images/rootfs.ext2,if=virtio,format=raw \
  -append "rootwait root=/dev/vda console=tty1 console=ttyS0" \
  -net nic,model=virtio -net user \
  -nographic
```

to exit: `poweroff` or `Ctrl-A X`


---

### roadmap

- [x] bootable Linux system in QEMU
- [x] working baseline userspace
- [ ] custom PID 1 (barely-init)
- [ ] custom shell (barely-sh)
- [ ] custom utilities