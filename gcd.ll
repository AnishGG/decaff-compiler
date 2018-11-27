; ModuleID = 'Program'
target triple = "x86_64-pc-linux-gnu"

@string = internal constant [10 x i8] c"gcd is %d\00"

define void @main() {
entry:
  call void @main1()
  ret void
}

define internal i64 @gcd(i64 %x, i64 %y) {
point:
  %x1 = alloca i64
  store i64 %x, i64* %x1
  %y2 = alloca i64
  store i64 %y, i64* %y2
  %tmp = load i64* %x1
  %tmp3 = load i64* %y2
  %tmp4 = icmp eq i64 %tmp, %tmp3
  %zext = zext i1 %tmp4 to i64
  %tmp5 = icmp ne i64 %zext, 0
  br i1 %tmp5, label %ifBlock, label %elseBlock

mergeBlock:                                       ; preds = %mergeBlock7
  ret i64 0

ifBlock:                                          ; preds = %point
  %tmp6 = load i64* %x1
  ret i64 %tmp6

elseBlock:                                        ; preds = %point
  %tmp9 = load i64* %x1
  %tmp10 = load i64* %y2
  %tmp11 = icmp slt i64 %tmp9, %tmp10
  %zext12 = zext i1 %tmp11 to i64
  %tmp13 = icmp ne i64 %zext12, 0
  br i1 %tmp13, label %ifBlock8, label %elseBlock18

mergeBlock7:                                      ; No predecessors!
  br label %mergeBlock

ifBlock8:                                         ; preds = %elseBlock
  %tmp14 = load i64* %x1
  %tmp15 = load i64* %y2
  %tmp16 = load i64* %x1
  %tmp17 = sub i64 %tmp15, %tmp16
  %gcd = call i64 @gcd(i64 %tmp14, i64 %tmp17)
  ret i64 %gcd

elseBlock18:                                      ; preds = %elseBlock
  %tmp19 = load i64* %x1
  %tmp20 = load i64* %y2
  %tmp21 = sub i64 %tmp19, %tmp20
  %tmp22 = load i64* %y2
  %gcd23 = call i64 @gcd(i64 %tmp21, i64 %tmp22)
  ret i64 %gcd23
}

define internal void @main1() {
point:
  %a = alloca i64
  store i64 0, i64* %a
  %gcd = call i64 @gcd(i64 6, i64 12)
  store i64 %gcd, i64* %a
  %tmp = load i64* %a
  %printf = call i64 (...)* @printf([10 x i8]* @string, i64 %tmp)
  ret void
}

declare i64 @printf(...)
