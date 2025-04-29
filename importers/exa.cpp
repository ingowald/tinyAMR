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
#include <optional>
#include <vector>
#include <stdexcept>
#include <fstream>
// tamr
#include "tinyAMR/Model.h"

namespace tamr {
  namespace wholeFile {
    template<typename T>
    std::vector<T> readVector(const std::string &fileName)
    {
      std::vector<T> res;
      std::ifstream in(fileName.c_str(),std::ios::binary);
      while (!in.eof()) {
        T t;
        in.read((char*)&t,sizeof(t));
        if (!in.good()) break;
        res.push_back(t);
      }
      return res;
    }
  }
  
  struct Cell {
    template<int BS>
    std::pair<vec3i,int> gridCoords() const {
      return {(coord/BS),level};
    }
    vec3i coord; int level;
  };

  std::ostream &operator<<(std::ostream &out, const Cell &cell)
  {
    out << "(" << cell.coord << ";" << cell.level << ")";
    return out;
  }


  template<int BS>
  Model::SP makeGridsT(const std::vector<Cell> &cells,
                       std::vector<int> &cellOffsets)
  {
    std::cout << "exa: trying to make grids of size " << vec3i(BS) << std::endl;
    Model::SP model = std::make_shared<Model>();
    std::map<std::pair<vec3i,int>,int> cellCountOfGrid;
    for (auto cell : cells)
      cellCountOfGrid[cell.gridCoords<BS>()]++;

    for (auto pair : cellCountOfGrid)
      if (pair.second != BS*BS*BS) {
        std::cout << "  -> not possible..." << std::endl;
        throw std::runtime_error("incomplete grid");
      }
    std::cout << "K, found our grid size: " << vec3i(BS)
              << " it is going to be!" << std::endl;

    std::map<std::pair<vec3i,int>,int> gridIDOfGridCoord;
    size_t offset = 0;
    for (auto pair : cellCountOfGrid) {
      vec3i origin = pair.first.first;
      int   level  = pair.first.second;
      Model::Grid grid;
      
      grid.origin = origin * vec3i(BS);
      grid.level  = level;
      grid.dims   = vec3i(BS);
      grid.user   = 0;
      grid.offset = offset;

      offset += BS*BS*BS;
      
      gridIDOfGridCoord[pair.first] = model->grids.size();
      model->grids.push_back(grid);
    }
    
    model->numCellsAcrossAllGrids = offset;
    cellOffsets.clear();
    for (int cellID = 0;cellID<cells.size();cellID++) {
      auto cell = cells[cellID];
      int gridID = gridIDOfGridCoord[cell.gridCoords<BS>()];
      const Model::Grid &grid = model->grids[gridID];
      vec3i local = cell.coord - grid.origin;
      assert(local.x >= 0);
      assert(local.y >= 0);
      assert(local.z >= 0);
      assert(local.x < BS);
      assert(local.y < BS);
      assert(local.z < BS);
      int ofsInGrid = local.x+BS*local.y+BS*BS*local.z;
      size_t globalOfs = grid.offset+ofsInGrid;
      assert(globalOfs >= 0);
      assert(globalOfs < cells.size());
      cellOffsets.push_back(globalOfs);//[cellID] = globalOfs;
    }
    return model;
  }
  
  Model::SP makeGrids(const std::vector<Cell> &cells,
                      std::vector<int> &cellOffsets)
  {
#if 0
    try {
      return makeGridsT<16>(cells,cellOffsets);
    } catch (...) {};
    try {
      return makeGridsT<8>(cells,cellOffsets);
    } catch (...) {};
#endif
    try {
      return makeGridsT<4>(cells,cellOffsets);
    } catch (...) {};
    try {
      return makeGridsT<2>(cells,cellOffsets);
    } catch (...) {};
    throw std::runtime_error
      ("tamr::exa::fatal error - cannot make grids, not even for octree refinement");
  }
  


  
  /*! note exabrick counts level from 0 upward, counts levels in
      refinements of 2 then given all coords in finest-level
      scale. Ie., a cell on level 0 is on the FINEST level, and a cell
      of level 12 is 2^12(=4k) times as wide/big as a level 0 cell -
      and all coords of level-12 (exa-)cells will be in mulitples of
      4k */
  Model::SP import_exa(const std::string &cellFileName,
                       const std::vector<std::string> &scalarsFileNames)
  {
    std::vector<Cell> cells = wholeFile::readVector<Cell>(cellFileName);
    int maxLevel = 0;
    box3i bounds;
    for (auto cell : cells) {
      maxLevel = std::max(maxLevel,cell.level);
      bounds.extend(cell.coord);
      bounds.extend(cell.coord+(1<<cell.level));
    }
    PRINT(maxLevel);
    PRINT(bounds);

    // transform back to our refinement
    for (auto &cell : cells) {
      cell.coord = cell.coord / vec3i(1<<cell.level);
      cell.level = maxLevel - cell.level;
    }

    std::vector<int> cellOffsets;
    Model::SP model = makeGrids(cells,cellOffsets);
    for (int i=0;i<=maxLevel;i++)
      model->refinementOfLevel.push_back((1<<i));

    for (auto fn : scalarsFileNames) {
      Model::FieldMeta field;
      field.offset = model->scalars.size();
      field.numDimensions = 1;
      field.name = fn;
      model->fieldMetas.push_back(field);
      
      std::cout << "loading scalars from " << fn << std::endl;
      std::vector<float> fromFile = wholeFile::readVector<float>(fn);
      assert(fromFile.size() == model->numCellsAcrossAllGrids);

      if (fromFile.size() != cellOffsets.size())
        throw std::runtime_error("mismatch of scalars count and cell count");
      std::vector<float> reordered(fromFile.size());
      for (int i=0;i<fromFile.size();i++) {
        int co = cellOffsets[i];
        reordered[co] = fromFile[i];
      }
      for (auto s : reordered)
        model->scalars.push_back(s);
    }
    
    return model;
  }

} // ::tamr

