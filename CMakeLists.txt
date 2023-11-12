cmake_minimum_required(VERSION 3.15)
project(kuriborosu)

set(CMAKE_POLICY_DEFAULT_CMP0025 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

#######################################################################################################################
# dependencies

find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE IMPORTED_TARGET REQUIRED sndfile)

#######################################################################################################################
# Import carla stuff

add_subdirectory(src/carla/cmake)

#######################################################################################################################
# Setup kuriborosu target

add_executable(kuriborosu)
set_property(TARGET kuriborosu PROPERTY RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/$<0:>")

target_compile_definitions(kuriborosu
  PRIVATE
    BUILDING_CARLA
)

target_include_directories(kuriborosu
  PRIVATE
    .
)

target_link_libraries(kuriborosu
  PUBLIC
    carla::host-plugin
    PkgConfig::SNDFILE
)

target_sources(kuriborosu
  PRIVATE
    src/host.c
    src/kuriborosu.c
)

#######################################################################################################################
# Setup kuribu target

add_executable(kuribu)
set_property(TARGET kuribu PROPERTY RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/$<0:>")

target_compile_definitions(kuribu
  PRIVATE
    BUILDING_CARLA
)

target_include_directories(kuribu
  PRIVATE
    .
)

target_link_libraries(kuribu
  PUBLIC
    carla::host-plugin
    PkgConfig::SNDFILE
)

target_sources(kuribu
  PRIVATE
    src/host.c
    src/kuribu.c
)

#######################################################################################################################
