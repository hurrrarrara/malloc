# Malloc

A custom implementation of the standard memory allocation functions:

* `malloc()`
* `calloc()`
* `free()`
* `realloc()`
* `reallocarray()`

## Description

This project recreates the behavior of the C standard library allocator by managing memory directly from the operating system.

The allocator handles:

* Dynamic memory allocation
* Memory deallocation
* Memory resizing
* Memory alignment
* Reuse of freed blocks
* Defragmentation

## Build

```bash
make
```

## Usage

Load the allocator before running a program:

```bash
export LD_PRELOAD=./libft_malloc.so
```

Example:

```bash
LD_PRELOAD=./libft_malloc.so ls
```
