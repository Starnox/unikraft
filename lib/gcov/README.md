# Unikraft Library for Profiling and Test Coverage

This library introduces initial setup with config options for console, binary file, and memory output. It takes advantage of gcc-13's newest feature of Profiling and Test Coverage in Freestanding Environments.

## Prerequisites

One way to build with `gcc-13` is to use a `Docker` container - [DockerImage](https://hub.docker.com/_/gcc/)

## Configuration

To utilize this library, you will need to select and configure some options:

- Select `gcov` from `Library Configuration` KConfig menu.
- Choose the way you want to extract the `gcov` information from the kernel.

### File System Configuration for extracting via File Output

- In `plat` -> `kvm` -> `virtio`, select:
  - `virtio PCI device support`
  - `virtio 9P device`
- In `lib` -> `vfscore`, select:
  - `Default root filesystem (9PFS)`
- Create a directory `fs0` where the resulting filename with the name provided in `KConfig` will reside.

### Note

Make sure that `gcov-tool` and `gcov` are upgraded to the latest `gcc-13` version as well.

## Usage

To extract the coverage information, you can call the `gcov_dump_info()` function at the end of the program. Various output methods are provided:

### 1. Console Output

*Avoid using the `-nographic` option as it clashes with the redirection of standard output.*

Example:

```bash
qemu-system -chardev stdio,id=char0,logfile=serial.log,signal=off -serial chardev:char0
```

### 2. File Output

Configure the file system as detailed above, and the results will reside in the directory `fs0`.

### 3. Memory Output

Use GDB and issue the commands to dump the coverage data into a binary file named `memory.bin`.

Example:

```bash
dump memory memdump.bin gcov_output_buffer gcov_output_buffer+gcov_output_buffer_pos
```

## Creating Coverage Report

Process the output using `ukgcov`'s `gcov_process.sh` script, a wrapper around tools provided by [`embedded-gcov`](https://github.com/nasa-jpl/embedded-gcov). Install dependencies `dox2unix` and `lcov`, then invoke `gcov_process.sh` with parameters depending on the output method selected.

Examples:

**For Console Output:**

```bash
./support/scripts/gcov/gcov_process.sh -c <console_output> <build_directory>
```

**For Binary File:**

```bash
./support/scripts/gcov/gcov_process.sh -b <binary_output> <build_directory>
```

## Conclusion

This new internal library provides a powerful set of tools for Profiling and Test Coverage in Freestanding Environments, leveraging the features of gcc-13.