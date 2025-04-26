#pragma once

#include "tinyAMR/TinyAMR.h"

namespace tamr {

  Model::SP import_exa(const std::string &cellFileName,
                       const std::vector<std::string> &scalarsFileName);
  
}
