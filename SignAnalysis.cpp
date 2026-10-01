//===- ZeroAnalysis.cpp - Transfer functions ------------------------------===//
//
// The transfer function: given what is known about an operation's operands,
// state what is known about its results.  This file and ZeroDomain.h are the
// two to replace when building a different analysis; the rest of the project
// is scaffolding.
//
// There are deliberately only two rules here, one of each kind an analysis
// needs: one that introduces facts out of nothing (constants), and one that
// propagates facts it was given (`and`).  Everything else is unknown.  Adding
// a third rule should be a matter of adding a third `if`.
//
//===----------------------------------------------------------------------===//

#include "SignAnalysis.h"

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Matchers.h"

using namespace mlir;

namespace sign {

namespace {

bool isPositive(Kind kind) {
  return kind == Kind::One || kind == Kind::Positive;
}

bool isNegative(Kind kind) { return kind == Kind::Negative; }

bool isNonNegative(Kind kind) {
  return kind == Kind::Zero || kind == Kind::One || kind == Kind::Positive ||
         kind == Kind::NonNegative;
}

bool isNonPositive(Kind kind) {
  return kind == Kind::Zero || kind == Kind::Negative ||
         kind == Kind::NonPositive;
}

Kind negate(Kind kind) {
  switch (kind) {
  case Kind::Bottom:
    return Kind::Bottom;
  case Kind::Zero:
    return Kind::Zero;
  case Kind::One:
  case Kind::Positive:
    return Kind::Negative;
  case Kind::Negative:
    return Kind::Positive;
  case Kind::NonNegative:
    return Kind::NonPositive;
  case Kind::NonPositive:
    return Kind::NonNegative;
  case Kind::Top:
    return Kind::Top;
  }
  return Kind::Top;
}

} // namespace

void SignAnalysis::setToEntryState(SignLattice *lattice) {
  propagateIfChanged(lattice, lattice->join(SignState::top()));
}

LogicalResult
SignAnalysis::visitOperation(Operation *op,
                             ArrayRef<const SignLattice *> operands,
                             ArrayRef<SignLattice *> results) {
  auto unknown = [&] {
    setAllToEntryStates(results);
    return success();
  };

  if (op->getNumResults() != 1 || !op->getResult(0).getType().isIntOrIndex())
    return unknown();
  SignLattice *result = results[0];

  auto update = [&](SignState state) {
    propagateIfChanged(result, result->join(state));
    return success();
  };

  IntegerAttr value;
  if (matchPattern(op, m_Constant(&value))) {
    const llvm::APInt &integer = value.getValue();

    Kind kind;
    if (integer.isZero()) {
      kind = Kind::Zero;
    } else if (integer.isNegative()) {
      kind = Kind::Negative;
    } else if (integer.isOne()) {
      kind = Kind::One;
    } else {
      kind = Kind::Positive;
    }

    return update(kind);
  }

  if (auto sub = dyn_cast<LLVM::SubOp>(op)) {
    SignState lhs = operands[0]->getValue();
    SignState rhs = operands[1]->getValue();

    if (lhs.isBottom() || rhs.isBottom())
      return success();

    if (op->getOperand(0) == op->getOperand(1))
      return update(Kind::Zero);

    if (rhs.kind == Kind::Zero)
      return update(lhs);

    if (!sub.hasNoSignedWrap())
      return unknown();

    if (lhs.kind == Kind::Zero)
      return update(negate(rhs.kind));

    if ((isPositive(lhs.kind) && isNonPositive(rhs.kind)) ||
        (isNonNegative(lhs.kind) && isNegative(rhs.kind)))
      return update(Kind::Positive);

    if ((isNegative(lhs.kind) && isNonNegative(rhs.kind)) ||
        (isNonPositive(lhs.kind) && isPositive(rhs.kind)))
      return update(Kind::Negative);

    if (isNonNegative(lhs.kind) && isNonPositive(rhs.kind))
      return update(Kind::NonNegative);

    if (isNonPositive(lhs.kind) && isNonNegative(rhs.kind))
      return update(Kind::NonPositive);

    return unknown();
  }

  if (auto mul = dyn_cast<LLVM::MulOp>(op)) {
    SignState lhs = operands[0]->getValue();
    SignState rhs = operands[1]->getValue();

    if (lhs.isBottom() || rhs.isBottom())
      return success();

    if (lhs.kind == Kind::Zero || rhs.kind == Kind::Zero)
      return update(Kind::Zero);

    if (lhs.kind == Kind::One)
      return update(rhs);

    if (rhs.kind == Kind::One)
      return update(lhs);

    if (!mul.hasNoSignedWrap())
      return unknown();

    if ((isPositive(lhs.kind) && isPositive(rhs.kind)) ||
        (isNegative(lhs.kind) && isNegative(rhs.kind)))
      return update(Kind::Positive);

    if ((isPositive(lhs.kind) && isNegative(rhs.kind)) ||
        (isNegative(lhs.kind) && isPositive(rhs.kind)))
      return update(Kind::Negative);

    if ((isNonNegative(lhs.kind) && isNonNegative(rhs.kind)) ||
        (isNonPositive(lhs.kind) && isNonPositive(rhs.kind)))
      return update(Kind::NonNegative);

    if ((isNonNegative(lhs.kind) && isNonPositive(rhs.kind)) ||
        (isNonPositive(lhs.kind) && isNonNegative(rhs.kind)))
      return update(Kind::NonPositive);

    if (op->getOperand(0) == op->getOperand(1))
      return update(Kind::NonNegative);

    return unknown();
  }

  if (auto add = dyn_cast<LLVM::AddOp>(op)) {
    SignState lhs = operands[0]->getValue();
    SignState rhs = operands[1]->getValue();

    if (lhs.isBottom() || rhs.isBottom())
      return success();

    if (lhs.kind == Kind::Zero)
      return update(rhs);

    if (rhs.kind == Kind::Zero)
      return update(lhs);

    if (!add.hasNoSignedWrap())
      return unknown();

    if ((isPositive(lhs.kind) && isNonNegative(rhs.kind)) ||
        (isNonNegative(lhs.kind) && isPositive(rhs.kind)))
      return update(Kind::Positive);

    if (isNonNegative(lhs.kind) && isNonNegative(rhs.kind))
      return update(Kind::NonNegative);

    if ((isNegative(lhs.kind) && isNonPositive(rhs.kind)) ||
        (isNonPositive(lhs.kind) && isNegative(rhs.kind)))
      return update(Kind::Negative);

    if (isNonPositive(lhs.kind) && isNonPositive(rhs.kind))
      return update(Kind::NonPositive);

    return unknown();
  }

  if (isa<LLVM::AndOp>(op)) {
    SignState lhs = operands[0]->getValue();
    SignState rhs = operands[1]->getValue();

    if (lhs.isBottom() || rhs.isBottom())
      return success();

    if (lhs.kind == Kind::Zero || rhs.kind == Kind::Zero)
      return update(Kind::Zero);
  }

  return unknown();
}

} // namespace sign
