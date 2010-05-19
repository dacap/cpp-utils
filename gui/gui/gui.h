// gui - Basic implementation of threads and GUI for C++          -*- C++ -*-
// Copyright (C) 2010 David Capello

#ifndef GUI_GUI_HEADER_FILE_INCLUDED
#define GUI_GUI_HEADER_FILE_INCLUDED

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400		// From Windows 2000
#endif

#include <windows.h>

#include <exception>

#include <gui/base.h>
#include <gui/thread.h>

namespace gui
{

  //////////////////////////////////////////////////////////////////////
  // win32_details namespace

  class win32_details
  {

    template<class WindowType>
    static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
      WindowType* wnd = (WindowType*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      if (wnd != NULL)
	wnd->process_message(msg, wparam, lparam);

      return DefWindowProc(hwnd, msg, wparam, lparam);
    }

  public:

    template<class WindowType>
    static void register_window_class()
    {
      HINSTANCE hinstance = ::GetModuleHandle(NULL);
      LPCTSTR class_name = "gui_window_class";
      WNDCLASSEX wcex;

      if (!::GetClassInfoEx(hinstance, class_name, &wcex)) {
	wcex.cbSize        = sizeof(WNDCLASSEX);
	wcex.style         = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc   = &wnd_proc<WindowType>;
	wcex.cbClsExtra    = 0;
	wcex.cbWndExtra    = 0;
	wcex.hInstance     = hinstance;
	wcex.hIcon         = (HICON)NULL;
	wcex.hCursor       = (HCURSOR)::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
	wcex.lpszMenuName  = (LPCTSTR)NULL;
	wcex.lpszClassName = class_name;
	wcex.hIconSm       = NULL;

	if (RegisterClassEx(&wcex) == 0)
	  //throw std::exception("Cannot register win32 window class");
	  throw std::exception();
      }
    }

    template<class WindowType>
    static HWND create_window(WindowType* wnd, int width, int height)
    {
      HINSTANCE hinstance = ::GetModuleHandle(NULL);
      HWND hwnd = CreateWindowEx(WS_EX_CONTROLPARENT,
				 "gui_window_class", "",
				 WS_OVERLAPPEDWINDOW,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 // CW_USEDEFAULT, CW_USEDEFAULT,
				 width, height,
				 (HWND)NULL,
				 (HMENU)NULL,
				 hinstance,
				 NULL);
      if (hwnd != NULL) {
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)wnd);
      }
      return hwnd;
    }


  };

  //////////////////////////////////////////////////////////////////////
  // window class

  class window
  {
    HWND m_handle;
    thread* m_thread;
    bool m_destroy;
    bool m_keypressed;
    bool m_closed;

    struct create_window_wrapper
    {
      window* m_wnd;
      int m_width;
      int m_height;
      create_window_wrapper(window* wnd, int width, int height)
	: m_wnd(wnd)
	, m_width(width)
	, m_height(height)
      {
      }
      void operator()()
      {
	m_wnd->m_handle = win32_details::create_window(m_wnd, m_width, m_height);

	MSG msg;

	::ShowWindow(m_wnd->m_handle, SW_SHOWNORMAL);
	::UpdateWindow(m_wnd->m_handle);

	while (!m_wnd->m_destroy &&
	       ::IsWindowVisible(m_wnd->m_handle) &&
	       ::GetMessage(&msg, m_wnd->m_handle, 0, 0)) {
	  ::TranslateMessage(&msg);
	  ::DispatchMessage(&msg);
	}

	::DestroyWindow(m_wnd->m_handle);
	m_wnd->m_handle = NULL;
      }
    };

    friend struct create_window_wrapper;

  public:

    window(int width, int height)
    {
      win32_details::register_window_class<window>();

      m_destroy = false;
      m_keypressed = false;
      m_closed = false;
      m_thread = new thread(create_window_wrapper(this, width, height));
      // m_thread = NULL;
      // create_window_wrapper(this, width, height)();

      // TODO wait m_handle != NULL
      while (m_handle == NULL)
      	;
    }

    ~window()
    {
      if (m_thread) {
	m_destroy = true;
	m_thread->join();
	delete m_thread;
      }
    }

    void waitkey()
    {
      // // do something
      while (!m_keypressed && !m_closed)
      	// wait_message();
	;
      m_keypressed = false;
    }
    
    void process_message(UINT msg, WPARAM wparam, LPARAM lparam)
    {
      if (msg == WM_KEYDOWN) {
	m_keypressed = true;
      }
      else if (msg == WM_CLOSE) {
	m_closed = true;
      }
    }

  };

}

#define gui_main()				\
  WINAPI WinMain(HINSTANCE hInstance,		\
		 HINSTANCE hPrevInstance,	\
		 LPSTR lpCmdLine,		\
		 int nCmdShow)			\

#endif // GUI_GUI_HEADER_FILE_INCLUDED
