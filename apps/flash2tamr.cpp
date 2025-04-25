
#include "../importers/flash.h"

void usage(const std::string &error)
{
  std::cout << "Error: " << error << "\n\n";
  std::cout << "Usage: ./flash2tamr inFileName.silcc -o outfile.tamr" << std::endl;
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
      usage("flash2tamr: unknown cmdline arg '"+arg+"'");
  }
    
  if (inFileName.empty()) usage("no input file specified");
  if (outFileName.empty()) usage("no output file specified");

  Model::SP model = import_FLASH(inFileName.c_str());
  std::cout << "done reading, saving to " << outFileName << std::endl;
  model->save(outFileName);
  return 0;
}
