

find_package(HDF5 COMPONENTS CXX)

if (HDF5_FOUND)
  add_library(tamr_flash STATIC flash.cpp)
  target_link_libraries(tamr_flash PUBLIC tinyAMR HDF5::HDF5)
else()
  message(WARNING "HDF5 importer not found - NOT building 'flash' importer (if you need this, you might want to 'sudo apt install libhdf5-dev'")
endif()

