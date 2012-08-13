// mt - thread library to create C++ experiments         -*- C++ -*-
// Copyright (C) 2010, 2012 David Capello
//
// Distributed under the terms of the New BSD License,
// see LICENSE.md for more details.

#ifndef MT_THREAD_HEADER_FILE_INCLUDED
#define MT_THREAD_HEADER_FILE_INCLUDED

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400             // From Windows 2000
#endif

#include <windows.h>

#include <cassert>
#include <string>
#include <exception>

namespace mt {

  //////////////////////////////////////////////////////////////////////
  // lock_guard class

  template<class Mutex>
  class lock_guard {
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

    // Non-copyable
    lock_guard(const lock_guard&);
    lock_guard& operator=(const lock_guard&);
  };

  //////////////////////////////////////////////////////////////////////
  // mutex class

  class mutex {
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

  private:
    CRITICAL_SECTION m_cs;

    // Non-copyable
    mutex(const mutex&);
    mutex& operator=(const mutex&);
  };

  //////////////////////////////////////////////////////////////////////
  // condition_variable class

  class condition_variable {
  public:
    condition_variable() {
      m_waiting_queue =
        CreateSemaphore(NULL,    // Security attributes
                        0,       // Initial count
                        1000,    // Maximum number of waiters (TODO)
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

  private:
    mutex m_monitor;            // To avoid running two condition_variable member function at the same time
    HANDLE m_waiting_queue;     // Queue of waiting threads
    LONG m_waiters;             // Number of waiters in the queue

    // Non-copyable
    condition_variable(const condition_variable&);
    condition_variable& operator=(const condition_variable&);
  };

  //////////////////////////////////////////////////////////////////////
  // ultra-simplistic thread class implementation based on C++0x

  class thread {
  public:
    class details;
    class id {
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
    struct f_wrapper0 {
      Callable f;
      f_wrapper0(const Callable& f) : f(f) { }
      void operator()() { f(); }
    };

    template<class Callable, class A>
    struct f_wrapper1 {
      Callable f;
      A a;
      f_wrapper1(const Callable& f, A a) : f(f), a(a) { }
      void operator()() { f(a); }
    };

    template<class Callable, class A, class B>
    struct f_wrapper2 {
      Callable f;
      A a;
      B b;
      f_wrapper2(const Callable& f, A a, B b) : f(f), a(a), b(b) { }
      void operator()() { f(a, b); }
    };

    template<class T>
    static DWORD WINAPI thread_proxy(LPVOID data) {
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
      , m_id() {
    }

    // Create a new thread without arguments
    template<class Callable>
    thread(const Callable& f) {
      m_native_handle =
        CreateThread(NULL, 0,
                     thread_proxy<f_wrapper0<Callable> >,
                     (LPVOID)new f_wrapper0<Callable>(f),
                     CREATE_SUSPENDED, &m_id.m_native_id);
      ResumeThread(m_native_handle);
    }

    // Create a new thread with one argument
    template<class Callable, class A>
    thread(const Callable& f, A a) {
      m_native_handle =
        CreateThread(NULL, 0,
                     thread_proxy<f_wrapper1<Callable, A> >,
                     (LPVOID)new f_wrapper1<Callable, A>(f, a),
                     CREATE_SUSPENDED, &m_id.m_native_id);
      ResumeThread(m_native_handle);
    }

    // Create a new thread with two arguments
    template<class Callable, class A, class B>
    thread(const Callable& f, A a, B b) {
      m_native_handle =
        CreateThread(NULL, 0,
                     thread_proxy<f_wrapper2<Callable, A, B> >,
                     (LPVOID)new f_wrapper2<Callable, A, B>(f, a, b),
                     CREATE_SUSPENDED, &m_id.m_native_id);
      ResumeThread(m_native_handle);
    }

    ~thread() {
      if (joinable())
        detach();
    }

    bool joinable() const {
      return
        m_native_handle != NULL &&
        m_id.m_native_id != ::GetCurrentThreadId();
    }

    void join() {
      if (joinable()) {
        ::WaitForSingleObject(m_native_handle, INFINITE);
        detach();
      }
    }

    void detach() {
      ::CloseHandle(m_native_handle);

      m_native_handle = NULL;
      m_id = id();
    }

    id get_id() const {
      return m_id;
    }

    native_handle_type native_handle() {
      return m_native_handle;
    }

    class details {
    public:
      static id get_current_thread_id() {
        return id(::GetCurrentThreadId());
      }
    };

  };

  //////////////////////////////////////////////////////////////////////
  // this_thread namespace

  namespace this_thread {

    inline thread::id get_id() {
      return thread::details::get_current_thread_id();
    }

    inline void yield() {
      ::Sleep(0);
    }

    // Simplified API (here we do not implement duration/time_point C++0x classes
    inline void sleep_for(int milliseconds) {
      ::Sleep(milliseconds);
    }

  } // namespace this_thread

  //////////////////////////////////////////////////////////////////////
  // thread_guard class

  class thread_guard {
  public:
    explicit thread_guard(thread& t) : m_thread(t) { }
    ~thread_guard() {
      if (m_thread.joinable())
        m_thread.join();
    }
  private:
    thread& m_thread;
  };

} // namespace mt

#endif // MT_THREAD_HEADER_FILE_INCLUDED
