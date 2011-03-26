#description

This is a port of the ucore project to the ARM architecture.

'ucore' is a teaching operating system used by Tsinghua University.
Project homepage: [http://ucore.googlecode.com].

Currently this port runs on the S3C2410X microprocessor, which is developed
using an ARM920T core.

#source code organization

* 'arch' contains architecture dependent driver and boot code. 
* 'lib' contains architecture independent library code. 
* 'kern' contains architecture independent kernel code.
* 'tool' contains various tools such as GDB scripts and emulator configurations.

#todo

* Rewrite Makefile, make it DRY and more like original ucore Makefile.
* Make clock interrupts to work.
* Find a way to do division on ARM.

#acknowledgment

* Most code is taken from the ucore project, of course.
* Boot code is inspired by david leels <davidontech@gmail.com>.
* div64 code for arm is taken from the Linux kernel.
