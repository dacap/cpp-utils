#ifndef CHRONO_H_INCLUDED
#define CHRONO_H_INCLUDED

//////////////////////////////////////////////////////////////////////
// Chrono class
//
// class Chrono
// {
// public:
//   Chrono();
//   void reset();
//   double elapsed() const;
// };

//////////////////////////////////////////////////////////////////////
//  For Windows

#ifdef _WIN32

  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>

  class Chrono
  {
    LARGE_INTEGER m_point;
    LARGE_INTEGER m_freq;

  public:

    Chrono() {
      QueryPerformanceFrequency(&m_freq);
      reset();
    }

    void reset() {
      QueryPerformanceCounter(&m_point);
    }

    double elapsed() const {
      LARGE_INTEGER now;
      QueryPerformanceCounter(&now);
      return static_cast<double>(now.QuadPart - m_point.QuadPart)
	/ static_cast<double>(m_freq.QuadPart);
    }

  };

#else  // For UNIX like

  #include <time.h>
  #include <sys/time.h>

  class Chrono
  {
    struct timeval m_point;

  public:

    Chrono() {
      reset();
    }

    void reset() {
      gettimeofday(&m_point, NULL);
    }

    double elapsed() const {
      struct timeval now;
      gettimeofday(&now, NULL);
      return
	(double)(now.tv_sec + (double)now.tv_usec/1000000) -
	(double)(m_point.tv_sec + (double)m_point.tv_usec/1000000);
    }

  };

#endif

#endif // CHRONO_H_INCLUDED
