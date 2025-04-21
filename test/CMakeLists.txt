project(refined_tests LANGUAGES CXX)

include(CTest)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} refined_test.cpp)

target_compile_options(
  ${PROJECT_NAME}
  PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
          -Werror
          -pedantic-errors
          -Wall
          -Wextra
          -Wconversion
          -Wsign-conversion>
          $<$<CXX_COMPILER_ID:MSVC>:
          /W4>)

find_package(Catch2 REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE rvarago::refined
                                              Catch2::Catch2WithMain)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  set(SANITIZERS "-fsanitize=address -fsanitize=undefined")

  target_compile_options(${PROJECT_NAME} PRIVATE ${SANITIZSERS})
  target_link_libraries(${PROJECT_NAME} PRIVATE ${SANITIZERS})

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE "-fno-omit-frame-pointer")
  endif()
endif()

add_test(${PROJECT_NAME} ${PROJECT_NAME})

file(GLOB_RECURSE ALL_CPP_FILES ${CMAKE_SOURCE_DIR}/include/*.hpp
     ${CMAKE_SOURCE_DIR}/test/*.cpp)

add_custom_target(
  check_all
  COMMENT "Running all checks"
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${PROJECT_NAME}
  COMMAND ${CMAKE_COMMAND} -E env CT_TEST_PATH=${CMAKE_BINARY_DIR} ctest
          --output-on-failure
  COMMAND clang-format --dry-run -Werror ${ALL_CPP_FILES}
  COMMAND clang-tidy ${ALL_CPP_FILES} --warnings-as-errors='*' -p
          ${CMAKE_BINARY_DIR}/compilation_commands.json)
