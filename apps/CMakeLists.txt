

add_executable(tamrInfo info.cpp)
target_link_libraries(tamrInfo PUBLIC tinyAMR)

if (TARGET tamr_flash)
  add_executable(flash2tamr flash2tamr.cpp)
  target_link_libraries(flash2tamr PUBLIC tamr_flash)
endif()



