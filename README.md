
Project Report: Real-Time Clock (RTC) OS
________________________________________
1. Introduction
This project aims to develop a basic Real-Time Clock (RTC) OS that fetches the current system time using the CMOS, displays it on the screen, updates it every second, and allows manual adjustment. The implementation includes setting up a minimal OS kernel, using VGA for output, and handling user input for time adjustments.
________________________________________
2. Setting Up the System
To start with the development, the following steps were followed:
1.	Installed a Linux environment (WSL) on Windows 11.
2.	Installed necessary dependencies including a cross-compiler and QEMU.
3.	Created a kernel directory structure to organize the project.
4.	Wrote essential files including kernel.c, linker.ld, and a shell script for execution.
________________________________________
3. Installing the Cross Compiler
To compile our OS kernel, we required a cross-compiler for i386 architecture. The following commands were used:
sudo apt update
sudo apt install -y build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo
Then, we built the GCC cross-compiler for i686-elf:
wget https://ftp.gnu.org/gnu/binutils/binutils-2.38.tar.gz
tar -xvzf binutils-2.38.tar.gz
cd binutils-2.38
mkdir build
cd build
../configure --target=i686-elf --prefix=/usr/local/cross --disable-nls --enable-gprofng=no --disable-werror
make -j$(nproc)
sudo make install
Similarly, we installed GCC for i686-elf:
wget https://ftp.gnu.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.gz
tar -xvzf gcc-12.2.0.tar.gz
cd gcc-12.2.0
mkdir build
cd build
../configure --target=i686-elf --prefix=/usr/local/cross --disable-nls --enable-languages=c,c++ --without-headers
make -j$(nproc) all-gcc all-target-libgcc
sudo make install-gcc install-target-libgcc
________________________________________
4. Installing QEMU
QEMU was required to emulate the OS. It was installed using:
sudo apt install qemu-system-x86
________________________________________
5. Creating the Kernel Directory and Files
The directory structure was as follows:
/rtc-os/
 ├── kernel/
 │   ├── kernel.c
 │   ├── linker.ld
 │   ├── Makefile
 │   ├── iso/
 │   │   ├── boot/
 │   │   │   ├── grub/
 │   │   │   │   ├── grub.cfg
Writing kernel.c
The kernel.c file handles system initialization, VGA output, RTC retrieval, and user input processing.
•	Fetching Time from CMOS: The CMOS is accessed using I/O ports 0x70 (address) and 0x71 (data).
•	Displaying Time on Screen: VGA memory at 0xB8000 is used to print the time.
•	Updating Time Every Second: A simple delay loop keeps refreshing the time.
•	Manual Adjustment of Time: User inputs allow modifying the stored time.
Writing linker.ld
A linker script was created to specify memory layout and entry point:
OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)
SECTIONS {
    . = 1M;
    .text : { *(.multiboot) *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}
Creating the Boot Image
A bootable ISO was made using GRUB:
mkdir -p iso/boot/grub
echo 'set timeout=0
menuentry "RTC OS" {
    multiboot /boot/kernel.elf
}' > iso/boot/grub/grub.cfg

cp kernel/kernel.elf iso/boot/kernel.elf
grub-mkrescue -o rtc-os.iso iso/
________________________________________
6. Executing the Kernel
A shell script was created to run the OS in QEMU:
qemu-system-i386 -cdrom rtc-os.iso
________________________________________
7. Kernel Working Explanation
Basic Structure
•	Multiboot Header: Ensures the kernel is loaded correctly.
•	VGA Text Mode: Directly writes to 0xB8000 for output.
•	CMOS Time Retrieval: Uses I/O ports to get time values.
•	User Input Handling: Processes keyboard events for time adjustments.
Working of CMOS Ports
CMOS registers store date/time. The ports used:
Register	Purpose
0x00	Seconds
0x02	Minutes
0x04	Hours
0x0A	Status Register A
________________________________________
8. Output Time Every Second
A delay was implemented using:
for (volatile int i = 0; i < 100000000; i++);
This ensures the time updates in real-time.
________________________________________
9. Adjusting Time Manually
The following keys were used to modify the time:
•	e → Toggle edit mode
•	h → Increment hours
•	m → Increment minutes
•	s → Increment seconds
These modifications were written to CMOS using:
outb(CMOS_ADDRESS, reg);
outb(CMOS_DATA, value);
________________________________________
10. Alarm Functionality
The alarm feature allows the user to set a specific time (hours, minutes, seconds). When the current CMOS time matches the alarm time:
•	The screen flashes or displays an "Alarm Triggered!" message.
•	The color of the background may change to alert the user.
User keys used for alarm:
•	a → Toggle alarm set mode
•	H → Increment alarm hour
•	M → Increment alarm minute
•	S → Increment alarm second

________________________________________
11. Contributions
Each team member contributed as follows:
•	Sohan Roy Chowdhury, Soumya Kumar Singh, Shashwat Nautiyal, Charan Teja:
•	Developed the kernel.c file including time retrieval and VGA output.
•	Sidharthkrishna Nair:
•	Created and refined the linker.ld file for correct memory mapping.
•	Simarjeet Singh:
•	Worked on building the bootable image using GRUB.
•	Siddharth Pradhan, Shaik Farhat:
•	Set up the development environment including QEMU and cross-compilers.
________________________________________
12. Conclusion
This project successfully implemented a basic Real-Time Clock OS that fetches and displays CMOS time, updates it every second, and allows manual adjustment. Future improvements could include setting the time via network synchronization.
________________________________________


