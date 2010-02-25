#include <cstdio>
#include "ui"

using namespace ui;

void func0()
{
  printf("[%08x] func0()\n", this_thread::get_id());
}

void func1(int a)
{
  printf("[%08x] func1(%d)\n", this_thread::get_id(), a);
}

void func2(int a, int b)
{
  printf("[%08x] func2(%d, %d)\n", this_thread::get_id(), a, b);
}

int ui_main()
{
  printf("[%08x] main\n", this_thread::get_id());

  thread a(&func0);
  thread b(&func1, 1);
  thread c(&func1, 2);
  thread d(&func2, 1, 2);
  thread e(&func2, 2, 3);

  thread_guard ga(a), gb(b), gc(c), gd(d), ge(e);

  window w(256, 256);

  w.waitkey();

  return 0;
}

