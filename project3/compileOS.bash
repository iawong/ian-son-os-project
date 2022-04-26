#!/bin/bash

dd if=/dev/zero of=floppya.img bs=512 count=2880

nasm bootload.asm

dd if=bootload of=floppya.img bs=512 count=1  conv=notrunc seek=0
dd if=map.img of=floppya.img bs=512 count=1 seek=1 conv=notrunc
dd if=dir.img of=floppya.img bs=512 count=1 seek=2 conv=notrunc

bcc -ansi -c -o kernel.o kernel.c
bcc -ansi -c -o uprog1.o uprog1.c
bcc -ansi -c -o uprog2.o uprog2.c
bcc -ansi -c -o shell.o shell.c
bcc -ansi -c -o userLib.o userLib.c

as86 kernel.asm -o kernel_asm.o
as86 lib.asm -o lib.o

ld86 -o kernel -d kernel.o kernel_asm.o
ld86 -o uprog1 -d uprog1.o lib.o
ld86 -o uprog2 -d uprog2.o lib.o
ld86 -o shell -d shell.o userLib.o lib.o

dd if=kernel of=floppya.img bs=512 conv=notrunc seek=3

dd if=message.txt of=floppya.img bs=512 conv=notrunc seek=30

gcc -o loadFile loadFile.c

./loadFile message.txt
./loadFile uprog1
./loadFile uprog2
./loadFile shell
./loadFile userLib

bochs -f opsys.bxrc