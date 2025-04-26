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
  for (int i=1;i<ac;i++) {
    const std::string arg = av[i];
    if (arg[0] != '-') {
      inFileName = arg;
    } else
      usage("tamrinfo: unknown cmdline arg '"+arg+"'");
  }

  if (inFileName.empty()) usage("no input file specified");

  tamr::Model::SP model = tamr::Model::load(inFileName);
  std::cout << "num grids   " << prettyNumber(model->grids.size()) << std::endl;
  std::cout << "num scalars " << prettyNumber(model->scalars.size()) << std::endl;
  std::cout << "num fields  " << prettyNumber(model->fieldMetas.size()) << std::endl;
  for (auto &meta : model->fieldMetas)
    std::cout << " - '" << meta.name << "' with array offset "
              << prettyNumber(meta.offset) << std::endl;
  std::cout << "num different levels used " << model->refinementOfLevel.size() << std::endl;
  for (int i=0;i<model->refinementOfLevel.size();i++) {
    vec3i dims;
    box3i bounds;
    for (auto &grid : model->grids)
      if (grid.level == i) {
        dims = grid.dims;
        bounds.extend(grid.origin);
        bounds.extend(grid.origin+dims-vec3i(1));
      }
    std::cout << " - level[" << i << "] : " << std::endl;
    std::cout << "   - refinement is "
              << model->refinementOfLevel[i] << " (-> cell width "
              << 1.f/(1<<model->refinementOfLevel[i]) << ")" << std::endl;
    std::cout << "   - coordinate bounds on this level " << bounds << std::endl;
    std::cout << "   - last brick on this level has dims " << dims << std::endl;
  }
  return 0;
}

