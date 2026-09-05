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

#include "../importers/chombo.h"

void usage(const std::string &error)
{
  std::cout << "Error: " << error << "\n\n";
  std::cout << "Usage: ./chombo2tamr inFileName.silcc -o outfile.tamr" << std::endl;
  exit(1);
}
  
int main(int ac, char **av)
{
  using namespace tamr;
    
  std::string inFileName;
  std::string outFileName;
  for (int i=1;i<ac;i++) {
    const std::string arg = av[i];
    if (arg[0] != '-')
      inFileName = arg;
    else if (arg == "-o") {
      outFileName = av[++i];
    } else
      usage("chombo2tamr: unknown cmdline arg '"+arg+"'");
  }
    
  if (inFileName.empty()) usage("no input file specified");
  if (outFileName.empty()) usage("no output file specified");

  Model::SP model = import_chombo(inFileName.c_str());
  std::cout << "done reading, saving to " << outFileName << std::endl;
  model->save(outFileName);
  return 0;
}
