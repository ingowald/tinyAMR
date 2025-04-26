
#include "../importers/exa.h"

void usage(const std::string &error)
{
  std::cout << "Error: " << error << "\n\n";
  std::cout << "Usage: ./exa2tamr inFileName.cells sclar1.scalars [scalar2.scalars ...] -o outfile.tamr" << std::endl;
  exit(1);
}
  
int main(int ac, char **av)
{
  using namespace tamr;
    
  std::string cellsFileName;
  std::vector<std::string> scalarsFileNames;
  std::string outFileName;
  for (int i=1;i<ac;i++) {
    const std::string arg = av[i];
    if (arg[0] != '-') {
      if (endsWith(arg,".scalars")) {
        scalarsFileNames.push_back(arg);
      } else if (endsWith(arg,".cells")) {
        cellsFileName = arg;
      } else
        usage("unrecognized input file name " + arg);
    } else if (arg == "-o") {
      outFileName = av[++i];
    } else
      usage("exa2tamr: unknown cmdline arg '"+arg+"'");
  }
    
  if (cellsFileName.empty()) usage("no .cells file specified");
  if (scalarsFileNames.empty()) usage("no .scalars file(s) specified");
  if (outFileName.empty()) usage("no output file specified");

  Model::SP model = import_exa(cellsFileName,scalarsFileNames);
  std::cout << "done reading, saving to " << outFileName << std::endl;
  model->save(outFileName);
  return 0;
}
