
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
  for (int i=1;i<ac;i++) {
    const std::string arg = av[i];
    if (arg[0] != '-') {
      inFileName = arg;
    } else
      usage("tamrinfo: unknown cmdline arg '"+arg+"'");
  }

  if (inFileName.empty()) usage("no input file specified");

  tamr::Model::SP model = tamr::Model::load(inFileName);
  std::cout << "num blocks  " << prettyNumber(model->blocks.size()) << std::endl;
  std::cout << "num scalars " << prettyNumber(model->scalars.size()) << std::endl;
  std::cout << "num fields  " << prettyNumber(model->fieldMetas.size()) << std::endl;
  for (auto &meta : model->fieldMetas)
    std::cout << " - " << meta.name << " at offset " << prettyNumber(meta.offset) << std::endl;
  return 0;
}

