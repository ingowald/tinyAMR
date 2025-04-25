#pragma once

// std
#include <sstream>
#include <string>
#include <string.h>
#include <mutex>
#include <stdexcept>
#include <set>
#include <map>
#include <vector>
#include <queue>
#ifdef __GNUC__
#include <execinfo.h>
#include <sys/time.h>
#endif

#define NOTIMPLEMENTED throw std::runtime_error(std::string(__PRETTY_FUNCTION__)+" not implemented")

#ifdef WIN32
# define TAMR_TERMINAL_RED ""
# define TAMR_TERMINAL_GREEN ""
# define TAMR_TERMINAL_YELLOW ""
# define TAMR_TERMINAL_BLUE ""
# define TAMR_TERMINAL_LIGHT_BLUE ""
# define TAMR_TERMINAL_RESET ""
# define TAMR_TERMINAL_DEFAULT TAMR_TERMINAL_RESET
# define TAMR_TERMINAL_LIGHT_GREEN ""
# define TAMR_TERMINAL_BOLD ""

# define TAMR_TERMINAL_MAGENTA ""
# define TAMR_TERMINAL_LIGHT_MAGENTA ""
# define TAMR_TERMINAL_CYAN ""
# define TAMR_TERMINAL_LIGHT_RED ""
#else
# define TAMR_TERMINAL_RED "\033[1;31m"
# define TAMR_TERMINAL_GREEN "\033[1;32m"
# define TAMR_TERMINAL_YELLOW "\033[1;33m"
# define TAMR_TERMINAL_BLUE "\033[1;34m"
# define TAMR_TERMINAL_LIGHT_BLUE "\033[1;34m"
# define TAMR_TERMINAL_LIGHT_GREEN "\033[1;32m"
# define TAMR_TERMINAL_RESET "\033[0m"
# define TAMR_TERMINAL_DEFAULT TAMR_TERMINAL_RESET
# define TAMR_TERMINAL_BOLD "\033[1;1m"

# define TAMR_TERMINAL_MAGENTA "\e[35m"
# define TAMR_TERMINAL_LIGHT_MAGENTA "\e[95m"
# define TAMR_TERMINAL_CYAN "\e[36m"
# define TAMR_TERMINAL_LIGHT_RED "\033[1;31m"
#endif

#if defined(_MSC_VER)
#  define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#ifndef PRINT
# define PRINT(var) std::cout << #var << "=" << var << std::endl << std::flush;
#ifdef __WIN32__
# define PING std::cout << __FILE__ << "::" << __LINE__ << ": " << __FUNCTION__ << std::endl << std::flush;
#else
# define PING std::cout << __FILE__ << "::" << __LINE__ << ": " << __PRETTY_FUNCTION__ << std::endl;
#endif
#endif



/*! for now, declare empty; will eventually have to set declspecs etc */
#define TAMR_INTERFACE /**/

#ifdef __CUDACC__
# define __tamr_host __host__
# define __tamr_device __device__
#else
# define __tamr_host /*nothing*/
# define __tamr_device /*nothing*/
#endif
#define __tamr_both __tamr_device __tamr_host

#ifdef _MSC_VER
# define TAMR_ALIGN(alignment) __declspec(align(alignment)) 
#else
# define TAMR_ALIGN(alignment) __attribute__((aligned(alignment)))
#endif


namespace tamr {
  namespace common {
#ifdef __WIN32__
#  define tamr_snprintf sprintf_s
#else
#  define tamr_snprintf snprintf
#endif
  
    /*! added pretty-print function for large numbers, printing 10000000 as "10M" instead */
    inline std::string prettyDouble(const double val) {
      const double absVal = abs(val);
      char result[1000];

      if      (absVal >= 1e+18f) tamr_snprintf(result,1000,"%.1f%c",float(val/1e18f),'E');
      else if (absVal >= 1e+15f) tamr_snprintf(result,1000,"%.1f%c",float(val/1e15f),'P');
      else if (absVal >= 1e+12f) tamr_snprintf(result,1000,"%.1f%c",float(val/1e12f),'T');
      else if (absVal >= 1e+09f) tamr_snprintf(result,1000,"%.1f%c",float(val/1e09f),'G');
      else if (absVal >= 1e+06f) tamr_snprintf(result,1000,"%.1f%c",float(val/1e06f),'M');
      else if (absVal >= 1e+03f) tamr_snprintf(result,1000,"%.1f%c",float(val/1e03f),'k');
      else if (absVal <= 1e-12f) tamr_snprintf(result,1000,"%.1f%c",float(val*1e15f),'f');
      else if (absVal <= 1e-09f) tamr_snprintf(result,1000,"%.1f%c",float(val*1e12f),'p');
      else if (absVal <= 1e-06f) tamr_snprintf(result,1000,"%.1f%c",float(val*1e09f),'n');
      else if (absVal <= 1e-03f) tamr_snprintf(result,1000,"%.1f%c",float(val*1e06f),'u');
      else if (absVal <= 1e-00f) tamr_snprintf(result,1000,"%.1f%c",float(val*1e03f),'m');
      else tamr_snprintf(result,1000,"%f",(float)val);

      return result;
    }
  

    /*! return a nicely formatted number as in "3.4M" instead of
        "3400000", etc, using mulitples of thousands (K), millions
        (M), etc. Ie, the value 64000 would be returned as 64K, and
        65536 would be 65.5K */
    inline std::string prettyNumber(const size_t s)
    {
      char buf[1000];
      if (s >= (1000LL*1000LL*1000LL*1000LL)) {
        tamr_snprintf(buf, 1000,"%.2fT",s/(1000.f*1000.f*1000.f*1000.f));
      } else if (s >= (1000LL*1000LL*1000LL)) {
        tamr_snprintf(buf, 1000, "%.2fG",s/(1000.f*1000.f*1000.f));
      } else if (s >= (1000LL*1000LL)) {
        tamr_snprintf(buf, 1000, "%.2fM",s/(1000.f*1000.f));
      } else if (s >= (1000LL)) {
        tamr_snprintf(buf, 1000, "%.2fK",s/(1000.f));
      } else {
        tamr_snprintf(buf,1000,"%zi",s);
      }
      return buf;
    }

    /*! return a nicely formatted number as in "3.4M" instead of
        "3400000", etc, using mulitples of 1024 as in kilobytes,
        etc. Ie, the value 65534 would be 64K, 64000 would be 63.8K */
    inline std::string prettyBytes(const size_t s)
    {
      char buf[1000];
      if (s >= (1024LL*1024LL*1024LL*1024LL)) {
        tamr_snprintf(buf, 1000,"%.2fT",s/(1024.f*1024.f*1024.f*1024.f));
      } else if (s >= (1024LL*1024LL*1024LL)) {
        tamr_snprintf(buf, 1000, "%.2fG",s/(1024.f*1024.f*1024.f));
      } else if (s >= (1024LL*1024LL)) {
        tamr_snprintf(buf, 1000, "%.2fM",s/(1024.f*1024.f));
      } else if (s >= (1024LL)) {
        tamr_snprintf(buf, 1000, "%.2fK",s/(1024.f));
      } else {
        tamr_snprintf(buf,1000,"%zi",s);
      }
      return buf;
    }
  
    inline double getCurrentTime()
    {
#ifdef _WIN32
      SYSTEMTIME tp; GetSystemTime(&tp);
      /*
         Please note: we are not handling the "leap year" issue.
     */
      size_t numSecsSince2020
          = tp.wSecond
          + (60ull) * tp.wMinute
          + (60ull * 60ull) * tp.wHour
          + (60ull * 60ul * 24ull) * tp.wDay
          + (60ull * 60ul * 24ull * 365ull) * (tp.wYear - 2020);
      return double(numSecsSince2020 + tp.wMilliseconds * 1e-3);
#else
      struct timeval tp; gettimeofday(&tp,nullptr);
      return double(tp.tv_sec) + double(tp.tv_usec)/1E6;
#endif
    }

    inline bool hasSuffix(const std::string &s, const std::string &suffix)
    {
      return s.substr(s.size()-suffix.size()) == suffix;
    }
    
  }
}

