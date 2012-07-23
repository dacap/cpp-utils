// ui - Basic User Interface library to do experiments          -*- C++ -*-
// Copyright (C) 2010, 2012 David Capello
//
// Distributed under the terms of the New BSD License,
// see LICENSE.md for more details.

#ifndef UI_BASE_HEADER_FILE_INCLUDED
#define UI_BASE_HEADER_FILE_INCLUDED

namespace ui
{

  //////////////////////////////////////////////////////////////////////
  // non_copyable class

  class non_copyable
  {
    non_copyable(const non_copyable&);
    non_copyable& operator=(const non_copyable&);
  public:
    non_copyable() { }
    ~non_copyable() { }
  };

}


#endif // UI_BASE_HEADER_FILE_INCLUDED
