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

// std
#include <algorithm>
#include <array>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <vector>
#include <stdexcept>
// hdf5
#include <H5Cpp.h>
// tamr
#include "tinyAMR/Model.h"

namespace tamr {

  
  bool pathExists(hid_t id, const std::string &path)
  {
    int ret = H5Lexists( id, path.c_str(), H5P_DEFAULT );
    PRINT(ret);
    return ret > 0;
  }
  
  herr_t collect_levels_cb(hid_t g_id,
                           const char *name,
                           const H5L_info1_t *info,
                           void *cbPtr)
  {
    int level;
    if (sscanf(name,"level_%i",&level) == 1) {
      std::vector<int> *listOfNames =
        (std::vector<int> *)cbPtr;
      listOfNames->push_back(level);
    }
    return 0; // 0 continues iteration, non-zero halts
  }

  herr_t collect_expressions_cb(hid_t g_id,
                                const char *name,
                                const H5A_info_t *info,
                                void *cbPtr)
  {
    Model *model = (Model *)cbPtr;
    int dims = 0;
    if (strstr(name,"scalar")==name) {
      name += strlen("scalar")+1;
      dims = 1;
    } else if (strstr(name,"vector")==name) {
      name += strlen("vector")+1;
      dims = 3;
    } else
      throw std::runtime_error("un-recognized chombo scalar field expression '"+std::string(name)+"'");
    std::cout << "found field " << name << ", dims = " << dims << std::endl;
    Model::FieldMeta meta;
    meta.name = name;
    meta.numDimensions = dims;
    model->fieldMetas.push_back(meta);
    return 0; // 0 continues iteration, non-zero halts
  }

  // void readLevel(Model::SP model, H5::H5File &file, int level)
  // {
  //   //     H5::DataSet dataset = file.openDataSet((path+"/boxes").c_str());
  //   //     H5::DataSpace dataspace = dataset.getSpace();
  // }
  
  void readChombo(Model::SP model, H5::H5File &file)
  {
    int numMC = -1;
    std::vector<int> usedLevels;

    hid_t file_id = file.getId();
    hsize_t idx = 0;

    model->numCellsAcrossAllGrids = 0;

    // ==================================================================
    // gather levels
    // ==================================================================
    idx = 0;
    H5Literate1(file_id, H5_INDEX_NAME, H5_ITER_INC, &idx,
                collect_levels_cb, (void*)&usedLevels);
  
    // ==================================================================
    // gather attribute names and types
    // ==================================================================
    idx = 0;
    // iterate through root to find all child groups
    H5::Group expressions_grp = file.openGroup("/Expressions");
    H5Aiterate2(expressions_grp.getId(),//file.getObjId("Expressions"),
                H5_INDEX_NAME,//H5_INDEX_CRT_ORDER,
                H5_ITER_INC, &idx,
                collect_expressions_cb, (void*)model.get());

    // tamr counts levels in refinements of 2
    int totalRefinement = 1;
    std::map<int,int> firstGridOnLevel;
    std::map<int,int> numGridsOnLevel;
    // ==================================================================
    // parse levels
    // ==================================================================
    for (auto level : usedLevels) {
      std::cout << "reading level ..." << level << std::endl;
      std::string level_path = "level_" + std::to_string(level);
      PING;
      PRINT(level_path);
      H5::Group level_grp = file.openGroup(level_path.c_str());
      // H5::DataSet level_ds = file.openDataSet(level_path.c_str());
      
      // ==================================================================
      // level/ref_ratio
      // ==================================================================
      int ref_ratio = 0;
      {
        H5::Attribute attr = level_grp.openAttribute("ref_ratio");
        attr.read(H5::PredType::NATIVE_INT32, &ref_ratio);
      }
      int thisBinaryLevel = (int)log2(totalRefinement);
      /* for NEXT level */totalRefinement *= ref_ratio;
      
      // ==================================================================
      H5::Group attributes_grp = level_grp.openGroup("data_attributes");
      H5::Attribute attr = attributes_grp.openAttribute("outputGhost");
      PRINT(attr.getInMemDataSize());
      // int ghost[3];

      vec3i ghost = -1;
      hid_t vec3i_id = H5Tcreate (H5T_COMPOUND, sizeof(vec3i));
      H5Tinsert (vec3i_id, "intvecti", HOFFSET(vec3i, x), H5T_NATIVE_INT);
      H5Tinsert (vec3i_id, "intvectj", HOFFSET(vec3i, y), H5T_NATIVE_INT);
      H5Tinsert (vec3i_id, "intvectk", HOFFSET(vec3i, z), H5T_NATIVE_INT);

      attr.read(vec3i_id,(void*)&ghost);
      if (ghost.y != ghost.x || ghost.z != ghost.x)
        throw std::runtime_error
          ("this version of tamr can only do ghost cells if they are the "
           "same number in all dimensions");
      
      // ==================================================================
      // level/boxes
      // ==================================================================
      H5::DataSet boxes_ds = level_grp.openDataSet("boxes");

      hid_t box3i_id = H5Tcreate (H5T_COMPOUND, sizeof(box3i));
      H5Tinsert (box3i_id, "lo_i", HOFFSET(box3i, lower.x), H5T_NATIVE_INT);
      H5Tinsert (box3i_id, "lo_j", HOFFSET(box3i, lower.y), H5T_NATIVE_INT);
      H5Tinsert (box3i_id, "lo_k", HOFFSET(box3i, lower.z), H5T_NATIVE_INT);
      H5Tinsert (box3i_id, "hi_i", HOFFSET(box3i, upper.x), H5T_NATIVE_INT);
      H5Tinsert (box3i_id, "hi_j", HOFFSET(box3i, upper.y), H5T_NATIVE_INT);
      H5Tinsert (box3i_id, "hi_k", HOFFSET(box3i, upper.z), H5T_NATIVE_INT);
      
      H5::DataSpace boxes_spc = boxes_ds.getSpace();
      std::vector<box3i> boxes(boxes_spc.getSimpleExtentNpoints());
      boxes_ds.read(boxes.data(), box3i_id,
                    boxes_spc, boxes_spc);
      boxes_ds.close();
      level_grp.close();

      firstGridOnLevel[level] = model->grids.size();
      numGridsOnLevel[level]  = boxes.size();
      for (auto box : boxes) {
        Model::Grid grid;
        grid.origin = box.lower;
        // chombo stores 'upper' as 'including' interval, rather than
        // our box classes that use including for rlower and excluding
        // for upper - so need to add '1' for size
        grid.dims = box.size()+vec3i(1);
        grid.level = thisBinaryLevel;
        grid.numGhostCells = ghost.x;
        grid.offset = model->numCellsAcrossAllGrids;
        
        model->numCellsAcrossAllGrids += grid.numTotalCells();
        model->grids.push_back(grid);
      }
    }
    PING;
    PRINT(model->numCellsAcrossAllGrids);

    size_t metaOffset = 0;
    for (auto &meta : model->fieldMetas) {
      meta.offset = metaOffset;
      metaOffset += meta.numDimensions * model->numCellsAcrossAllGrids;
    }
    size_t numScalarsTotal = metaOffset;
    PRINT(numScalarsTotal);
    model->scalars.resize(numScalarsTotal);
    
    // ==================================================================
    // now, read the actual data...
    // ==================================================================
    for (auto level : usedLevels) {
      std::cout << "reading level ..." << level << std::endl;
      std::string level_path = "level_" + std::to_string(level);
      PING;
      PRINT(level_path);
      H5::Group level_grp = file.openGroup(level_path.c_str());
      
      // ==================================================================
      // data::offsets=0
      // ==================================================================
      H5::DataSet offsets_ds = level_grp.openDataSet("data:offsets=0");
      
      H5::DataSpace offsets_spc = offsets_ds.getSpace();
      std::vector<size_t> offsets(offsets_spc.getSimpleExtentNpoints());
      PRINT(offsets.size());
      offsets_ds.read(offsets.data(), H5::PredType::NATIVE_INT64,
                      offsets_spc, offsets_spc);
      
      // ==================================================================
      // data::datatype=0
      // ==================================================================
      H5::DataSet data_ds = level_grp.openDataSet("data:datatype=0");
      
      H5::DataSpace data_spc = data_ds.getSpace();
      std::vector<double> data(data_spc.getSimpleExtentNpoints());
      PRINT(data.size());
      offsets_ds.read(data.data(), H5::PredType::NATIVE_DOUBLE,
                      data_spc, data_spc);
      
      for (int gridIdx=0;gridIdx<numGridsOnLevel[level];gridIdx++) {
        const auto &grid = model->grids[firstGridOnLevel[level]+gridIdx];
        size_t inOffset = offsets[gridIdx];
        for (auto &meta : model->fieldMetas) {
          for (int cellID=0;cellID<grid.numTotalCells();cellID++) {
            for (int dim=0;dim<meta.numDimensions;dim++) {
              double scalar = data[inOffset++];
              
              size_t outOffset
                = meta.offset
                + grid.offset
                + cellID * meta.numDimensions
                + dim;
              model->scalars[outOffset] = scalar;
            }
          }
        }
      }
    }
    
    // ==================================================================
    // compute and set grids' offsets and ghost values
    // ==================================================================
    size_t offset = 0;
    
    model->numCellsAcrossAllGrids = offset;
  }
  
  Model::SP import_chombo(const char *fileName, int fieldIndex)
  {
    Model::SP model = std::make_shared<Model>();
    if (!H5::H5File::isHdf5(fileName))
      return {};
    try {
      H5::H5File file(fileName, H5F_ACC_RDONLY);
      readChombo(model,file);
    } catch (H5::FileIException error) {
      error.printErrorStack();
      return {};
    }
    
    return model;
  }

} // ::tamr

