#!/bin/sh

tmp=$(mktemp)
trap 'rm -f "$tmp"' EXIT

mlir-translate --import-llvm "$1" -o "$tmp" || exit 1
(cd .. && ./run.sh "$tmp") 2>&1 |
  grep -Eq 'llvm\.sub .* is zero'
