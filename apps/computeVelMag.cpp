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
  int field_uvel = -1;
  int field_vvel = -1;
  int field_wvel = -1;
  for (int i=0;i<model->fieldMetas.size();i++) {
    auto &field = model->fieldMetas[i];
    if (field.name == "UVel") field_uvel = i;
    if (field.name == "VVel") field_vvel = i;
    if (field.name == "WVel") field_wvel = i;
  }
  if (field_uvel < 0) throw std::runtime_error("no 'UVel' field found");
  if (field_vvel < 0) throw std::runtime_error("no 'VVel' field found");
  if (field_wvel < 0) throw std::runtime_error("no 'WVel' field found");

  size_t u_ofs = model->fieldMetas[field_uvel].offset;
  size_t v_ofs = model->fieldMetas[field_vvel].offset;
  size_t w_ofs = model->fieldMetas[field_wvel].offset;
  
  std::vector<float> newScalars(model->numCellsAcrossAllGrids);
  for (size_t i=0;i<newScalars.size();i++) {
    float u = model->scalars[u_ofs+i];
    float v = model->scalars[v_ofs+i];
    float w = model->scalars[w_ofs+i];
    newScalars[i] = sqrtf(u*u+v*v+w*w);
  }
  Model::FieldMeta newField;
  newField.offset = 0;
  newField.name = "vel_mag";
  model->fieldMetas = { newField };
  model->scalars = newScalars;
  
  std::cout << "saving to " << outFileName << std::endl;
  model->save(outFileName);
  std::cout << "saved, done" << std::endl;
  return 0;
}

