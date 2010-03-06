#include <cstdio>
#include "gui/ui.h"

using namespace ui;

void func0()
{
  printf("[%08x] func0()\n", this_thread::get_id().get_native_id());
}

void func1(int a)
{
  printf("[%08x] func1(%d)\n", this_thread::get_id().get_native_id(), a);
}

void func2(int a, int b)
{
  printf("[%08x] func2(%d, %d)\n", this_thread::get_id().get_native_id(), a, b);
}

int main()
{
  printf("[%08x] main\n", this_thread::get_id().get_native_id());
  {
    thread a(&func0);
    thread b(&func1, 1);
    thread c(&func1, 2);
    thread d(&func2, 1, 2);
    thread e(&func2, 2, 3);

    thread_guard ga(a), gb(b), gc(c), gd(d), ge(e);
  }
  printf("[%08x] main exit\n", this_thread::get_id().get_native_id());
  return 0;
}

