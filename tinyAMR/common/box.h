// ======================================================================== //
// Copyright 2018-2022 Ingo Wald                                            //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "vec.h"
#include <climits>
#include <cmath>

namespace tamr {
  namespace common {

    template<typename T> inline __tamr_both T empty_bounds_lower();
    template<typename T> inline __tamr_both T empty_bounds_upper();
    
    template<> inline __tamr_both uint32_t empty_bounds_lower<uint32_t>() 
    { return uint32_t(UINT_MAX); }
    template<> inline __tamr_both uint32_t empty_bounds_upper<uint32_t>() 
    { return uint32_t(0); }

    template<> inline __tamr_both int32_t empty_bounds_lower<int32_t>() 
    { return int32_t(INT_MAX); }
    template<> inline __tamr_both int32_t empty_bounds_upper<int32_t>() 
    { return int32_t(INT_MIN); }

    template<> inline __tamr_both uint64_t empty_bounds_lower<uint64_t>() 
    { return uint64_t(ULONG_MAX); }
    template<> inline __tamr_both uint64_t empty_bounds_upper<uint64_t>() 
    { return uint64_t(0); }

    template<> inline __tamr_both int64_t empty_bounds_lower<int64_t>() 
    { return int64_t(LONG_MAX); }
    template<> inline __tamr_both int64_t empty_bounds_upper<int64_t>() 
    { return int64_t(LONG_MIN); }

    template<> inline __tamr_both uint16_t empty_bounds_lower<uint16_t>() 
    { return uint16_t(USHRT_MAX); }
    template<> inline __tamr_both uint16_t empty_bounds_upper<uint16_t>() 
    { return uint16_t(0); }

    template<> inline __tamr_both int16_t empty_bounds_lower<int16_t>() 
    { return int16_t(SHRT_MAX); }
    template<> inline __tamr_both int16_t empty_bounds_upper<int16_t>() 
    { return int16_t(SHRT_MIN); }

    template<> inline __tamr_both uint8_t empty_bounds_lower<uint8_t>() 
    { return uint8_t(CHAR_MAX); }
    template<> inline __tamr_both uint8_t empty_bounds_upper<uint8_t>() 
    { return uint8_t(CHAR_MIN); }

    template<> inline __tamr_both int8_t empty_bounds_lower<int8_t>() 
    { return int8_t(SCHAR_MIN); }
    template<> inline __tamr_both int8_t empty_bounds_upper<int8_t>() 
    { return int8_t(SCHAR_MAX); }
    
    template<typename T>
    struct interval {
      typedef T scalar_t;
      inline __tamr_both interval() 
        : lower(empty_bounds_lower<T>()),
          upper(empty_bounds_upper<T>())
      {}
      inline __tamr_both interval(T begin, T end) : lower(begin), upper(end) {}
      
      T lower;
      T upper;
      
      inline __tamr_both bool contains(const T &t) const { return t >= lower && t <= upper; }
      inline __tamr_both bool empty() const { return lower > upper; }
      inline __tamr_both T center() const { return (lower+upper)/2; }
      inline __tamr_both T span() const { return upper - lower; }
      inline __tamr_both T diagonal() const { return upper - lower; }
      inline __tamr_both interval<T> &extend(const T &t)
      { lower = min(lower,t); upper = max(upper,t); return *this; }
      inline __tamr_both interval<T> &extend(const interval<T> &t)
      { lower = min(lower,t.lower); upper = max(upper,t.upper); return *this; }
      inline __tamr_both interval<T> including(const T &t)
      { return interval<T>(min(lower,t),max(upper,t)); }
    };

    template<typename T>
    inline __tamr_both std::ostream &operator<<(std::ostream &o, const interval<T> &b)
    {
#ifndef __CUDA_ARCH__
      o << "[" << b.lower << ":" << b.upper << "]";
#endif
      return o;
    }
  
    template<typename T>
    inline __tamr_both interval<T> build_interval(const T &a, const T &b)
    { return interval<T>(min(a,b),max(a,b)); }
  
    template<typename T>
    inline __tamr_both interval<T> intersect(const interval<T> &a, const interval<T> &b)
    { return interval<T>(max(a.lower,b.lower),min(a.upper,b.upper)); }
  
    template<typename T>
    inline __tamr_both interval<T> operator-(const interval<T> &a, const T &b)
    { return interval<T>(a.lower-b,a.upper-b); }

    template<typename T>
    inline __tamr_both interval<T> operator*(const interval<T> &a, const T &b)
    { return build_interval<T>(a.lower*b,a.upper*b); }
  
    template<typename T>
    inline __tamr_both bool operator==(const interval<T> &a, const interval<T> &b)
    { return a.lower == b.lower && a.upper == b.upper; }
  
    template<typename T>
    inline __tamr_both bool operator!=(const interval<T> &a, const interval<T> &b)
    { return !(a == b); }

    template<typename T>
    struct box_t {
      typedef T vec_t;
      typedef typename T::scalar_t scalar_t;
      enum { dims = T::dims };

      inline __tamr_both box_t()
        : lower(empty_bounds_lower<typename T::scalar_t>()),
          upper(empty_bounds_upper<typename T::scalar_t>())
      {}

      /*! construct a new box around a single point */
      explicit inline __tamr_both box_t(const vec_t &v)
        : lower(v),
          upper(v)
      {}

      /*! construct a new, origin-oriented box of given size */
      inline __tamr_both box_t(const vec_t &lo, const vec_t &hi)
        : lower(lo),
          upper(hi)
      {}

      /*! returns new box including both ourselves _and_ the given point */
      inline __tamr_both box_t including(const vec_t &other) const
      { return box_t(min(lower,other),max(upper,other)); }
      /*! returns new box including both ourselves _and_ the given point */
      inline __tamr_both box_t including(const box_t &other) const
      { return box_t(min(lower,other.lower),max(upper,other.upper)); }

    
      /*! returns new box including both ourselves _and_ the given point */
      inline __tamr_both box_t &extend(const vec_t &other) 
      { lower = min(lower,other); upper = max(upper,other); return *this; }
      /*! returns new box including both ourselves _and_ the given point */
      inline __tamr_both box_t &extend(const box_t &other) 
      { lower = min(lower,other.lower); upper = max(upper,other.upper); return *this; }
    
    
      /*! get the d-th dimensional slab (lo[dim]..hi[dim] */
      inline __tamr_both interval<scalar_t> get_slab(const uint32_t dim)
      {
        return interval<scalar_t>(lower[dim],upper[dim]);
      }
    
      inline __tamr_both bool contains(const vec_t &point) const
      { return !(any_less_than(point,lower) || any_greater_than(point,upper)); }

      inline __tamr_both bool overlaps(const box_t &other) const
      { return !(any_less_than(other.upper,lower) || any_greater_than(other.lower,upper)); }

      inline __tamr_both vec_t center() const { return (lower+upper)/(typename vec_t::scalar_t)2; }
      inline __tamr_both vec_t span()   const { return upper-lower; }
      inline __tamr_both vec_t size()   const { return upper-lower; }

      inline __tamr_both T volume() const
      { return volume(size()); }
    
      inline __tamr_both bool empty() const { return any_less_than(upper,lower); }

      vec_t lower, upper;
    };
  
    // =======================================================
    // default functions
    // =======================================================

    template<typename T>
    inline __tamr_both T area(const box_t<vec_t<T,2>> &b)
    { return area(b.upper - b.lower); }

    template<typename T>
    inline __tamr_both T area(const box_t<vec_t<T,3>> &b)
    {
      const vec_t<T,3> diag = b.upper - b.lower;
      return 2.f*(area(vec_t<T,2>(diag.x,diag.y))+
                  area(vec_t<T,2>(diag.y,diag.z))+
                  area(vec_t<T,2>(diag.z,diag.x)));
    }

    template<typename T>
    inline __tamr_both T volume(const box_t<vec_t<T,3>> &b)
    {
      const vec_t<T,3> diag = b.upper - b.lower;
      return diag.x*diag.y*diag.z;
    }

    template<typename T>
    inline __tamr_both std::ostream &operator<<(std::ostream &o, const box_t<T> &b)
    {
#ifndef __CUDA_ARCH__
      o << "[" << b.lower << ":" << b.upper << "]";
#endif
      return o;
    }

    template<typename T>
    inline __tamr_both box_t<T> intersection(const box_t<T> &a, const box_t<T> &b)
    { return box_t<T>(max(a.lower,b.lower),min(a.upper,b.upper)); }
  
    template<typename T>
    inline __tamr_both bool operator==(const box_t<T> &a, const box_t<T> &b)
    { return a.lower == b.lower && a.upper == b.upper; }
  
    template<typename T>
    inline __tamr_both bool operator!=(const box_t<T> &a, const box_t<T> &b)
    { return !(a == b); }


    
  
    // =======================================================
    // default instantiations
    // =======================================================
  
#define _define_box_types(T,t)                  \
    typedef box_t<vec_t<T,2>> box2##t;          \
    typedef box_t<vec_t<T,3>> box3##t;          \
    typedef box_t<vec_t<T,4>> box4##t;          \
    typedef box_t<vec3a_t<T>> box3##t##a;       \
  
    _define_box_types(bool ,b);
    _define_box_types(int8_t ,c);
    _define_box_types(int16_t ,s);
    _define_box_types(int32_t ,i);
    _define_box_types(int64_t ,l);
    _define_box_types(uint8_t ,uc);
    _define_box_types(uint16_t,us);
    _define_box_types(uint32_t,ui);
    _define_box_types(uint64_t,ul);
    _define_box_types(float,f);
    _define_box_types(double,d);
  
#undef _define_box_types

  } // ::tamr::common
} // ::owl
