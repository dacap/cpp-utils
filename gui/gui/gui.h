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

  namespace win32
  {

  //////////////////////////////////////////////////////////////////////
  // win32_details namespace

  class win32_details
  {

    template<class WindowType>
    static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
      WindowType* wnd = (WindowType*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      if (wnd != NULL) {
	LRESULT result;
	if (wnd->process_message(msg, wparam, lparam, result))
	  return result;
      }

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
	// Set the pointer in the user-data field of the window (TODO: use an ATOM)
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)wnd);
      }
      return hwnd;
    }


  };

  //////////////////////////////////////////////////////////////////////
  // win32_bitmap class

  class win32_bitmap
  {
    HBITMAP m_hbitmap;
    HDC m_hdc;

  public:

    win32_bitmap(int width, int height)
    {
      assert(width > 0 && height > 0);

      BITMAPINFOHEADER bhdr;
      bhdr.biSize = sizeof(BITMAPINFOHEADER);
      bhdr.biWidth = width;
      bhdr.biHeight = -height;
      bhdr.biPlanes = 1;
      bhdr.biBitCount = 32;	// 32 bpp
      bhdr.biCompression = BI_RGB;
      bhdr.biSizeImage = 0;
      bhdr.biXPelsPerMeter = 0;
      bhdr.biYPelsPerMeter = 0;
      bhdr.biClrUsed = 0;
      bhdr.biClrImportant = 0;

      BITMAPINFO binf;
      RGBQUAD dummy = { 0, 0, 0, 0 };
      binf.bmiColors[0] = dummy;
      binf.bmiHeader = bhdr;

      char* bits = NULL;

      HDC hdc = GetDC(GetDesktopWindow());
      m_hbitmap = CreateDIBSection(hdc, &binf, DIB_RGB_COLORS,
				  reinterpret_cast<void**>(&bits),
				  NULL, 0);

      if (m_hbitmap == NULL)
	throw std::exception(); // TODO throw gui_exception

      m_hdc = CreateCompatibleDC(hdc);
      SelectObject(m_hdc, m_hbitmap);

      // Clear the whole bitmap with a white background
      {
	HGDIOBJ oldPen   = SelectObject(m_hdc, ::CreatePen(PS_NULL, 0, 0));
	HGDIOBJ oldBrush = SelectObject(m_hdc, ::CreateSolidBrush(RGB(255, 255, 255)));

	Rectangle(m_hdc, 0, 0, width, height);

	DeleteObject(SelectObject(m_hdc, oldPen));
	DeleteObject(SelectObject(m_hdc, oldBrush));
      }
    }

    ~win32_bitmap()
    {
      ::DeleteDC(m_hdc);
      ::DeleteObject(m_hbitmap);
    }

    HBITMAP hbitmap()
    {
      return m_hbitmap;
    }

    HDC hdc()
    {
      return m_hdc;
    }

  };

  //////////////////////////////////////////////////////////////////////
  // win32_window class

  class win32_window
  {
    HWND m_handle;
    thread* m_thread;
    bool m_destroy;
    bool m_keypressed;
    bool m_closed;
    win32_bitmap m_bitmap;

    struct create_window_wrapper
    {
      win32_window* m_wnd;
      int m_width;
      int m_height;

      create_window_wrapper(win32_window* wnd, int width, int height)
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

	// Message loop while:
	// 1) the window is not destroyed
	// 2) the window is visible
	// 3) the Windows message queue is still alive
	while (!m_wnd->m_destroy &&
	       ::IsWindowVisible(m_wnd->m_handle) &&
	       ::GetMessage(&msg, m_wnd->m_handle, 0, 0)) {
	  ::TranslateMessage(&msg);
	  ::DispatchMessage(&msg);
	}

	// Destroy the window
	::DestroyWindow(m_wnd->m_handle);
	m_wnd->m_handle = NULL;
      }

    };

    friend struct create_window_wrapper;
    friend class win32_details;

  public:

    win32_window(int width, int height)
      : m_bitmap(width, height)
    {
      win32_details::register_window_class<win32_window>();

      m_destroy = false;
      m_keypressed = false;
      m_closed = false;
      m_thread = new thread(create_window_wrapper(this, width, height));

      // TODO wait m_handle != NULL (use a notification, avoid 100% CPU wait)
      while (m_handle == NULL)
      	;
    }

    ~win32_window()
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
      while (!m_keypressed && !m_closed) // TODO avoid 100% CPU wait
      	// wait_message();
	;
      m_keypressed = false;
    }

    size_t width()
    {
      RECT rc;
      ::GetWindowRect(m_handle, &rc);
      return rc.right - rc.left;
    }

    size_t height()
    {
      RECT rc;
      ::GetWindowRect(m_handle, &rc);
      return rc.bottom - rc.top;
    }

    void write(const std::string& text)
    {
      TextOut(m_bitmap.hdc(), 0, 0, text.c_str(), static_cast<int>(text.size()));

      ::InvalidateRect(m_handle, NULL, FALSE);
    }

  private:

    // This function is used to process
    bool process_message(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT& result)
    {
      switch (msg) {

	case WM_KEYDOWN:
	  m_keypressed = true;
	  break;

	case WM_CLOSE:
	  m_closed = true;
	  break;

	case WM_PAINT:
	  {
	    PAINTSTRUCT ps;
	    HDC window_hdc = ::BeginPaint(m_handle, &ps);
	    HDC bitmap_hdc = m_bitmap.hdc();

	    if (!::IsRectEmpty(&ps.rcPaint)) {
	      BitBlt(window_hdc,
		     ps.rcPaint.left, ps.rcPaint.top, // X, Y (dst)
		     ps.rcPaint.right - ps.rcPaint.left, // Width
		     ps.rcPaint.bottom - ps.rcPaint.top, // Height
		     bitmap_hdc,
		     ps.rcPaint.left, ps.rcPaint.top, // X, Y (src)
		     SRCCOPY);
	    }

	    ::EndPaint(m_handle, &ps);

	    result = TRUE;
	  }
	  return true;
      }
      return false;
    }

  };

  } // namespace win32

  typedef win32::win32_window window_impl;

  //////////////////////////////////////////////////////////////////////
  // window class

  class window : public non_copyable
  {
  public:

    window(int width, int height)
    {
      m_impl = new window_impl(width, height);
    }

    ~window()
    {
      delete m_impl;
    }

    size_t width()
    {
      return m_impl->width();
    }

    size_t height() 
    {
      return m_impl->height();
    }

    void waitkey()
    {
      m_impl->waitkey();
    }

    window& operator<<(const std::string& text)
    {
      m_impl->write(text);
      return *this;
    }

  private:
    window_impl* m_impl;
  };

}

#define gui_main()				\
  WINAPI WinMain(HINSTANCE hInstance,		\
		 HINSTANCE hPrevInstance,	\
		 LPSTR lpCmdLine,		\
		 int nCmdShow)			\

#endif // GUI_GUI_HEADER_FILE_INCLUDED
