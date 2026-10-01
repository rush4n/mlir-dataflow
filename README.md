# Extended Sign Analysis for MLIR

This project implements the extended sign lattice from Chapter 5 of SPA

This analysis derives the following nontrivial facts:

(see examples/output.txt)

```mlir
%2 = llvm.mul %1, %1 overflow<nsw> : i64 // %2 is nonnegative
%3 = llvm.add %2, %0 overflow<nsw, nuw> : i64 // %3 is positive
```

(see examples/sqlite3.txt)

```mlir
// argument: %18 is nonnegative
%34 = llvm.add %18, %7 overflow<nsw, nuw> : i64 // %34 is positive
```

## Requirements

- LLVM and MLIR 23
- CMake 3.20 or newer
- A C++17 compiler

## Build and test

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Run

```sh
./run.sh foo.mlir
```

## Example

```c
long square_plus_one(int val) {
  long x = val;
  return x * x + 1;
}
```

```sh
clang -O1 -S -emit-llvm examples/example.c \
  -o examples/example.ll
mlir-translate --import-llvm examples/example.ll \
  -o examples/example.mlir
./run.sh examples/example.mlir
```

The saved analysis output is in examples/output.txt
