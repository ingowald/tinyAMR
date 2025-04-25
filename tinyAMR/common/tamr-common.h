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


