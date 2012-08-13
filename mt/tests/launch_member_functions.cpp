// mt - thread library to create C++ experiments         -*- C++ -*-
// Copyright (C) 2010, 2012 David Capello
//
// Distributed under the terms of the New BSD License,
// see LICENSE.md for more details.

#include "mt/thread.h"

#include <iostream>

using std::cout;
using namespace mt;

class Test0 {
public:
  Test0() { cout << "Test0()\n"; }
  Test0(const Test0&) { cout << "Test0(const Test0&)\n"; }
  ~Test0() { cout << "~Test0()\n"; }
  void operator()() {
    cout << "["
         << this_thread::get_id().get_native_id()
         << "] func0()\n";
  }
};

class Test1 {
public:
  Test1() { cout << "Test1()\n"; }
  Test1(const Test1&) { cout << "Test1(const Test1&)\n"; }
  ~Test1() { cout << "~Test1()\n"; }
  void operator()(int a) {
    cout << "["
         << this_thread::get_id().get_native_id()
         << "] func1(" << a << ")\n";
  }
};

class Test2 {
public:
  Test2() { cout << "Test2()\n"; }
  Test2(const Test2&) { cout << "Test2(const Test2&)\n"; }
  ~Test2() { cout << "~Test2()\n"; }
  void operator()(int a, int b) {
    cout << "["
         << this_thread::get_id().get_native_id()
         << "] func2(" << a << ", " << b << ")\n";
  }
};

int main() {
  cout << "["
       << this_thread::get_id().get_native_id()
       << "] main\n";
  {
    thread a((Test0()));        // Very important the double parenthesis ((   ))
    thread b(Test1(), 1);
    thread c(Test1(), 2);
    thread d(Test2(), 1, 2);
    thread e(Test2(), 2, 3);

    thread_guard ga(a), gb(b), gc(c), gd(d), ge(e);
  }
  cout << "["
       << this_thread::get_id().get_native_id()
       << "] main exit\n";
}
