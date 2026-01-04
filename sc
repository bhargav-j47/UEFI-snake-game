#!bash
make clean
rm -rf image/*
make
cp game.efi image/
qemu-system-x86_64 -bios /usr/share/ovmf/x64/OVMF.4m.fd -drive format=raw,file=fat:rw:image -net none 
