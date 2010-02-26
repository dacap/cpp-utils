// ui - Basic implementation of threads and GUI for C++          -*- C++ -*-
// Copyright (C) 2010 David Capello

#ifndef UI_HEADER_FILE_INCLUDED
#define UI_HEADER_FILE_INCLUDED

#include <cassert>
#include <windows.h>
#include <string>
#include <exception>

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

  //////////////////////////////////////////////////////////////////////
  // lock_guard class

  template<class Mutex>
  class lock_guard : public non_copyable
  {
  public:
    typedef Mutex mutex_type;

    explicit lock_guard(mutex_type& mutex) : m_mutex(mutex) {
      m_mutex.lock();
    }

    ~lock_guard() {
      m_mutex.unlock();
    }

    void lock() {
      m_mutex.lock();
    }

    void unlock() {
      m_mutex.unlock();
    }

  private:
    mutex_type& m_mutex;

  };

  //////////////////////////////////////////////////////////////////////
  // mutex class

  class mutex : public non_copyable
  {
    CRITICAL_SECTION m_cs;

  public:

    mutex() {
      InitializeCriticalSection(&m_cs);
    }

    ~mutex() {
      DeleteCriticalSection(&m_cs);
    }

    void lock() {
      EnterCriticalSection(&m_cs);
    }

    bool try_lock() {
      return TryEnterCriticalSection(&m_cs) ? true: false;
    }

    void unlock() {
      LeaveCriticalSection(&m_cs);
    }

  };

  //////////////////////////////////////////////////////////////////////
  // condition_variable class

  class condition_variable : public non_copyable
  {
    mutex m_monitor;		// To avoid running two condition_variable member function at the same time
    HANDLE m_waiting_queue;	// Queue of waiting threads
    LONG m_waiters;		// Number of waiters in the queue

  public:

    condition_variable() {
      m_waiting_queue =
	CreateSemaphore(NULL,	 // Security attributes
			0,	 // Initial count
			1000,	 // Maximum number of waiters (TODO)
			NULL); // Unnamed semaphore
      m_waiters = 0;
    }

    ~condition_variable() {
      // Here we could call notify_all, but, if a condition variable
      // is destroyed, "there shall be no thread blocked on *this"
      // (see 30.5.1 C++ working draft n3035)
      assert(m_waiters == 0);

      CloseHandle(m_waiting_queue);
    }

    void wait(lock_guard<mutex>& external_monitor) {
      {
	lock_guard<mutex> lock(m_monitor);

	assert(m_waiters >= 0);

	++m_waiters;
	external_monitor.unlock();
      }
      ::WaitForSingleObject(m_waiting_queue, INFINITE);
      external_monitor.lock();
    }

    void notify_one() {
      lock_guard<mutex> lock(m_monitor);

      assert(m_waiters >= 0);

      // If there are one or more waiters...
      if (m_waiters > 0) {
	// Increment semaphore count to unlock a waiting thread
	ReleaseSemaphore(m_waiting_queue, 1, NULL);
	--m_waiters;
      }
    }

    void notify_all() {
      lock_guard<mutex> lock(m_monitor);

      assert(m_waiters >= 0);

      // If there are one or more waiters...
      if (m_waiters > 0) {
	// Increment the semaphore to the number of waiting threads
	ReleaseSemaphore(m_waiting_queue, m_waiters, NULL);
	m_waiters = 0;
      }
    }
    
  };

  //////////////////////////////////////////////////////////////////////
  // ultra-simplistic thread class implementation based on C++0x

  class thread
  {
  public:
    class details;
    class id
    {
      friend class thread;
      friend class details;

      DWORD m_native_id;
      id(DWORD id) : m_native_id(id) { }
    public:
      id() : m_native_id(0) { }
      bool operator==(const id& y) const { return m_native_id == y.m_native_id; }
      bool operator!=(const id& y) const { return m_native_id != y.m_native_id; }
      bool operator< (const id& y) const { return m_native_id <  y.m_native_id; }
      bool operator<=(const id& y) const { return m_native_id <= y.m_native_id; }
      bool operator> (const id& y) const { return m_native_id >  y.m_native_id; }
      bool operator>=(const id& y) const { return m_native_id >= y.m_native_id; }

      // TODO should we replace this with support for iostreams?
      DWORD get_native_id() { return m_native_id; }
    };

    typedef HANDLE native_handle_type;

  private:

    template<class Callable>
    struct f_wrapper0
    {
      Callable f;
      f_wrapper0(Callable f) : f(f) { }
      void operator()() { f(); }
    };

    template<class Callable, class A>
    struct f_wrapper1
    {
      Callable f;
      A a;
      f_wrapper1(Callable f, A a) : f(f), a(a) { }
      void operator()() { f(a); }
    };

    template<class Callable, class A, class B>
    struct f_wrapper2
    {
      Callable f;
      A a;
      B b;
      f_wrapper2(Callable f, A a, B b) : f(f), a(a), b(b) { }
      void operator()() { f(a, b); }
    };

    template<class T>
    static DWORD WINAPI thread_proxy(LPVOID data)
    {
      T* t = (T*)data;
      (*t)();
      delete t;
      return 0;
    }

    native_handle_type m_native_handle;
    id m_id;

  public:

    // Create an instance to represent the current thread
    thread()
      : m_native_handle(NULL)
      , m_id()
    {
    }

    // Create a new thread without arguments
    template<class Callable>
    thread(Callable f)
    {
      m_native_handle =
	CreateThread(NULL, 0,
		     thread_proxy<f_wrapper0<Callable> >,
		     (LPVOID)new f_wrapper0<Callable>(f),
		     CREATE_SUSPENDED, &m_id.m_native_id);
      ResumeThread(m_native_handle);
    }

    // Create a new thread with one argument
    template<class Callable, class A>
    thread(Callable f, A a)
    {
      m_native_handle =
	CreateThread(NULL, 0,
		     thread_proxy<f_wrapper1<Callable, A> >,
		     (LPVOID)new f_wrapper1<Callable, A>(f, a),
		     CREATE_SUSPENDED, &m_id.m_native_id);
      ResumeThread(m_native_handle);
    }

    // Create a new thread with two arguments
    template<class Callable, class A, class B>
    thread(Callable f, A a, B b)
    {
      m_native_handle =
	CreateThread(NULL, 0,
		     thread_proxy<f_wrapper2<Callable, A, B> >,
		     (LPVOID)new f_wrapper2<Callable, A, B>(f, a, b),
		     CREATE_SUSPENDED, &m_id.m_native_id);
      ResumeThread(m_native_handle);
    }

    ~thread()
    {
      if (joinable())
	detach();
    }

    bool joinable() const
    {
      return 
	m_native_handle != NULL &&
	m_id.m_native_id != ::GetCurrentThreadId();
    }

    void join()
    {
      if (joinable()) {
	::WaitForSingleObject(m_native_handle, INFINITE);
	detach();
      }
    }

    void detach()
    {
      ::CloseHandle(m_native_handle);

      m_native_handle = NULL;
      m_id = id();
    }

    id get_id() const
    {
      return m_id;
    }

    native_handle_type native_handle()
    {
      return m_native_handle;
    }

    class details
    {
    public:
      static id get_current_thread_id()
      {
	return id(::GetCurrentThreadId());
      }
    };

  };

  //////////////////////////////////////////////////////////////////////
  // this_thread namespace

  namespace this_thread
  {
    inline thread::id get_id()
    {
      return thread::details::get_current_thread_id();
    }

    inline void yield()
    {
      ::Sleep(0);
    }

    // Simplified API (here we do not implement duration/time_point C++0x classes
    inline void sleep_for(int milliseconds)
    {
      ::Sleep(milliseconds);
    }
  }

  //////////////////////////////////////////////////////////////////////
  // thread_guard class

  class thread_guard 
  {
    thread& m_thread;
  public:
    explicit thread_guard(thread& t) : m_thread(t) { }
    ~thread_guard()
    {
      if (m_thread.joinable())
	m_thread.join();
    }
  };

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
      LPCTSTR class_name = "ui_window_class";
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
				 "ui_window_class", "",
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

#define ui_main()				\
  WINAPI WinMain(HINSTANCE hInstance,		\
		 HINSTANCE hPrevInstance,	\
		 LPSTR lpCmdLine,		\
		 int nCmdShow)			\


#endif // UI_HEADER_FILE_INCLUDED
