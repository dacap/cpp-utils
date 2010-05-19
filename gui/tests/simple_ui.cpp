#include <gui/gui.h>
#include <iostream>

using namespace gui;

int gui_main()
{
  window w(256, 256);
  w << "Hello World!";
  w.waitkey();
  return 0;
}

