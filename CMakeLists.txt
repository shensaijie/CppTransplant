cmake_minimum_required(VERSION 3.25)
project(CppTransplant VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

find_package(Clang REQUIRED CONFIG)
message(STATUS "Found Clang ${CLANG_PACKAGE_VERSION}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(CppTransplant main.cpp)

# 如果你需要特定的LLVM组件
set(LLVM_LINK_COMPONENTS
  MC
  Option
  FrontendOpenMP
  Support
  TargetParser)
llvm_map_components_to_libnames(llvm_libs ${LLVM_LINK_COMPONENTS})

target_link_libraries(CppTransplant
    PRIVATE
    ${llvm_libs}
    clangAST
    clangASTMatchers
    clangBasic
    clangDependencyScanning
    clangDriver
    clangFormat
    clangFrontend
    clangLex
    clangRewrite
    clangSerialization
    clangTooling
    clangToolingCore
    clangToolingInclusions
    clangToolingInclusionsStdlib
    clangToolingRefactoring
    clangTransformer
)