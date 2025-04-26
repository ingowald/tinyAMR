// Copyright 2024-2025 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

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
// hdf5
#include <H5Cpp.h>
// tamr
#include "tinyAMR/TinyAMR.h"

namespace tamr {

#define MAX_STRING_LENGTH 80

  using GridBounds = std::array<int, 6>;
  struct GridData
  {
    int dims[3];
    std::vector<float> values;
  };
  
  struct AMRField
  {
    std::vector<float> cellWidth;
    std::vector<int> gridLevel;
    std::vector<GridBounds> gridBounds;
    std::vector<GridData> gridData;
    struct
    {
      float x, y;
    } voxelRange;
  };

  struct sim_info_t
  {
    int file_format_version;
    char setup_call[400];
    char file_creation_time[MAX_STRING_LENGTH];
    char flash_version[MAX_STRING_LENGTH];
    char build_date[MAX_STRING_LENGTH];
    char build_dir[MAX_STRING_LENGTH];
    char build_machine[MAX_STRING_LENGTH];
    char cflags[400];
    char fflags[400];
    char setup_time_stamp[MAX_STRING_LENGTH];
    char build_time_stamp[MAX_STRING_LENGTH];
  };

  struct grid_t
  {
    using char4 = std::array<char, 4>;
    struct __attribute__((packed)) vec3d
    {
      double x, y, z;
    };

    struct aabbd
    {
      vec3d min, max;
    };

    struct __attribute__((packed)) gid_t
    {
      int neighbors[6];
      int parent;
      int children[8];
    };

    std::vector<char4> unknown_names;
    std::vector<int> refine_level;
    std::vector<int> node_type; // node_type 1 ==> leaf
    std::vector<gid_t> gid;
    std::vector<vec3d> coordinates;
    std::vector<vec3d> grid_size;
    std::vector<aabbd> bnd_box;
    std::vector<int> which_child;
  };

  struct variable_t
  {
    size_t global_num_grids;
    size_t nxb;
    size_t nyb;
    size_t nzb;
    std::vector<double> data;
  };

  inline void read_sim_info(sim_info_t &dest, H5::H5File const &file)
  {
    H5::StrType str80(H5::PredType::C_S1, 80);
    H5::StrType str400(H5::PredType::C_S1, 400);

    H5::CompType ct(sizeof(sim_info_t));
    ct.insertMember("file_format_version", 0, H5::PredType::NATIVE_INT);
    ct.insertMember("setup_call", 4, str400);
    ct.insertMember("file_creation_time", 404, str80);
    ct.insertMember("flash_version", 484, str80);
    ct.insertMember("build_date", 564, str80);
    ct.insertMember("build_dir", 644, str80);
    ct.insertMember("build_machine", 724, str80);
    ct.insertMember("cflags", 804, str400);
    ct.insertMember("fflags", 1204, str400);
    ct.insertMember("setup_time_stamp", 1604, str80);
    ct.insertMember("build_time_stamp", 1684, str80);

    H5::DataSet dataset = file.openDataSet("sim info");

    dataset.read(&dest, ct);
  }

  inline void read_grid(grid_t &dest, H5::H5File const &file)
  {
    H5::DataSet dataset;
    H5::DataSpace dataspace;

    {
      H5::StrType str4(H5::PredType::C_S1, 4);

      dataset = file.openDataSet("unknown names");
      dataspace = dataset.getSpace();
      dest.unknown_names.resize(dataspace.getSimpleExtentNpoints());
      dataset.read(dest.unknown_names.data(), str4, dataspace, dataspace);
    }

    {
      dataset = file.openDataSet("refine level");
      dataspace = dataset.getSpace();
      dest.refine_level.resize(dataspace.getSimpleExtentNpoints());
      dataset.read(dest.refine_level.data(),
                   H5::PredType::NATIVE_INT,
                   dataspace,
                   dataspace);
    }
    
    {
      dataset = file.openDataSet("node type");
      dataspace = dataset.getSpace();
      dest.node_type.resize(dataspace.getSimpleExtentNpoints());
      dataset.read(
                   dest.node_type.data(), H5::PredType::NATIVE_INT, dataspace, dataspace);
    }

    {
      dataset = file.openDataSet("gid");
      dataspace = dataset.getSpace();

      hsize_t dims[2];
      dataspace.getSimpleExtentDims(dims);
      dest.gid.resize(dims[0]);
      assert(dims[1] == 15);

      dataset.read(
                   dest.gid.data(), H5::PredType::NATIVE_INT, dataspace, dataspace);
    }

    {
      dataset = file.openDataSet("coordinates");
      dataspace = dataset.getSpace();

      hsize_t dims[2];
      dataspace.getSimpleExtentDims(dims);
      dest.coordinates.resize(dims[0]);
      assert(dims[1] == 3);

      dataset.read(dest.coordinates.data(),
                   H5::PredType::NATIVE_DOUBLE,
                   dataspace,
                   dataspace);
    }

    {
      dataset = file.openDataSet("grid size");
      dataspace = dataset.getSpace();

      hsize_t dims[2];
      dataspace.getSimpleExtentDims(dims);
      dest.grid_size.resize(dims[0]);
      assert(dims[1] == 3);

      dataset.read(dest.grid_size.data(),
                   H5::PredType::NATIVE_DOUBLE,
                   dataspace,
                   dataspace);
    }

    {
      dataset = file.openDataSet("bounding box");
      dataspace = dataset.getSpace();

      hsize_t dims[3];
      dataspace.getSimpleExtentDims(dims);
      dest.bnd_box.resize(dims[0] * 2);
      assert(dims[1] == 3);
      assert(dims[2] == 2);

      std::vector<double> temp(dims[0] * dims[1] * dims[2]);

      dataset.read(
                   temp.data(), H5::PredType::NATIVE_DOUBLE, dataspace, dataspace);

      dest.bnd_box.resize(dims[0]);
      for (size_t i = 0; i < dims[0]; ++i) {
        dest.bnd_box[i].min.x = temp[i * 6];
        dest.bnd_box[i].max.x = temp[i * 6 + 1];
        dest.bnd_box[i].min.y = temp[i * 6 + 2];
        dest.bnd_box[i].max.y = temp[i * 6 + 3];
        dest.bnd_box[i].min.z = temp[i * 6 + 4];
        dest.bnd_box[i].max.z = temp[i * 6 + 5];
      }
    }

    {
      dataset = file.openDataSet("which child");
      dataspace = dataset.getSpace();
      dest.which_child.resize(dataspace.getSimpleExtentNpoints());
      dataset.read(dest.which_child.data(),
                   H5::PredType::NATIVE_INT,
                   dataspace,
                   dataspace);
    }
  }

  inline void read_variable(
                            variable_t &var, H5::H5File const &file, char const *varname)
  {
    H5::DataSet dataset = file.openDataSet(varname);
    H5::DataSpace dataspace = dataset.getSpace();

    hsize_t dims[4];
    dataspace.getSimpleExtentDims(dims);
    var.global_num_grids = dims[0];
    var.nxb = dims[1];
    var.nyb = dims[2];
    var.nzb = dims[3];
    var.data.resize(dims[0] * dims[1] * dims[2] * dims[3]);
    dataset.read(
                 var.data.data(), H5::PredType::NATIVE_DOUBLE, dataspace, dataspace);
  }

  inline AMRField toAMRField(const grid_t &grid, const variable_t &var)
  {
    AMRField result;

    // Length of the sides of the bounding box
    double len_total[3] = {grid.bnd_box[0].max.x - grid.bnd_box[0].min.x,
                           grid.bnd_box[0].max.y - grid.bnd_box[0].min.y,
                           grid.bnd_box[0].max.z - grid.bnd_box[0].min.z};

    int max_level = 0;
    double len[3];
    for (size_t i = 0; i < var.global_num_grids; ++i) {
      if (grid.refine_level[i] > max_level) {
        max_level = grid.refine_level[i];
        len[0] = grid.bnd_box[i].max.x - grid.bnd_box[i].min.x;
        len[1] = grid.bnd_box[i].max.y - grid.bnd_box[i].min.y;
        len[2] = grid.bnd_box[i].max.z - grid.bnd_box[i].min.z;
      }
    }

    // --- cellWidth
    for (int l = 0; l <= max_level; ++l) {
      result.cellWidth.push_back(1 << l);
    }

    len[0] /= var.nxb;
    len[1] /= var.nyb;
    len[2] /= var.nzb;

    // This is the number of cells for the finest level (?)
    int vox[3];
    vox[0] = static_cast<int>(round(len_total[0] / len[0]));
    vox[1] = static_cast<int>(round(len_total[1] / len[1]));
    vox[2] = static_cast<int>(round(len_total[2] / len[2]));

    float max_scalar = -FLT_MAX;
    float min_scalar = FLT_MAX;

    size_t numLeaves = 0;
    for (size_t i = 0; i < var.global_num_grids; ++i) {
      if (grid.node_type[i] == 1)
        numLeaves++;
    }

    for (size_t i = 0; i < var.global_num_grids; ++i) {
      // Project min on vox grid
      int level = max_level - grid.refine_level[i];
      int cellsize = 1 << level;

      int lower[3] = {
        static_cast<int>(round((grid.bnd_box[i].min.x - grid.bnd_box[0].min.x)
                               / len_total[0] * vox[0])),
        static_cast<int>(round((grid.bnd_box[i].min.y - grid.bnd_box[0].min.y)
                               / len_total[1] * vox[1])),
        static_cast<int>(round((grid.bnd_box[i].min.z - grid.bnd_box[0].min.z)
                               / len_total[2] * vox[2]))};

      GridBounds bounds = {{lower[0] / cellsize,
                               lower[1] / cellsize,
                               lower[2] / cellsize,
                               int(lower[0] / cellsize + var.nxb - 1),
                               int(lower[1] / cellsize + var.nyb - 1),
                               int(lower[2] / cellsize + var.nzb - 1)}};

      GridData data;
      data.dims[0] = var.nxb;
      data.dims[1] = var.nyb;
      data.dims[2] = var.nzb;
      for (int z = 0; z < var.nzb; ++z) {
        for (int y = 0; y < var.nyb; ++y) {
          for (int x = 0; x < var.nxb; ++x) {
            size_t index = i * var.nxb * var.nyb * var.nzb + z * var.nyb * var.nxb
              + y * var.nxb + x;
            double val = var.data[index];
            val = val == 0.0 ? 0.0 : log(val);
            float valf(val);
            min_scalar = fminf(min_scalar, valf);
            max_scalar = fmaxf(max_scalar, valf);
            data.values.push_back((float)val);
          }
        }
      }

      result.gridLevel.push_back(level);
      result.gridBounds.push_back(bounds);
      result.gridData.push_back(data);
      result.voxelRange = {min_scalar, max_scalar};
    }

    // logStatus("[import_FLASH] --> value range: %f, %f", min_scalar, max_scalar);

    return result;
  }

  struct FlashReader
  {
    bool open(const char *fileName)
    {
      if (!H5::H5File::isHdf5(fileName))
        return false;

      try {
        file = H5::H5File(fileName, H5F_ACC_RDONLY);
        // Read simulation info
        sim_info_t sim_info;
        read_sim_info(sim_info, file);

        // Read grid data
        read_grid(grid, file);

        // logStatus("[import_FLASH] variables found:");
        for (std::size_t i = 0; i < grid.unknown_names.size(); ++i) {
          std::string uname(
                            grid.unknown_names[i].data(), grid.unknown_names[i].data() + 4);
          // logStatus("    %s", uname.c_str());
          fieldNames.push_back(uname);
        }
      } catch (H5::FileIException error) {
        error.printErrorStack();
        return false;
      }

      return true;
    }

    AMRField getField(int index, std::string &fieldName)
    {
      try {
        // logStatus(
        //           "[import_FLASH] reading field '%s'...", fieldNames[index].c_str());
        fieldName = fieldNames[index].c_str();
        printf("[import_FLASH] reading field '%s'...", fieldName.c_str());
        read_variable(currentField, file, fieldName.c_str());
        // logStatus("[import_FLASH] converting to AMRField...");
        return toAMRField(grid, currentField);
      } catch (H5::DataSpaceIException error) {
        error.printErrorStack();
        exit(EXIT_FAILURE);
      } catch (H5::DataTypeIException error) {
        error.printErrorStack();
        exit(EXIT_FAILURE);
      }

      return {};
    }

    H5::H5File file;
    std::vector<std::string> fieldNames;
    grid_t grid;
    variable_t currentField;
  };

  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////

  Model::SP import_FLASH(const char *filepath, int fieldIndex)
  {
    FlashReader reader;
    if (!reader.open(filepath)) {
      throw std::runtime_error
        ("[import_FLASH] failed to open file '"+std::string(filepath)+"'");
    }
    
    Model::SP model = std::make_shared<Model>();
    model->userMeta = filepath;

    std::string fieldName;
    AMRField data = reader.getField(fieldIndex,fieldName);
    model->fieldMetas.resize(1);
    model->fieldMetas[0].name = fieldName;
    
    int numGrids = data.gridLevel.size();
    assert(numGrids == data.cellWidth.size());
    assert(numGrids == data.gridData.size());
    std::map<float,int> cellWidthToLevelID;
    for (int bid=0;bid<numGrids;bid++) {
      model->numCellsAcrossAllGrids = 0;
      Model::Grid grid;

      box3i coords = (const box3i &)data.gridBounds[bid];
      vec3i dims = (const vec3i &)data.gridData[bid].dims;
      
      // coords.size() is one less than dims
      grid.dims = dims;
      grid.level = data.gridLevel[bid];
      grid.origin = coords.lower;
      grid.offset = model->scalars.size();
      model->numCellsAcrossAllGrids += data.gridData[bid].values.size();
      for (auto scalar : data.gridData[bid].values)
        model->scalars.push_back(scalar);
      float cellWidth = data.cellWidth[grid.level];//bid];
      if (cellWidthToLevelID.find(cellWidth) == cellWidthToLevelID.end()) {
        int refine = int(log2(1.f/cellWidth));
        
        cellWidthToLevelID[cellWidth] = model->refinementOfLevel.size();
        model->refinementOfLevel.push_back(refine);
      }
      grid.level = cellWidthToLevelID[cellWidth];

      model->grids.push_back(grid);
    }
    // now fix the refinement levels; flash importer stolen from tsd
    // adjusts to FINEST level having width 1, not coarsest.
    int minRef = 0;
    for (auto &rol : model->refinementOfLevel)
      minRef = std::min(minRef,rol);
    for (auto &rol : model->refinementOfLevel)
      rol = rol - minRef;
      
    return model;
  }

} // ::tamr

