# CPU Emulator implementation ![Build Status](https://github.com/V13kv/CPU-Emulator/workflows/BuildAndTest/badge.svg)
Implementation of [`CPU`](https://en.wikipedia.org/wiki/Central_processing_unit) emulator and custom [`assembler`](https://en.wikipedia.org/wiki/Assembly_language#Assembler) for it.  
Third task in MIPT in the first year of education.

## Motivation
The idea is to understand how CPU works, how CPU communicates with devices (input, output), how assembler works, how translation process is done, how CPU processes [`bytecode`](https://en.wikipedia.org/wiki/Bytecode), what is CPU [`microcode`](https://en.wikipedia.org/wiki/Microcode) and how it is implemented and some other intricacies of CPU, assembler.

## Documentation
You can see the user documentation [`here`](https://docs.google.com/document/d/1youW9-Raz-lGzc0pJ37jc7rNF6vHhlz0xsxcjXyCMYY/edit?usp=sharing) and developer documentation [`here`](https://docs.google.com/document/d/1q9ce8neP75xqnBPQkJJ05GUwjR5-Uvag2BscOHG7MAw/edit?usp=share_link).

## What does this project have?
This project has two main parts:  
1. Assembler implementation with custom [`ISA`](https://en.wikipedia.org/wiki/Instruction_set_architecture).
2. Custom CPU Emulator implementation with [`DSL`](https://en.wikipedia.org/wiki/Domain-specific_language) for it.

## Supported assembler commands
1. Data manipulating: `push`, `pop`.
2. Arithmetic: `add`, `sub`, `mul`, `div`, `sqrt`.
3. Program flow control: `call`, `ret`, `jmp`, `je`, `jl`, `jg`, `jne`.
4. I/O: `out`, `outc`, `in`.
5. Logical: `cmp`.

## Program architecture 
Coming soon...

## Implementation details
Coming soon...

## Setting up
**Clone the repository**
```
git clone https://github.com/V13kv/CPU-Emulator;
cd CPU-Emulator
```
**Compiling**
```
make init;
make asm proc
```

## Running
```
./asm.exe <path_to_vasm_file> <output_file_name>
```

```
./proc.exe <path_to_compiled_vasm_file>
```
