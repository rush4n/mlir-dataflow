#!/bin/sh
set -eu

BUILD_DIR="${BUILD_DIR:-build}"

if [ -z "${PLUGIN:-}" ]; then
  for candidate in "$BUILD_DIR"/SignAnalysis.so "$BUILD_DIR"/SignAnalysis.dylib; do
    if [ -f "$candidate" ]; then
      PLUGIN="$candidate"
      break
    fi
  done
fi

if [ -z "${PLUGIN:-}" ]; then
  echo "No plugin found in $BUILD_DIR; build it first (see README.md)." >&2
  exit 1
fi

if [ "$#" -lt 1 ]; then
  echo "usage: $0 input.mlir" >&2
  exit 2
fi

if [ -z "${MLIR_OPT:-}" ]; then
  for candidate in mlir-opt mlir-opt-23; do
    if command -v "$candidate" >/dev/null 2>&1; then
      MLIR_OPT="$candidate"
      break
    fi
  done
fi

if [ -z "${MLIR_OPT:-}" ]; then
  echo "mlir-opt not found" >&2
  exit 1
fi

"$MLIR_OPT" --load-pass-plugin="$PLUGIN" \
             --pass-pipeline='builtin.module(sign-analysis)' \
             "$@" 2>&1 1>/dev/null
