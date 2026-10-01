module {
  llvm.func @transfers(%arg0: i32, %flag: i1) -> i32 {
    %zero = llvm.mlir.constant(0 : i32) : i32
    %one = llvm.mlir.constant(1 : i32) : i32
    %and_lhs = llvm.and %zero, %arg0 : i32
    %and_rhs = llvm.and %arg0, %zero : i32
    %and_chain = llvm.and %and_lhs, %arg0 : i32
    %and_nn = llvm.and %one, %one : i32
    %or_n = llvm.or %one, %arg0 : i32
    %add_zz = llvm.add %zero, %zero : i32
    %and_unknown = llvm.and %arg0, %one : i32

    llvm.return %and_chain : i32
  }

  llvm.func @join(%flag: i1, %arg0: i32) -> i32 {
    %zero = llvm.mlir.constant(0 : i32) : i32
    llvm.cond_br %flag, ^lhs, ^rhs
  ^lhs:
    %a = llvm.and %zero, %arg0 : i32
    llvm.br ^exit(%a : i32)
  ^rhs:
    llvm.br ^exit(%zero : i32)
  ^exit(%merged: i32):
    llvm.return %merged : i32
  }

  llvm.func @nonnegative_join(%flag: i1) -> i32 {
    %zero = llvm.mlir.constant(0 : i32) : i32
    %positive = llvm.mlir.constant(8 : i32) : i32
    llvm.cond_br %flag, ^positive, ^zero
  ^positive:
    llvm.br ^exit(%positive : i32)
  ^zero:
    llvm.br ^exit(%zero : i32)
  ^exit(%merged: i32):
    llvm.return %merged : i32
  }

  llvm.func @nonpositive_join(%flag: i1) -> i32 {
    %zero = llvm.mlir.constant(0 : i32) : i32
    %negative = llvm.mlir.constant(-5 : i32) : i32
    llvm.cond_br %flag, ^negative, ^zero
  ^negative:
    llvm.br ^exit(%negative : i32)
  ^zero:
    llvm.br ^exit(%zero : i32)
  ^exit(%merged: i32):
    %sum = llvm.add %merged, %negative overflow<nsw> : i32
    llvm.return %sum : i32
  }

  llvm.func @constant_signs() {
    %negative = llvm.mlir.constant(-5 : i32) : i32
    %positive = llvm.mlir.constant(8 : i32) : i32
    llvm.return
  }

  llvm.func @self_subtraction(%x: i32) -> i32 {
    %difference = llvm.sub %x, %x : i32
    llvm.return %difference : i32
  }

  llvm.func @self_multiplication(%x: i32) -> i32 {
    %square = llvm.mul %x, %x overflow<nsw> : i32
    %wrapping_square = llvm.mul %x, %x : i32
    llvm.return %square : i32
  }

  llvm.func @square_plus_one(%x: i32) -> i32 {
    %one = llvm.mlir.constant(1 : i32) : i32
    %square = llvm.mul %x, %x overflow<nsw> : i32
    %answer = llvm.add %square, %one overflow<nsw> : i32
    llvm.return %answer : i32
  }

  llvm.func @mixed_sign_arithmetic() {
    %negative = llvm.mlir.constant(-5 : i32) : i32
    %positive = llvm.mlir.constant(8 : i32) : i32
    %product = llvm.mul %negative, %positive overflow<nsw> : i32
    %negative_squared = llvm.mul %negative, %negative overflow<nsw> : i32
    %difference = llvm.sub %positive, %negative overflow<nsw> : i32
    %reverse_difference = llvm.sub %negative, %positive overflow<nsw> : i32
    %sum = llvm.add %positive, %positive overflow<nsw> : i32
    %negative_sum = llvm.add %negative, %negative overflow<nsw> : i32
    %wrapping_sum = llvm.add %positive, %positive : i32
    %wrapping_difference = llvm.sub %positive, %negative : i32
    llvm.return
  }
}
