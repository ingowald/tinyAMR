

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
# EXABRICK style data
# ------------------------------------------------------------------
if (TARGET tamr_exa)
  add_executable(exa2tamr exa2tamr.cpp)
  target_link_libraries(exa2tamr PUBLIC tamr_exa)
endif()



