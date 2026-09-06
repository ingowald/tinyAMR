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

#include "tinyAMR/Model.h"

void usage(const std::string &error)
{
  std::cout << "Error: " << error << "\n\n";
  std::cout << "Usage: ./tamrInfo inFileName.tamr" << std::endl;
  exit(1);
}

int main(int ac, char **av)
{
  using namespace tamr;
    
  std::string inFileName;
  std::string outFileName = "stripped.tamr";
  for (int i=1;i<ac;i++) {
    const std::string arg = av[i];
    if (arg[0] != '-') {
      inFileName = arg;
    } else if (arg == "-o") {
      outFileName = av[++i];
    } else
      usage("tamrinfo: unknown cmdline arg '"+arg+"'");
  }

  if (inFileName.empty()) usage("no input file specified");

  tamr::Model::SP model = tamr::Model::load(inFileName);
  std::cout << "generating stripped scalars and updating field offsets" << std::endl;
  std::vector<float> outScalars;
  const float *in = model->scalars.data();
  for (auto &field : model->fieldMetas) {
    field.offset = outScalars.size();
    for (auto &grid : model->grids) {
      vec3i N = grid.dims + vec3i(grid.numGhostCells);
      int Ng = grid.numGhostCells;
      for (int iz=0;iz<N.z;iz++)
        for (int iy=0;iy<N.y;iy++)
          for (int ix=0;ix<N.x;ix++) {
            float f = *in++;
            if (ix < Ng) continue;
            if (iy < Ng) continue;
            if (iz < Ng) continue;
            if (ix-Ng >= grid.dims.x) continue;
            if (iy-Ng >= grid.dims.y) continue;
            if (iz-Ng >= grid.dims.z) continue;
            outScalars.push_back(f);
          }
    }
  }
  model->scalars = outScalars;
  
  std::cout << "fixing grid offsets" << std::endl;
  size_t offset = 0;
  for (auto &grid : model->grids) {
    grid.offset = offset;
    grid.numGhostCells = 0;
    offset += grid.numTotalCells();
  }
  std::cout << "saving to " << outFileName << std::endl;
  model->save(outFileName);
  std::cout << "saved, done" << std::endl;
  return 0;
}

