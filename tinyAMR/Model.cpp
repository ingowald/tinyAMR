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
#include <fstream>

namespace tamr {
  const size_t magic = 0x66554465ABABull;

  template<typename T> void write(std::ostream &out, const T &scalar)
  { out.write((char*)&scalar,sizeof(scalar)); }

  template<typename T> T read(std::istream &in)
  { T scalar; in.read((char*)&scalar,sizeof(scalar)); return scalar; }
  
  void writeString(std::ostream &out, const std::string &s)
  {
    write(out,(int)s.size());
    out.write((char*)s.c_str(),s.size());
  }

  std::string readString(std::istream &in)
  {
    std::vector<char> chars(read<int>(in));
    in.read((char*)chars.data(),chars.size());
    chars.push_back(0);
    return chars.data();
  }

  template<typename T>
  void writeVector(std::ostream &out, const std::vector<T> &vec)
  {
    write(out,vec.size());
    out.write((char*)vec.data(),vec.size()*sizeof(T));
  }
  
  template<typename T>
  void readVector(std::istream &in, std::vector<T> &vec)
  {
    vec.resize(read<size_t>(in));
    in.read((char*)vec.data(),vec.size()*sizeof(T));
  }
  
  void Model::save(const std::string &fileName) const
  {
    std::ofstream out(fileName,std::ios::binary);
    out.write((char *)&magic,sizeof(magic));

    writeVector(out,refinementOfLevel);
    writeVector(out,scalars);
    writeVector(out,grids);
    write(out,numCellsAcrossAllGrids);
    write(out,(int)fieldMetas.size());
    for (auto &meta : fieldMetas) {
      writeString(out,meta.name);
      write(out,meta.numDimensions);
      write(out,meta.offset);
      writeString(out,meta.info);
    }
    writeString(out,userMeta);
  }

  Model::SP Model::load(const std::string &fileName)
  {
    Model::SP model = std::make_shared<Model>();
    std::ifstream in(fileName,std::ios::binary);

    size_t magic;
    in.read((char *)&magic,sizeof(magic));
    if (magic != tamr::magic) throw std::runtime_error("wrong magic number");

    readVector(in,model->refinementOfLevel);
    readVector(in,model->scalars);
    readVector(in,model->grids);
    model->numCellsAcrossAllGrids = read<size_t>(in);
    model->fieldMetas.resize(read<int>(in));
    for (auto &meta : model->fieldMetas) {
      meta.name = readString(in);
      meta.numDimensions = read<int>(in);
      meta.offset = read<uint64_t>(in);
      meta.info = readString(in);
    }
    model->userMeta = readString(in);
    
    
    return model;
  }

} // ::tinyAMR
