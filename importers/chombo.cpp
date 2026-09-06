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
#include <map>
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
    std::vector<std::pair<std::string,int>> &usedFields
      = *(std::vector<std::pair<std::string,int>>*)cbPtr;
                                
    int dims = 0;
    PING; PRINT(name);
    if (strstr(name,"scalar")==name) {
      name += strlen("scalar")+1;
      dims = 1;
    } else if (strstr(name,"vector")==name) {
      name += strlen("vector")+1;
      dims = 3;
    } else
      throw std::runtime_error("un-recognized chombo scalar field expression '"
                               +std::string(name)+"'");
    std::cout << "found field " << name << ", dims = " << dims << std::endl;
    usedFields.push_back({std::string(name),dims});
    return 0; // 0 continues iteration, non-zero halts
  }

  herr_t collect_components_cb(hid_t g_id,
                               const char *name,
                               const H5A_info_t *info,
                               void *cbPtr)
  {
    std::vector<std::string> &foundComps
      = *(std::vector<std::string>*)cbPtr;

    int component = -1;
    if (sscanf(name,"component_%i",&component) != 1)
      return 0;

    // auto attr_id = g_id;
    // hid_t type_id = H5Aget_type(attr_id);
    // char* compName = NULL;
    // H5Aread(attr_id, type_id, &compName);
    // PING; PRINT(compName);
    const char *compName = name;
    if (component >= foundComps.size())
      foundComps.resize(component+1);
    foundComps[component] = compName;
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
    
    // tamr counts levels in refinements of 2
    int totalRefinement = 1;
    std::map<int,size_t> firstGridOnLevel;
    std::map<int,size_t> numGridsOnLevel;
    // ==================================================================
    // parse levels
    // ==================================================================
    for (auto level : usedLevels) {
      std::cout << "reading bricks from level ... " << level << std::endl;
      std::string level_path = "level_" + std::to_string(level);
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

    // ==================================================================
    // now we know all boxes - across all levels - compute per-field
    // size and offsets
    // ==================================================================
    size_t offset = 0;
    for (auto &grid : model->grids) {
      grid.offset = offset;
      offset += grid.numTotalCells();
    }
    model->numCellsAcrossAllGrids = offset;
    std::cout << "num cells, across all grids, including ghosts, per field : "
              << model->numCellsAcrossAllGrids << std::endl;

    // ==================================================================
    // gather attribute names and types
    // ==================================================================
    idx = 0;
    std::vector<std::string> foundComps;
    int numComponents;
    hid_t numComponents_id = H5Aopen_name(file.getId(), "num_components");
    H5Aread(numComponents_id, H5T_NATIVE_INT, &numComponents);
    PRINT(numComponents);
    H5Aclose(numComponents_id);
    for (int compID=0;compID<numComponents;compID++) {
      std::string varName = "component_"+std::to_string(compID);
      hid_t name_id = H5Aopen_name(file.getId(), varName.c_str());
      char value[1024] = { '\0' };
      hid_t atype = H5Aget_type(name_id);
      H5Aread(name_id, atype, value);
      foundComps.push_back(value);
      H5Aclose(name_id);
    }
 
    // ==================================================================
    // create field metas, with offsets
    // ==================================================================
    offset = 0;
    for (auto &field : foundComps) {
      Model::FieldMeta meta;
      meta.name = field;
      meta.offset = offset;
      
      model->fieldMetas.push_back(meta);
      offset += model->numCellsAcrossAllGrids;
    }
    for (int i=0;i<model->fieldMetas.size();i++)
      std::cout << "found field #" << i << ": " << model->fieldMetas[i].name
                << ", to be stored at " << model->fieldMetas[i].offset
                << std::endl;
    const size_t totalNumCellsAcrossAllFields = offset;
    PRINT(totalNumCellsAcrossAllFields);
    model->scalars.resize(totalNumCellsAcrossAllFields);
    
    // ==================================================================
    // now, read the actual scalars:
    // ==================================================================
    for (auto level : usedLevels) {
      std::cout << "reading scalars from level ... " << level << std::endl;
      std::string level_path = "level_" + std::to_string(level);
      H5::Group level_grp = file.openGroup(level_path.c_str());
      
      // ==================================================================
      // data::offsets=0
      // ==================================================================
      H5::DataSet offsets_ds = level_grp.openDataSet("data:offsets=0");
      
      H5::DataSpace offsets_spc = offsets_ds.getSpace();
      std::vector<size_t> offsets(offsets_spc.getSimpleExtentNpoints());
      // PRINT(offsets.size());
      offsets_ds.read(offsets.data(), H5::PredType::NATIVE_INT64,
                      offsets_spc, offsets_spc);
      // PRINT(offsets[0]);
      // PRINT(offsets[offsets.size()-1]);
      size_t allScalarsOnThisLevel = offsets.back();
       // PRINT(allScalarsOnThisLevel);

      // ==================================================================
      // data::datatype=0
      // ==================================================================
      H5::DataSet data_ds = level_grp.openDataSet("data:datatype=0");
      
      H5::DataSpace data_spc = data_ds.getSpace();
      int rank = data_spc.getSimpleExtentNdims();;
      // PRINT(rank);
      hsize_t data_dims[2] = { 0, 0 };
      data_spc.getSimpleExtentDims(data_dims, NULL);
      // these should match:
      // PRINT(data_dims[0]); 
      // PRINT(data_dims[1]);
      // PRINT(allScalarsOnThisLevel);

      std::vector<double> scalarsOnThisLevel(allScalarsOnThisLevel);

      /* we read this in chunks; as chunk size we choose
         'scalarsPerField'; note this does NOT mean that every chunk
         we read is one scalar field (it isn't!), but this is a value
         we know that total num scalars is a multiple of, so easisest
         to chunk in */
      size_t numFields = model->fieldMetas.size();
      size_t numChunks = numFields;
      size_t scalarsPerChunk=allScalarsOnThisLevel/numChunks;

      hsize_t mem_dims[2] = { scalarsPerChunk, 1 };
      H5::DataSpace memspace(1, mem_dims);
      for (size_t chunk=0;chunk<numChunks;chunk++) {
        hsize_t offset[2] = { chunk * scalarsPerChunk, 0 };
        hsize_t count[2]  = { scalarsPerChunk, 1 };
        
        data_spc.selectHyperslab(H5S_SELECT_SET, count, offset);
        
        // Read the chunk from the file into the memory buffer
        data_ds.read(scalarsOnThisLevel.data() + chunk * scalarsPerChunk,
                     H5::PredType::NATIVE_DOUBLE,
                     memspace, data_spc);
      }

      for (size_t gridIdx=0;gridIdx<numGridsOnLevel[level];gridIdx++) {
        auto grid = model->grids[firstGridOnLevel[level]+gridIdx];
        size_t numCellsInGrid = grid.numTotalCells();
        size_t numScalarsIndicated = offsets[gridIdx+1]-offsets[gridIdx];
        size_t numScalarsExpected = numFields * numCellsInGrid;
        if (numScalarsExpected != numScalarsIndicated) {
          PING;
          PRINT(numFields);
          PRINT(numCellsInGrid);
          PRINT(numScalarsIndicated);
          PRINT(numScalarsExpected);
          PRINT(level);
          PRINT(gridIdx);
          throw std::runtime_error("un-expected num scalars");
        }
        const double *in
          = scalarsOnThisLevel.data()
          + offsets[gridIdx];
        for (int fieldIdx=0;fieldIdx<numFields;fieldIdx++) {
          float *out
            = model->scalars.data()
            + model->fieldMetas[fieldIdx].offset
            + grid.offset;
          for (int i=0;i<numCellsInGrid;i++)
            *out++ = *in++;
        }
      }
      
      offsets_ds.close();
      data_ds.close();
      level_grp.close();
    }
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

    std::cout << "just a few sanity prints ..." << std::endl;
    int numGridsToPrint = 16;
    for (int i=0;i<numGridsToPrint;i++) {
      int gridID = (int)(model->grids.size()*(i+.5)/numGridsToPrint);
      auto &grid = model->grids[gridID];
      std::cout << "=======================================================" << std::endl;
      std::cout << "grid #" << gridID
                << " (@level " << grid.level << ")"
                << std::endl;
      vec3i N = grid.dims + 2*grid.numGhostCells;
      if (N.x == N.y && N.x == N.z) {
      for (auto &field : model->fieldMetas) {
        std::cout << "- for field " << field.name << std::endl;
        float *cells
          = model->scalars.data()
          + field.offset
          + grid.offset;

        std::cout << "  * [0,0,x=:] (incl ghost) =";
        for (int i=0;i<N.x;i++) {
          vec3i idx(i,0,0);
          std::cout << " " << cells[idx.x+N.x*idx.y+N.x*N.y*idx.z];
        }
        std::cout << std::endl;

        std::cout << "  * [0,y=:,0] (incl ghost) =";
        for (int i=0;i<N.x;i++) {
          vec3i idx(0,i,0);
          std::cout << " " << cells[idx.x+N.x*idx.y+N.x*N.y*idx.z];
        }
        std::cout << std::endl;

        std::cout << "  * [:,:,:] (diag, incl ghost) =";
        for (int i=0;i<N.x;i++) {
          vec3i idx(i,i,i);
          std::cout << " " << cells[idx.x+N.x*idx.y+N.x*N.y*idx.z];
        }
        std::cout << std::endl;
      }
      }
    }
    return model;
  }

} // ::tamr

