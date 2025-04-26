// ======================================================================== //
// Copyright 2025++ Ingo Wald                                               //
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

#include "tinyAMR/common.h"
#include <vector>
#include <memory>

namespace tamr {

  struct Model {
    typedef std::shared_ptr<Model> SP;
    
    struct Grid {
      /*! origin of this grid, on the logical grid of this grid's
          respective level. Note these coordinates will typically need
          to be mulitples of the respective level's refinement level
          to make sense. */
      vec3i    origin;
      
      /*! dimensions of this grid's 3D array of cells. The Nx*Ny*Nz
          scalars for this grid iwill be stored at
          scalars[grid.offset] in z-major order */
      vec3i    dims;
      
      /*! level of this grid, *relative to Model::refinementLevel* --
        i.e., the refinement level (and thus, size) or a cell in this
        grid is *NOT* necessarily 1/2^grid.level (or
        1/refinement^grid.level), but it is whatever
        Model::refinementOfLevel[grid.level] specifies. */
      int      level;
      
      /*! any user-specified 32-bit int; this library will not assign
          any meaning to this value, nor will it even modify it */
      uint32_t user;
      
      /* offset into the given scalar field's scalars[] array. Offsets
         are counted in *scalars*, not bytes. */
      uint64_t offset;
    };

    struct FieldMeta {
      /*! logical name of that field; it is up to the user to know
          what might or might not mean */
      std::string name = "<undefined>";
      
      /*! number of dimensions; usually '1' but some codes can
          generate 3D fields */
      int         numDimensions = 1;

      /*! offset into the Model::scalars[] array. If this field
          contains more than one dimension the different dimensions
          are supposed to be stored consecutively in that array, so
          the first dimension is at FieldMeta::offset, the second at
          FieldMeta::offset+Model::scalarsPerLevel, etc. Offsets are
          counted in *scalars*, not bytes. */
      uint64_t    offset = 0;
      
      /*! any additional meta info; it is up to the user to know what
          might or might not mean. Ie, an app _might_ write some XML
          description of additional semantical information in here,
          but it is not this library that will define or standardize
          what any such string may or may not mean */
      std::string info = "<undefined>";
    };

    void save(const std::string &fileName) const;
    
    static Model::SP load(const std::string &fileName);
    
    /*! must be one int per level; a value of 'i' means that the
        respective level's cells are a 2^i refinement of the unit
        cell, so each cell on that level is 2^{-i} wide */
    std::vector<int>       refinementOfLevel;
    
    /*! array of all scalars, across all grids, across all scalar
        fields */
    std::vector<float>     scalars;
    std::vector<FieldMeta> fieldMetas;
    std::vector<Grid>      grids;
    
    /*! total number of cells/scalar values across all bricks; if
        there's a single, one-dimensional scalar field that is the
        same as scalars.size(); otherwise, each field will have
        exactly as many scalar values */
    uint64_t    numCellsAcrossAllGrids;
    
    /*! meta information that the app or importer may attach; this
        library makes no claims at what this field may or may not
        contain */
    std::string userMeta;
  };
  
} // ::tamr
