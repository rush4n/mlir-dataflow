target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-macosx15.1.0"

define i32 @self_difference() {
  %1 = sub i32 0, 0
  ret i32 %1
}
