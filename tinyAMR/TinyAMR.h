#pragma once

#include "tinyAMR/common.h"
#include <vector>
#include <memory>

namespace tamr {

  struct Model {
    typedef std::shared_ptr<Model> SP;
    
    struct Block {
      /*! origin of this block, on the logical grid of this block's
          respective level. Note these coordinates will typically need
          to be mulitples of the respective level's refinement level
          to make sense. */
      vec3i    origin;
      
      /*! dimensions of this block's 3D array of cells. The Nx*Ny*Nz
          scalars for this block iwill be stored at
          scalars[block.offset] in z-major order */
      vec3i    dims;
      
      /*! level of this block, *relative to Model::refinementLevel* --
        i.e., the refinement level (and thus, size) or a cell in this
        block is *NOT* necessarily 1/2^block.level (or
        1/refinement^block.level), but it is whatever
        Model::refinementOfLevel[block.level] specifies. */
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
          the frist dimension is at FieldMeta::offset, the second at
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
    std::vector<int>      refinementOfLevel;
    
    /*! array of all scalars, across all blocks, across all scalar
        fields */
    std::vector<float>     scalars;
    std::vector<FieldMeta> fieldMetas;
    std::vector<Block>     blocks;
    /*! meta information that the app or importer may attach; this
        library makes no claims at what this field may or may not
        contain */
    std::string userMeta;
  };
  
} // ::tamr
