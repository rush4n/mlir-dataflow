//===- ZeroAnalysis.h - Sparse forward analysis over ZeroState ------------===//

#ifndef SIGN_ANALYSIS_H
#define SIGN_ANALYSIS_H

#include "SignDomain.h"
#include "mlir/Analysis/DataFlow/SparseAnalysis.h"

namespace sign {

using SignLattice = mlir::dataflow::Lattice<SignState>;

class SignAnalysis
    : public mlir::dataflow::SparseForwardDataFlowAnalysis<SignLattice> {
public:
  using SparseForwardDataFlowAnalysis::SparseForwardDataFlowAnalysis;

  mlir::LogicalResult
  visitOperation(mlir::Operation *op,
                 llvm::ArrayRef<const SignLattice *> operands,
                 llvm::ArrayRef<SignLattice *> results) override;

  void setToEntryState(SignLattice *lattice) override;
};

} // namespace sign

#endif
