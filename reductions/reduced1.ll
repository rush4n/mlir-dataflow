target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-macosx15.1.0"

define i32 @loop_counter() {
  br label %1

1:                                                ; preds = %1, %0
  %2 = phi i32 [ 1, %1 ], [ 0, %0 ]
  br label %1
}
