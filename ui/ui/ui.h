// ui - Basic User Interface library to do experiments          -*- C++ -*-
// Copyright (C) 2010, 2012 David Capello
//
// Distributed under the terms of the New BSD License,
// see LICENSE.md for more details.

#ifndef UI_UI_HEADER_FILE_INCLUDED
#define UI_UI_HEADER_FILE_INCLUDED

#if WIN32
  #include "ui/win32.h"
#endif

namespace ui {

  //////////////////////////////////////////////////////////////////////
  // window class

  class window {
  public:
    window(int width, int height) {
      m_impl = new window_impl(width, height);
    }

    ~window() {
      delete m_impl;
    }

    size_t width() {
      return m_impl->width();
    }

    size_t height() {
      return m_impl->height();
    }

    void waitkey() {
      m_impl->waitkey();
    }

    window& operator<<(const std::string& text) {
      m_impl->write(text);
      return *this;
    }

  private:
    window_impl* m_impl;

    // Non-copyable
    window(const window&);
    window& operator=(const window&);
  };

} // namespace ui

#endif // UI_UI_HEADER_FILE_INCLUDED
