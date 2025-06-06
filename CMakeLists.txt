# ======================================================================== #
# Copyright 2025++ Ingo Wald                                               #
#                                                                          #
# Licensed under the Apache License, Version 2.0 (the "License");          #
# you may not use this file except in compliance with the License.         #
# You may obtain a copy of the License at                                  #
#                                                                          #
#     http://www.apache.org/licenses/LICENSE-2.0                           #
#                                                                          #
# Unless required by applicable law or agreed to in writing, software      #
# distributed under the License is distributed on an "AS IS" BASIS,        #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. #
# See the License for the specific language governing permissions and      #
# limitations under the License.                                           #
# ======================================================================== #

cmake_minimum_required(VERSION 3.22)

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()

project(TinyAMR VERSION 1.4.0 LANGUAGES C CXX)

if (NOT (${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_HOME_DIRECTORY}))
  SET(TINYAMR_IS_SUBMODULE ON)
else()
  SET(TINYAMR_IS_SUBMODULE OFF)
endif()

if (NOT TINYAMR_IS_SUBPROJECT)
  if(NOT SET_UP_CONFIGURATIONS_DONE)
    set(SET_UP_CONFIGURATIONS_DONE 1)

    # No reason to set CMAKE_CONFIGURATION_TYPES if it's not a multiconfig generator
    # Also no reason mess with CMAKE_BUILD_TYPE if it's a multiconfig generator.
    if(CMAKE_CONFIGURATION_TYPES) # multiconfig generator?
      set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE) 
    else()
      if(NOT CMAKE_BUILD_TYPE)
	#            message("Defaulting to release build.")
        set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
      endif()
      set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build")
      # set the valid options for cmake-gui drop-down list
      set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release")
    endif()
  endif()

  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()


# ------------------------------------------------------------------
# everything relating to umesh format/data/library
# -----------------------------------------------------------------
add_subdirectory(tinyAMR)

# ------------------------------------------------------------------
# converters into/out of umesh
# -----------------------------------------------------------------
add_subdirectory(importers)

# ------------------------------------------------------------------
# actual apps, including those that laucnch the converters
# -----------------------------------------------------------------
add_subdirectory(apps)


