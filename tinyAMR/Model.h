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

#define TAMR_VERSION_MAJOR 1
#define TAMR_VERSION_MINOS 1
#define TAMR_VERSION_PATCH 0

namespace tamr {

  struct Model {
    typedef std::shared_ptr<Model> SP;
    struct FieldMeta;
    
    struct Grid {
      //! number of all cell values for this grid _in_cluding ghost layers
      int numTotalCells() const;
      //! number of actual data cells, as given by 'dims', _ex_cluding ghosts
      int numInnerCells() const;
      /*! extract (only) the inner cells, and store them at the given
        pointer. the pointer must be pre-allocated to be able to
        store 'numInnerCells' */
      void  extractInnerCells(float *whereToWrite,
                              /* the full scalar field, as stored - with
                                 proper offset - in Model::scalars[] */
                              const Model::FieldMeta &field,
                              const std::vector<float>& allScalars);
      
      /*! origin of this grid, on the logical grid of this grid's
          respective level. Note these coordinates will typically need
          to be mulitples of the respective level's refinement level
          to make sense. Origin refers to the lower left front data
          cell _ex_cludign ghost cells, so ghost cells can be to the
          left/front of this origin value. */
      vec3i    origin;
      
      /*! dimensions of this grid's 3D array of cells, _ex_cluding
        ghost cells.  The total number of cells for this grid -
        including ghosts - is `dims+2*ghost`. Assuming
        Mx=dims.x+2*ghost, My=dims.y+2*ghost, etc, then this grid's
        Mx*My*Mz cells are stored, including ghosts, at
        scalars[grid.offset] in z-major order. Careful, including
        ghosts means that if numGhosts!=0, the value stored at
        scalars[grid.offset] is a ghost cell, not the first data
        cell. */
      vec3i    dims;
      
      /*! level of this grid. 0 is the coarsest level, with cell size
          of 1x1x1. Level 1 is 2x refined, level 2 is 4x refineded,
          etc. Codes that use 4x or 8x refinements will simply be
          stores by using only every log(brickSize)'th level */
      uint16_t level;
      // number of ghost cells. this assumes euqal number in all
      // direcions, and both on lower and upper end.
      uint16_t numGhostCells  = 0;
      
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

    vec3f gridOrigin = { 0.f, 0.f, 0.f };
    vec3f gridOffset = { 1.f, 1.f, 1.f };
  };
  
} // ::tamr
