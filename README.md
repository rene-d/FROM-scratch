# FROM scratch

Demonstration of creating a base image that contains a statically-linked program.

## The background

In classic Linux distro, some libraries are available only for dynamic linking (ex: GLib). Even the [glibc](https://www.gnu.org/software/libc/) requires a dynamically loaded library for some functions ([read here](https://stackoverflow.com/questions/2725255/create-statically-linked-binary-that-uses-getaddrinfo)).

> Indeed, on [`getaddrinfo()`](https://linux.die.net/man/3/getaddrinfo) calls, static and dynamic flavours of `glibc` always **dynamically** load at least `libnss_files.so.2` which depends on `libc.so` itself. So you *cannot* make a static self-sufficient executable that uses `getaddrinfo()` API with the standard `glibc`.

In order to create an [empty Docker image](https://docs.docker.com/develop/develop-images/baseimages/) that contains just an executable, you will need statically linked program that will not require or load any dynamic libraries, since the image will not have any shared libraries.

The best choice is to use [musl-libc](https://www.musl-libc.org) and [Alpine Linux](https://alpinelinux.org), where all the hard work has already be done.

## Building the image

The Docker image uses a [multi-stage build](https://docs.docker.com/develop/develop-images/multistage-build/).

- The first stage prepares the build environment (compiler and libraries).
- The second one creates a bare image with only the program and its data.

## Running the demo

The [demo.sh](demo.sh) script builds and inspects the image ([jq](https://stedolan.github.io/jq/) is required to filter the [inspect](https://docs.docker.com/engine/reference/commandline/inspect/) output, or comment line 8).

Then, it runs the unique executable (`a.out`) in the container.

![demo.png](https://raw.githubusercontent.com/rene-d/FROM-scratch/master/demo.png)

## To go further : technical details

To understand why it works, let's look deeper in `a.out`:

```
$ file a.out
a.out: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), statically linked, stripped
```
Technically, `a.out` makes only [system calls](https://en.wikipedia.org/wiki/System_call) to the kernel, and does not rely on any other libraries. Ant it can be run on any Linux that supports x86-64 architecture.

Linux has many [system calls](https://filippo.io/linux-syscall-table/). Symbolic constants for system call numbers can be found in the header file `<sys/syscall.h>` or in the [kernel tree](https://github.com/torvalds/linux/blob/master/arch/x86/entry/syscalls/syscall_64.tbl).

## An example: hello.c

Let's consider a classic « Hello World »:
```c
#include <stdio.h>

int main()
{
	puts("hello");
}
```

## Dynamically linked executable

By default, `cc` compiles and links dynamically:
```
/ # cc -o hello hello.c
/ # file hello
hello: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib/ld-musl-x86_64.so.1, with debug_info, not stripped
/ # ldd hello
	/lib/ld-musl-x86_64.so.1 (0x7fc14504b000)
	libc.musl-x86_64.so.1 => /lib/ld-musl-x86_64.so.1 (0x7fc14504b000)
```

Let's look at `main` function disassembly:

```asm
000000000000119a <main>:
    119a:	55                   	push   %rbp
    119b:	48 89 e5             	mov    %rsp,%rbp
    119e:	48 8d 3d 5b 0e 00 00 	lea    0xe5b(%rip),%rdi        # 2000 <_fini+0xe2d>
    11a5:	e8 76 fe ff ff       	callq  1020 <puts@plt>
    11aa:	b8 00 00 00 00       	mov    $0x0,%eax
    11af:	5d                   	pop    %rbp
    11b0:	c3                   	retq
```

`puts()` at `0x11a5` will jump to its PLT entry, that uses the [GOT](https://en.wikipedia.org/wiki/Global_Offset_Table), then jumps to `puts` entry of the shared libc function in the shared library (here, [musl-libc](https://www.musl-libc.org)).

```asm
0000000000001020 <puts@plt>:
    1020:       ff 25 a2 2f 00 00       jmpq   *0x2fa2(%rip)        # 3fc8 <puts>
```

## Static executable

Same program, with `-static` option:
```
/ # cc -static -o hello hello.c
```

`main` function:
```asm
0000000000001070 <main>:
    1070:	48 83 ec 08          	sub    $0x8,%rsp
    1074:	48 8d 3d 85 0f 00 00 	lea    0xf85(%rip),%rdi        # 2000 <_fini+0x13e>
    107b:	e8 a2 04 00 00       	callq  1522 <puts>
    1080:	31 c0                	xor    %eax,%eax
    1082:	48 83 c4 08          	add    $0x8,%rsp
    1086:	c3                   	retq
```

Now, `puts()` calls a routine implemented within the executable. This routine calls others, then ends to a `syscall` assembly instruction in the `__stdio_write` routine.

`hello.c` is somewhere equivalent to this assembly program (in AT&T syntax):

```asm
.section .rodata
    hello: .string "hello\n"

.section .text
    .globl main
    .type main, @function

main:
    mov $1, %rax            # SYS_write
    mov $1, %rdi            # stdout
    lea hello(%rip), %rsi   # "hello\n"
    mov $6, %rdx            # length
    syscall

    mov $60, %rax           # SYS_exit
    mov $0, %rdi            # exit code
    syscall
```

To run it:
```bash
as -o hello.o hello.s
ld -s --entry main -o hello hello.o
```
