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

then generate the buildroot config:
```bash
make defconfig
```

---

### building & running

build the image:
```bash
make
```
launch QEMU and run the image:
```bash
make run
```

or manually:
```bash
qemu-system-x86_64 -M pc \
  -kernel buildroot/output/images/bzImage \
  -drive file=buildroot/output/images/rootfs.ext2,if=virtio,format=raw \
  -append "rootwait root=/dev/vda console=tty1 console=ttyS0 init=/sbin/barely-init" \
  -net nic,model=virtio -net user \
  -nographic
```

to exit: `Ctrl A, X` or type `poweroff` in the shell


---

### roadmap

- [x] bootable Linux system in QEMU
- [x] working baseline userspace
- [ ] custom PID 1 (barely-init)
- [ ] custom shell (barely-sh)
- [ ] custom utilities

--- 

### philosophy

barelyOS isn't about reinventing Linux.

we use existing infrastructure where it makes sense,
and build the parts we actually want to understand.

this is very much a "learning" project.