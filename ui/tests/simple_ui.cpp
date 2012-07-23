// ui - Basic User Interface library to do experiments          -*- C++ -*-
// Copyright (C) 2010, 2012 David Capello
//
// Distributed under the terms of the New BSD License,
// see LICENSE.md for more details.

#include <ui/ui.h>
#include <iostream>

using namespace ui;

int ui_main()
{
  window w(256, 256);
  w << "Hello World!";
  w .waitkey();
  return 0;
}

