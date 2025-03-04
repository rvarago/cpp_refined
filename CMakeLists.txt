cmake_minimum_required(VERSION 3.21)

project(
  refined
  DESCRIPTION "Types + Predicates in C++, sort-of"
  LANGUAGES CXX)
add_library(${PROJECT_NAME} INTERFACE)

option(RVARAGO_REFINED_WITH_TESTS "All correctness checks" OFF)

add_library(rvarago::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
  ${PROJECT_NAME}
  INTERFACE $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Config)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT ${PROJECT_NAME}Config
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  NAMESPACE rvarago::)

export(EXPORT ${PROJECT_NAME}Config NAMESPACE rvarago::)

export(PACKAGE ${PROJECT_NAME})

if(RVARAGO_REFINED_WITH_TESTS)
  add_subdirectory(test)
endif()
