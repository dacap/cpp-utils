// gui - Basic implementation of threads and GUI for C++          -*- C++ -*-
// Copyright (C) 2010 David Capello

#ifndef GUI_BASE_HEADER_FILE_INCLUDED
#define GUI_BASE_HEADER_FILE_INCLUDED

namespace gui
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


#endif // GUI_BASE_HEADER_FILE_INCLUDED
