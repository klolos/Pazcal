#!/bin/bash

as $1.s -o $1.o --32
ld -m elf_i386 $1.o lib/*.o -o $1
