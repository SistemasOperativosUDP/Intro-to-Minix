// RUN: %clang_cc1 -emit-llvm %s -o %t
// RUN: FileCheck %s -input-file=%t -check-prefix=CHECK-GLOBALS
// RUN: FileCheck %s -input-file=%t -check-prefix=CHECK-1
// RUN: FileCheck %s -input-file=%t -check-prefix=CHECK-2
// RUN: FileCheck %s -input-file=%t -check-prefix=CHECK-3

int foo();
int global;

// Single statement
void test1() {
  int i = 0;
  #pragma clang __debug captured
  {
    i++;
  }
  // CHECK-1: %struct.anon = type { i32* }
  //
  // CHECK-1: test1
  // CHECK-1: alloca %struct.anon
  // CHECK-1: getelementptr inbounds %struct.anon*
  // CHECK-1: store i32* %i
  // CHECK-1: call void @[[HelperName:__captured_stmt[0-9]+]]
}

// CHECK-1: define internal void @[[HelperName]](%struct.anon
// CHECK-1:   getelementptr inbounds %struct.anon{{.*}}, i32 0, i32 0
// CHECK-1:   load i32**
// CHECK-1:   load i32*
// CHECK-1:   add nsw i32
// CHECK-1:   store i32

// Compound statement with local variable
void test2(int x) {
  #pragma clang __debug captured
  {
    int i;
    for (i = 0; i < x; i++)
      foo();
  }
  // CHECK-2: test2
  // CHECK-2-NOT: %i
  // CHECK-2: call void @[[HelperName:__captured_stmt[0-9]+]]
}

// CHECK-2: define internal void @[[HelperName]]
// CHECK-2-NOT: }
// CHECK-2:   %i = alloca i32

// Capture array
void test3() {
  int arr[] = {1, 2, 3, 4, 5};
  #pragma clang __debug captured
  {
    arr[2] = arr[1];
  }
  // CHECK-3: test3
  // CHECK-3: alloca [5 x i32]
  // CHECK-3: call void @__captured_stmt
}

void dont_capture_global() {
  static int s;
  extern int e;
  #pragma clang __debug captured
  {
    global++;
    s++;
    e++;
  }

  // CHECK-GLOBALS: %[[Capture:struct\.anon[\.0-9]*]] = type {}
  // CHECK-GLOBALS: call void @__captured_stmt[[HelperName:[0-9]+]](%[[Capture]]
}

// CHECK-GLOBALS: define internal void @__captured_stmt[[HelperName]]
// CHECK-GLOBALS-NOT: ret
// CHECK-GLOBALS:   load i32* @global
// CHECK-GLOBALS:   load i32* @
// CHECK-GLOBALS:   load i32* @e
