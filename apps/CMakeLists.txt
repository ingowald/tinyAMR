

add_executable(tamrInfo info.cpp)
target_link_libraries(tamrInfo PUBLIC tinyAMR)

# ------------------------------------------------------------------
# FLASH reader (e.g, for SILCC or SoaresFurtado test data)
# ------------------------------------------------------------------
if (TARGET tamr_flash)
  add_executable(flash2tamr flash2tamr.cpp)
  target_link_libraries(flash2tamr PUBLIC tamr_flash)
endif()


# ------------------------------------------------------------------
# CHOMBO reader (e.g, for SILCC or SoaresFurtado test data)
# ------------------------------------------------------------------
if (TARGET tamr_chombo)
  add_executable(chombo2tamr chombo2tamr.cpp)
  target_link_libraries(chombo2tamr PUBLIC tamr_chombo)
endif()


# ------------------------------------------------------------------
# EXABRICK style data
# ------------------------------------------------------------------
if (TARGET tamr_exa)
  add_executable(exa2tamr exa2tamr.cpp)
  target_link_libraries(exa2tamr PUBLIC tamr_exa)
endif()




# ------------------------------------------------------------------
# tool for stripping ghost cells (from chombo imported data)
# ------------------------------------------------------------------
add_executable(tamrStripGhosts stripGhosts.cpp)
target_link_libraries(tamrStripGhosts PUBLIC tamr_chombo)


# ------------------------------------------------------------------
# tool to compute vel_mag field from UVel, VVel, WVel (chombo)
# ------------------------------------------------------------------
add_executable(tamrComputeVelMag computeVelMag.cpp)
target_link_libraries(tamrComputeVelMag PUBLIC tamr_chombo)

