add_library(tinyAMR STATIC
  Model.h
  Model.cpp
)
target_include_directories(tinyAMR PUBLIC
  ${PROJECT_SOURCE_DIR}
)



