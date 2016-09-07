cmake_minimum_required(VERSION 3.6)

project(CuFM)

find_package(CUDA QUIET REQUIRED)

set(
        CUDA_HOST_COMPILER
        /usr/bin/g++-4.9
)

set(
        CUDA_NVCC_FLAGS
        ${CUDA_NVCC_FLAGS}
        -Xcompiler -DADD_ -gencode arch=compute_35,code=sm_35 -gencode arch=compute_35,code=compute_35
)

cuda_compile(ALS_O als.cu)
cuda_compile(CG_O cg.cu)
cuda_compile(DU_O device_utilities.cu)

set_source_files_properties( host_utilities.cpp PROPERTIES CUDA_SOURCE_PROPERTY_FORMAT OBJ )
cuda_compile(HU_O host_utilities.cpp )

set_source_files_properties( main.cpp PROPERTIES CUDA_SOURCE_PROPERTY_FORMAT OBJ )
cuda_compile(MAIN_O main.cpp )

cuda_add_executable(
        main
        ${ALS_O}
        ${CG_O}
        ${DU_O}
        ${HU_O}
        ${MAIN_O}
)

target_link_libraries(
        main
        -lcusparse
        -lcublas
)

link_directories(
        /usr/lib
)