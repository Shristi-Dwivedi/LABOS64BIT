# LABOS : A Conceptual Operating System

LABOS is a bootable operating system built from scratch using **C and Assembly**.  
This project is part of my exploration into **low-level system development, kernels, and OS architecture**.

## Current Development Stage
LABOS is currently in **early development**.

## Features Implemented So Far

- Bootloader initialization
- Kernel loading
- Basic screen output
- Simple shell environment
- Command execution framework
- Mouse support
- GUI booting
- Desktop Loading
- 2 basic applications
- Implementation of Calculator app

## Features In Progress

- GUI system
- Event handling
- Window manager

## Screenshots

### GRUB Menu
![GRUB menu](screenshots/LABOS_GRUB.png)

### Shell Interface
![Shell](screenshots/LABOS_SHELL.png)

### Boot Screen
![Boot Screen](screenshots/LABOS_GUI_BOOT.png)

### GUI Screen
![GUI Screen](screenshots/LABOS_GUI.png)

### Desktop
![Desktop Screen](screenshots/LABOS_Desktop.png)

### Calculator
![Calculator](screenshots/LABOS_Calc.png)

## How to Run

Run using **QEMU**:

```bash
qemu-system-x86_64 -cdrom dist/x86_64/kernel.iso -vga std
```
or
```bash
qemu-system-x86_64 kernel.iso
