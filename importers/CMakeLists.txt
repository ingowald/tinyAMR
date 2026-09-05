

find_package(HDF5 COMPONENTS CXX)

# ------------------------------------------------------------------
# FLASH reader (e.g, for SILCC or SoaresFurtado test data)
# ------------------------------------------------------------------
if (HDF5_FOUND)
  add_library(tamr_flash STATIC flash.cpp)
  target_link_libraries(tamr_flash PUBLIC tinyAMR HDF5::HDF5)
else()
  message(WARNING "HDF5 importer not found - NOT building 'flash' importer (if you need this, you might want to 'sudo apt install libhdf5-dev'")
endif()

# ------------------------------------------------------------------
# CHOMBO reader (e.g, for nasa lava landing gear
# ------------------------------------------------------------------
if (HDF5_FOUND)
  add_library(tamr_chombo STATIC chombo.cpp) 
  target_link_libraries(tamr_chombo PUBLIC tinyAMR HDF5::HDF5)
else()
  message(WARNING "HDF5 importer not found - NOT building 'chombo' importer (if you need this, you might want to 'sudo apt install libhdf5-dev'")
endif()


# ------------------------------------------------------------------
# EXABRICK style data
# ------------------------------------------------------------------
add_library(tamr_exa STATIC exa.cpp)
target_link_libraries(tamr_exa PUBLIC tinyAMR)
