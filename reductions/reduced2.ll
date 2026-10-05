target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-macosx15.1.0"

define i32 @square_plus_one() {
  %1 = add nsw i32 1, 1
  ret i32 %1
}
