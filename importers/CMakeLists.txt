

find_package(HDF5 COMPONENTS CXX)

if (HDF5_FOUND)
  add_executable(flash2tamr flash.cpp)
  target_link_libraries(flash2tamr PUBLIC tinyAMR HDF5::HDF5)
else()
  message(WARNING "HDF5 importer not found - NOT building 'flash' importer (if you need this, you might want to 'sudo apt install libhdf5-dev'")
endif()

