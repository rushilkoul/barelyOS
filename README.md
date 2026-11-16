### barelyOS
barely a linux distribution.

---

### what's in the box

- **arch:** x86_64
- **kernel:** Linux 6.18.7
- **init (PID 1):** barely-init (custom, `src/init/`)
- **shell:** barelysh (custom, `src/shell/`) with BusyBox fallback for unimplemented commands
- **libc:** glibc
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

### building & running

#### first-time build


```bash
make defconfig
make -C buildroot -j$(nproc)
make
```

#### subsequent builds
```bash
make
```

#### running
```bash
make run

# or if you want to use your own terminal:
make run-os
```

or manually:
```bash
qemu-system-x86_64 -M pc \
  -kernel buildroot/output/images/bzImage \
  -drive file=buildroot/output/images/rootfs.ext2,if=virtio,format=raw \
  -append "rootwait root=/dev/vda rw console=tty1 console=ttyS0 init=/sbin/barely-init quiet loglevel=3" \
  -net nic,model=virtio -net user \
  -nographic
```

to exit: `Ctrl A, X` or type `poweroff` in the shell


---

### roadmap

- [x] bootable Linux system in QEMU
- [x] working baseline userspace
- [x] custom PID 1 (barely-init)
- [x] custom shell (barelysh) — builtins in progress
- [x] custom utilities (ls, cat, mkdir, mv, ...)

--- 

### philosophy

barelyOS isn't about reinventing Linux.

we use existing infrastructure where it makes sense,
and build the parts we actually want to understand.

this is very much a "learning" project.