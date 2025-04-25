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

/*! this library can be built in two ways; either with the 'owl'
    library available to this library, or without. In the first case,
    miniScene uses the owl::common classes for vec3f, affine3f, etc,
    and just imports those into the mini namespace; without owl, it
    includes 'its own' copy of the relevant header files, which are
    basically a copy of the owl ones (at a given point in time),
    frozen in time, and put into another namespace to avoid namespace
    conflicts if used as a submodule. This flag gets passed down from
    the CMakeLists.txt. */
#ifdef OWL_DLL_EXPORT
# define TAMR_HAVE_OWL_COMMON 1
#else
// do NOT define TAMR_HAVE_OWL_COMMON
#endif

#if TAMR_HAVE_OWL_COMMON
// owl is already included, use owl box and vec types
#  include "owl/common/box.h"
#else
#  include "tinyAMR/common/box.h"
#endif

namespace tamr {
  
#if TAMR_HAVE_OWL_COMMON
  using namespace owl;
  namespace common {
    using namespace owl::common;
  }
#endif
  using namespace tamr::common;
  
  namespace common {
    
    inline bool endsWith(const std::string &s, const std::string &suffix)
    {
      return s.substr(s.size()-suffix.size(),suffix.size()) == suffix;
    }

  }
} // ::tamr

